#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opcodes.h"
#include "rputils.h"

#define X(foo) #foo, 
char *mnemonics[] = {
	OPCODES
};
#undef X

#define ARRLEN(X) ( sizeof(X) / sizeof((X)[0]) )

//int read_word(FILE *instream, char *buf, unsigned buflen);
//struct name_addr_node *name_addr_node_init(void *p, char *name, unsigned addr);


int main(int argc, char *argv[])
{
	if(argc != 3) {
		printf("usage: %s <infile> <outfile>\n", argv[0]);
		return -1;
	}
	char *infile = argv[1];
	char *outfile = argv[2];

	FILE *instream;
	if(strcmp(infile, "--") == 0)
		instream = stdin;
	else
		instream = fopen(infile, "r");
	if(! instream) {
		perror("fopen()");
		fprintf(stderr, "file %s\n", infile);
		return -1;
	}

	FILE *outstream;
	if(strcmp(outfile, "--") == 0)
		outstream = stdout;
	else
		outstream = fopen(outfile, "w");
	if(! outstream) {
		perror("fopen()");
		fprintf(stderr, "file %s\n", outfile);
		return -1;
	}

	// tokenize
	struct clist words;
	clist_init(&words);
	read_words_from_FILE(instream, &words, ';');


	struct comp_unit cu;
	size_t out_text_cap = 0;
	comp_unit_init(&cu);

	// pass 1:
	// identify and store all labels and addressess
	// identify exported labels
	// identify patch locations
	// generate code and immediates

	struct clist label_addrs;
	clist_init(&label_addrs);

	struct clist abs_patch_addrs;
	clist_init(&abs_patch_addrs);

	struct clist rel_patch_addrs;
	clist_init(&rel_patch_addrs);

	for(struct clnode *i = clist_first(&words);
	    i != clist_end(&words);
	    i = clnode_next(i)) {

		struct word_node *wn = (struct word_node *)i;
		char *directive = wn->word;
		if(*directive != '.') {
			printf("expected directive: %s\n", directive);
			return -1;
		}
		directive++;

		i = clnode_next(i);
		if(i == clist_end(&words)) {
			printf("all directives require an argument\n");
			return -1;
		}
		struct word_node *wn_arg = (struct word_node *)i;
		char *arg = wn_arg->word;
		if(*arg == '.') {
			printf("expected arg: %s\n", arg);
			return -1;
		}

		struct name_addr_node *n_a_node;
		char *endp;
		unsigned long long arg_uval;
		long long arg_sval;

		if(strcmp(directive, "label") == 0) {

			n_a_node =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)), arg, cu.text_len);

			clist_queue(&label_addrs, &n_a_node->hdr);

		} else if(strcmp(directive, "export") == 0) {

			n_a_node=
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)), arg, cu.text_len);

			clist_queue(&label_addrs, &n_a_node->hdr);

			n_a_node =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)), arg, cu.text_len);

			clist_queue(&cu.exported, &n_a_node->hdr);

		} else if(strcmp(directive, "data_u1") == 0) {

			arg_uval = strtoull(arg, &endp, 0);
			if(*endp) {
				printf("parse u1 immedate error for %s\n", arg);
				return 0;
			}
			if(arg_uval > 0xFFULL) {
				printf("parse u1 immedate too large %s\n", arg);
				return 0;
			}

			uint8_t v = arg_uval;

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, &v, sizeof(v));

		} else if(strcmp(directive, "data_u2") == 0) {

			arg_uval = strtoull(arg, &endp, 0);
			if(*endp) {
				printf("parse u2 immedate error for %s\n", arg);
				return 0;
			}
			if(arg_uval > 0xFFFFULL) {
				printf("parse u2 immedate too large %s\n", arg);
				return 0;
			}

			uint16_t v = arg_uval;
			v = htobe16(v);

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "data_u4") == 0) {

			arg_uval = strtoull(arg, &endp, 0);
			if(*endp) {
				printf("parse u4 immedate error for %s\n", arg);
				return 0;
			}
			if(arg_uval > 0xFFFFFFFFULL) {
				printf("parse u4 immedate too large %s\n", arg);
				return 0;
			}

			uint32_t v = arg_uval;
			v = htobe32(v);

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "data_u8") == 0) {

			arg_uval = strtoull(arg, &endp, 0);
			if(*endp) {
				printf("parse u8 immedate error for %s\n", arg);
				return 0;
			}

			uint64_t v = arg_uval;
			v = htobe64(v);

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "data_s1") == 0) {

			arg_sval = strtoll(arg, &endp, 0);
			if(*endp) {
				printf("parse s1 immedate error for %s\n", arg);
				return 0;
			}
			if(arg_sval > 0x7FLL || arg_sval < -0x80LL) {
				printf("parse s1 immedate out of range %s\n", arg);
				return 0;
			}

			int8_t v = arg_sval;

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "data_s2") == 0) {

			arg_sval = strtoll(arg, &endp, 0);
			if(*endp) {
				printf("parse s2 immedate error for %s\n", arg);
				return 0;
			}
			if(arg_sval > 0x7FFFLL || arg_sval < -0x8000LL) {
				printf("parse s2 immedate out of range %s\n", arg);
				return 0;
			}

			int16_t v = arg_sval;
			v = htobe16(v);

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "data_s4") == 0) {

			arg_sval = strtoll(arg, &endp, 0);
			if(*endp) {
				printf("parse s4 immedate error for %s\n", arg);
				return 0;
			}
			if(arg_sval > 0x7FFFFFFFLL || arg_sval < -0x80000000LL) {
				printf("parse s4 immedate out of range %s\n", arg);
				return 0;
			}

			int32_t v = arg_sval;
			v = htobe32(v);

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "data_s8") == 0) {

			arg_sval = strtoll(arg, &endp, 0);
			if(*endp) {
				printf("parse s8 immedate error for %s\n", arg);
				return 0;
			}

			int64_t v = arg_sval;
			v = htobe64(v);

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "addr") == 0) {

			n_a_node =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)), arg, cu.text_len);
			clist_queue(&abs_patch_addrs, &n_a_node->hdr);

			uint32_t v = 0;
			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "reladdr") == 0) {

			n_a_node =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)), arg, cu.text_len);
			clist_queue(&rel_patch_addrs, &n_a_node->hdr);

			int32_t v = 0;
			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "foreign_id") == 0) {

			n_a_node =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)), arg, cu.text_len);
			clist_queue(&cu.foreign_deps, &n_a_node->hdr);

			// patched up later
			uint32_t v = 0;
			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, (unsigned char*)&v, sizeof(v));

		} else if(strcmp(directive, "mnemonic") == 0) {

			uint8_t op;
			for(op = 0; op < ARRLEN(mnemonics); op++)
				if(strcasecmp(mnemonics[op], arg) == 0)
					break;

			if(op == ARRLEN(mnemonics)) {
				printf("unknown mnemonic: %s\n", arg);
				return -1;
			}

			array_append_bytes(
				&cu.text, &cu.text_len, &out_text_cap, &op, sizeof(op));

		} else {
			printf("unknown assembler directive: %s\n", directive);
			return -1;
		}

	}

	// pass 2:
	// fixup patch addresses

	while(! clist_is_empty(&rel_patch_addrs)) {
		struct clnode *i = clnode_remove(clist_first(&rel_patch_addrs));
		struct name_addr_node *patch_dst = (struct name_addr_node *)i;

		struct name_addr_node *patch_src =
			name_addr_node_find(&label_addrs, patch_dst->name);

		if(! patch_src) { // external dep
			clist_queue(&cu.rel_deps, &patch_dst->hdr);
		} else {
			jump_t jump_target = patch_src->addr;
			jump_t jump_source = patch_dst->addr;
			jump_t jump = jump_target - jump_source;
			jump = htobe32(jump);
			memcpy(cu.text + patch_dst->addr, &jump, sizeof(jump));

			free(i);
		}
	}


	while(! clist_is_empty(&abs_patch_addrs)) {
		struct clnode *i = clnode_remove(clist_first(&abs_patch_addrs));
		struct name_addr_node *patch_dst = (struct name_addr_node *)i;

		struct name_addr_node *patch_src =
			name_addr_node_find(&label_addrs, patch_dst->name);

		if(! patch_src) { // external dep
			clist_queue(&cu.abs_deps, &patch_dst->hdr);
		} else {
			jump_t jump = patch_src->addr;
			jump = htobe32(jump);
			memcpy(cu.text + patch_dst->addr, &jump, sizeof(jump));

			free(i);
		}
	}
	
	//
	// write object file
	//

	comp_unit_write(&cu, argv[2]);

	return 0;
}
