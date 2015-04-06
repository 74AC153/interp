#if ! defined(RPUTILS_H_INCLUDED)
#define RPUTILS_H_INCLUDED

#include <stdio.h>

#define TOKEN_MAX_LEN 256

#include "clist.h"

#include "interp.h"


int read_word(FILE *instream, char *buf, unsigned buflen, char comment_ch);

struct word_node {
	struct clnode hdr;
	char word[TOKEN_MAX_LEN];
};

struct word_node *word_node_init(void *p);

int read_words_from_FILE(FILE *instream, struct clist *l, char comment_ch);


int array_append_bytes(
	unsigned char **arr, size_t *len, size_t *cap,
	unsigned char *data, unsigned data_len);

int array_append_byte(
	unsigned char **arr, size_t *len, size_t *cap,
	unsigned char byte);


struct name_addr_node {
	struct clnode hdr;
	char name[TOKEN_MAX_LEN];
	jump_t addr;
};

struct name_addr_node *name_addr_node_init(void *p, char *name, unsigned addr);

struct name_addr_node *name_addr_node_find(struct clist *l, char *name);

size_t name_addr_list_write(struct clist *l, FILE *outstream);

size_t name_addr_list_read(struct clist *l, unsigned char *buf);


struct comp_unit {
	struct clist exported;
	struct clist foreign_deps;
	struct clist abs_deps;
	struct clist rel_deps;
	size_t text_len;
	unsigned char *text;
};

struct comp_unit *comp_unit_init(void *p);

struct comp_unit *comp_unit_read(char *path);

int comp_unit_patch(
	struct comp_unit *cu,
	struct clist *abs_patch_addrs,
	struct clist *rel_patch_addrs);

int comp_unit_write(struct comp_unit *u, char *path);

#endif
