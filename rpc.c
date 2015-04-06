#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "rputils.h"
#include "interp.h"
#include "clist.h"
#include "opcodes.h"

/*
	e.g.:

	:factorial 1 eq ?ret dup 1 @core_sub factorial mul { .opcode halt }
*/
#define LIBNAME_LEN  256
#define FUNCNAME_LEN 256
#define MAX_TOKENS   1024

struct toknode {
	struct clnode hdr;
	enum {
		TOK_LABEL,
		TOK_LITERAL_INT,
		TOK_BUILTIN_RET,
		TOK_GOTO,
		TOK_FUNCALL,
		TOK_FOREIGN,
		TOK_ASM,
	} type;
	_Bool is_cond;
	_Bool is_export;
	char name[TOKEN_MAX_LEN];
	int64_t lit_val;
};

struct toknode *toknode_init(void *p)
{
	struct toknode *tn = (struct toknode *)p;
	if(tn) {
		clnode_init(&tn->hdr);
		tn->is_cond = 0;
		tn->is_export = 0;
		tn->name[0] = 0;
		tn->lit_val = 0;
	}
	return tn;
}

unsigned tokenize_FILE(FILE *instream, struct clist *tokens)
{
	char buf[TOKEN_MAX_LEN + 1];
	unsigned len;
	unsigned num_tokens = 0;
	while((len = read_word(instream, buf, sizeof(buf)-1, '#'))) {
		struct toknode *n = toknode_init(malloc(sizeof(struct toknode)));
		char *cursor = buf;

		// label?
		if(cursor[0] == ':') {
			cursor++;
			n->type = TOK_LABEL;
			if(cursor[0] == ':') {
				n->is_export = 1;
				cursor++;
			}
			strncpy(n->name, cursor, sizeof(n->name));
			n->name[sizeof(n->name)-1] = 0;
			goto next;
		}

		// inline asm?
		if(cursor[0] == '$') {
			cursor++;
			n->type = TOK_ASM;
			strncpy(n->name, cursor, sizeof(n->name));
			n->name[sizeof(n->name)-1] = 0;
			char *c;
			while((c = strchr(n->name, ',')) != NULL) {
				*c = ' ';
			}
			goto next;
		}

		// conditional?
		if(cursor[0] == '?') {
			n->is_cond = 1;
			cursor++;
		}

		// literal?
		{
			char *endp;
			int64_t val = strtoll(buf, &endp, 0);
			if(*endp == 0) {
				n->type = TOK_LITERAL_INT;
				n->lit_val = val;
				goto next;
			}
		}

		// builtin?
		if(strcmp(cursor, "ret") == 0) {
			n->type = TOK_BUILTIN_RET;
			goto next;
		}

		// goto / funcall
		if(cursor[0] == '/') {
			n->type = TOK_GOTO;
			cursor++;
		} else if(cursor[0] == '@') {
			n->type = TOK_FOREIGN;
			cursor++;
		} else {
			n->type = TOK_FUNCALL;
		}
		strncpy(n->name, cursor, sizeof(n->name));
		n->name[sizeof(n->name)-1] = 0;

next:
		clist_queue(tokens, &n->hdr);
		num_tokens++;
	}
	return num_tokens;;
}

void print_toknode(struct toknode *tn)
{
	switch(tn->type) {
	case TOK_LABEL:
		printf("label   %s%s", tn->name,
		       tn->is_export ? " (export)" : "");
		break;
	case TOK_LITERAL_INT:
		printf("literal %lld%s", (long long) tn->lit_val,
		       tn->is_cond ? " (conditional)" : "");
		break;
	case TOK_BUILTIN_RET:
		printf("builtin re %s",
		       tn->is_cond ? " (conditional)" : "");
		break;
	case TOK_GOTO:
		printf("goto    %s%s", tn->name,
		       tn->is_cond ? " (conditional)" : "");
		break;
	case TOK_FUNCALL:
		printf("funcall %s%s", tn->name,
		       tn->is_cond ? " (conditional)" : "");
		break;
	case TOK_FOREIGN:
		printf("foreign %s%s", tn->name,
		       tn->is_cond ? " (conditional)" : "");
		break;
	case TOK_ASM:
		printf("asm     %s", tn->name);
		break;
	}
}

void print_toknodes(struct clist *tokens)
{
	for(struct clnode *i = clist_first(tokens);
	    i != &tokens->hdr;
	    i = clnode_next(i)) {
		struct toknode *tn = (struct toknode *)i;
		print_toknode(tn);
		printf("\n");
	}
}

int main(int argc, char *argv[])
{
	if(argc != 3) {
usage:
		fprintf(stderr, "usage: %s <infile> <outfile>\n", argv[0]);
		return -1;
	} 
	char *infile = argv[1];
	char *outfile = argv[2];

	//
	// tokenize/parse input file
	//

	FILE *instream;
	if(strcmp(infile, "--") == 0)
		instream = stdin;
	else
		instream = fopen(infile, "r");
	if(! instream) {
		perror("fopen()");
		fprintf(stderr, "file %s\n", infile);
		goto usage;
	}
	struct clist tokens;	
	clist_init(&tokens);
	unsigned n = tokenize_FILE(instream, &tokens);
	(void) n;
	fclose(instream);

	FILE *outstream;
	if(strcmp(outfile, "--") == 0)
		outstream = stdout;
	else
		outstream = fopen(outfile, "w");
	if(! outstream) {
		perror("fopen()");
		fprintf(stderr, "file %s\n", infile);
		goto usage;
	}

	//
	// code generation
	//

	unsigned local_label_num = 0;
	char local_label[TOKEN_MAX_LEN];

	for(struct clnode *i = clist_first(&tokens);
	    i != clist_end(&tokens);
	    i = clnode_next(i)) {
		struct toknode *tn = (struct toknode *)i;

		if(tn->is_cond) {
			snprintf(local_label, sizeof(local_label)-1,
			         "__local_lbl_%u", local_label_num++);
			fprintf(outstream, "\t.mnemonic branchz .reladdr %s\n", local_label);
		}

		switch(tn->type) {
		case TOK_LABEL:
			if(tn->is_export)
				fprintf(outstream, ".export %s\n", tn->name);
			else
				fprintf(outstream, ".label %s\n", tn->name);
			break;

		case TOK_LITERAL_INT:
			fprintf(outstream, "\t.mnemonic push64 .data_s8 %lld\n",
			        (long long) tn->lit_val);
			break;

		case TOK_BUILTIN_RET:
			fprintf(outstream, "\t.mnemonic ret\n");
			break;

		case TOK_GOTO:
			fprintf(outstream, "\t.mnemonic gotorel .reladdr %s\n", tn->name);
			break;

		case TOK_FUNCALL:
			fprintf(outstream, "\t.mnemonic callrel .reladdr %s\n", tn->name);
			break;

		case TOK_FOREIGN:
			fprintf(outstream, "\t.mnemonic foreign .foreign_id %s\n", tn->name);
			break;

		case TOK_ASM:
			fprintf(outstream, "\t%s\n", tn->name);
			break;
		}

		if(tn->is_cond)
			fprintf(outstream, ".label %s\n", local_label);
	}

	fclose(outstream);

	return 0;
}
