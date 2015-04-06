#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "rputils.h"

int read_word(FILE *instream, char *buf, unsigned buflen, char comment_ch)
{
	int ch;
	char *cursor = buf;

start:
	ch = fgetc(instream);

	if(ch == EOF)
		return 0;

	// skip leading whitespace
	if(isspace(ch))
		while((ch = fgetc(instream)) != EOF)
			if(! isspace(ch)) {
				ungetc(ch, instream);
				goto start;
			}

	// skip comments
	if(ch == comment_ch)
		while((ch = fgetc(instream)) != EOF)
			if(ch == '\n')
				goto start;

	ungetc(ch, instream);

	// read token up until whitespace or comment
	while((ch = fgetc(instream)) != EOF) {
		if(isspace(ch))
			break;

		if(ch == comment_ch)
			break;

		*cursor++ = ch;

		if(cursor - buf == buflen - 1)
			break;
	}

	*cursor++ = 0; // null byte

	return cursor - buf;
}

struct word_node *word_node_init(void *p)
{
	struct word_node *n = (struct word_node *)p;
	if(n) {
		clnode_init(&n->hdr);
		n->word[0] = 0;
	}
	return n;
}

int read_words_from_FILE(FILE *instream, struct clist *l, char comment_ch)
{
	char buf[TOKEN_MAX_LEN] = { 0 };
	unsigned buf_used = 0;

	while(0 < (buf_used = read_word(instream, buf, sizeof(buf), comment_ch))) {
		struct word_node *wn = word_node_init(malloc(sizeof(struct word_node)));
		strncpy(wn->word, buf, sizeof(wn->word));
		wn->word[sizeof(wn->word)-1] = 0;	
		clist_queue(l, &wn->hdr);
	}
	return 0;
}

int array_append_bytes(
	unsigned char **arr, size_t *len, size_t *cap,
	unsigned char *data, unsigned data_len)
{
	assert(*len <= *cap);
	if(*len + data_len > *cap) {
		assert((*cap == 0) == (*arr == NULL));
		size_t newcap = *cap ? *cap * 2 : 8;
		while(newcap < (*len + data_len))
			newcap *= 2;

		unsigned char *newarr = realloc(*arr, newcap);
		if(! newarr)
			return -1;
		*arr = newarr;
		*cap = newcap;
	}
	assert(*arr);

	memcpy(*arr + *len, data, data_len);
	(*len) += data_len;

	return 0;
}

int array_append_byte(
	unsigned char **arr, size_t *len, size_t *cap,
	unsigned char byte)
{
	return array_append_bytes(arr, len, cap, &byte, 1);
}

struct name_addr_node *name_addr_node_init(void *p, char *name, unsigned addr)
{
	struct name_addr_node *nan = (struct name_addr_node *)p;
	if(nan) {
		clnode_init(&nan->hdr);
		strncpy(nan->name, name, sizeof(nan->name)-1);
		nan->name[sizeof(nan->name)-1] = 0;
		nan->addr = addr;
	}
	return nan;
}

struct name_addr_node *name_addr_node_find(struct clist *l, char *name)
{
	struct clnode *j;
	struct name_addr_node *match;
	for(j = clist_first(l); j != clist_end(l); j = clnode_next(j)) {
		match = (struct name_addr_node *) j;
		if(strcmp(match->name, name) == 0)
			return match;
	}
	return NULL;
}

size_t name_addr_list_write(struct clist *l, FILE *outstream)
{
	for(struct clnode *i = clist_first(l);
	    i != clist_end(l);
	    i = clnode_next(i)) {
		struct name_addr_node *n_a_node = (struct name_addr_node *)i;
		fwrite(n_a_node->name, strlen(n_a_node->name)+1, 1, outstream);
		jump_t targ = htobe32(n_a_node->addr);
		fwrite(&targ, sizeof(targ), 1, outstream);
	}
	fputc(0, outstream);
	return 0;
}

size_t name_addr_list_read(
	struct clist *l, unsigned char *buf)
{
	unsigned char *cursor = buf;
	char name[TOKEN_MAX_LEN];
	jump_t addr;

	// exported functions
	while(*cursor) {
		strncpy(name, (char*)cursor, sizeof(name)-1);
		name[TOKEN_MAX_LEN-1] = 0;
		cursor += strlen((char *)cursor) + 1;

		memcpy(&addr, cursor, sizeof(addr));
		addr = be32toh(addr);
		cursor += sizeof(addr);

		struct name_addr_node *nan = 
			name_addr_node_init(malloc(sizeof(struct name_addr_node)),
			                    name, addr);

		clist_queue(l, &nan->hdr);
	}
	cursor++;

	return cursor - buf;
}

struct comp_unit *comp_unit_init(void *p)
{
	struct comp_unit *u = (struct comp_unit *)p;
	if(u) {
		clist_init(&u->exported);
		clist_init(&u->foreign_deps);
		clist_init(&u->abs_deps);
		clist_init(&u->rel_deps);
		u->text_len = 0;
		u->text = NULL;
	}
	return u;
}

struct comp_unit *comp_unit_read(char *path)
{
	int infd = open(path, O_RDONLY);

	if(! infd < 0) {
		perror("open()");
		fprintf(stderr, "path=%s\n", path);
		return NULL;
	}

	struct stat sb;
	if(fstat(infd, &sb)) {
		perror("fstat()");
		return NULL;
	}

	unsigned char *data =
		mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, infd, 0);
	if(data == MAP_FAILED) {
		perror("mmap()");
		return NULL;
	}

	struct comp_unit *cu = 
		comp_unit_init(malloc(sizeof(struct comp_unit)));

	unsigned char *cursor = data;

	size_t len;

	// exported functions
	len = name_addr_list_read(&cu->exported, cursor);
	if(len == 0)
		return NULL;
	cursor += len;

	len = name_addr_list_read(&cu->foreign_deps, cursor);
	if(len == 0)
		return NULL;
	cursor += len;

	len = name_addr_list_read(&cu->abs_deps, cursor);
	if(len == 0)
		return NULL;
	cursor += len;

	len = name_addr_list_read(&cu->rel_deps, cursor);
	if(len == 0)
		return NULL;
	cursor += len;

	// program text
	cu->text_len = sb.st_size - (cursor - data);
	cu->text = malloc(cu->text_len);
	memcpy(cu->text, cursor, cu->text_len);

	return cu;
}

int comp_unit_patch(struct comp_unit *cu, struct clist *abs_patch_addrs, struct clist *rel_patch_addrs)
{
	while(! clist_is_empty(rel_patch_addrs)) {
		struct clnode *i = clnode_remove(clist_first(rel_patch_addrs));
		struct name_addr_node *patch_dst = (struct name_addr_node *)i;

		struct name_addr_node *patch_src =
			name_addr_node_find(&cu->exported, patch_dst->name);

		if(! patch_src) { // external dep
			clist_queue(&cu->rel_deps, &patch_dst->hdr);
		} else {
			jump_t jump_target = patch_src->addr;
			jump_t jump_source = patch_dst->addr;
			jump_t jump = jump_target - jump_source;
			jump = htobe32(jump);
			memcpy(cu->text + patch_dst->addr, &jump, sizeof(jump));

			free(i);
		}
	}

	while(! clist_is_empty(abs_patch_addrs)) {
		struct clnode *i = clnode_remove(clist_first(abs_patch_addrs));
		struct name_addr_node *patch_dst = (struct name_addr_node *)i;

		struct name_addr_node *patch_src =
			name_addr_node_find(&cu->exported, patch_dst->name);

		if(! patch_src) { // external dep
			clist_queue(&cu->abs_deps, &patch_dst->hdr);
		} else {
			jump_t jump = patch_src->addr;
			jump = htobe32(jump);
			memcpy(cu->text + patch_dst->addr, &jump, sizeof(jump));

			free(i);
		}
	}

	return 0;
}

int comp_unit_write(struct comp_unit *u, char *path)
{
	FILE *outstream = fopen(path, "wb");
	if(! outstream) {
		perror("fopen()");
		return -1;
	}

	name_addr_list_write(&u->exported, outstream);
	name_addr_list_write(&u->foreign_deps, outstream);
	name_addr_list_write(&u->abs_deps, outstream);
	name_addr_list_write(&u->rel_deps, outstream);
	fwrite(u->text, u->text_len, 1, outstream);

	fclose(outstream);
	return 0;
}
