#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "clist.h"

#include "rputils.h"

struct comp_unit_node {
	struct clnode hdr;
	struct comp_unit *unit;
};

struct comp_unit_node *comp_unit_node_init(void *p, struct comp_unit *u)
{
	struct comp_unit_node *n = (struct comp_unit_node *)p;
	if(n) {
		clnode_init(&n->hdr);
		n->unit = u;
	}
	return n;
}

int main(int argc, char *argv[])
{
	struct clist comp_units;
	clist_init(&comp_units);
	char *outfile = "out.rpx";

	// usage: rpl -o <outfile> <infile>
	int opt;
	while((opt = getopt(argc, argv, "o:")) != -1) {
		switch(opt) {
		case 'o': 
			outfile = optarg;
			break;
		default:
			{
				printf("usage: %s -o <outfile> <infile> ...\n", argv[0]);
				return -1;
			}
			break;
		}
	}


	struct comp_unit out_u;
	comp_unit_init(&out_u);
	size_t out_text_cap = 0;

	struct clist abs_patch_addrs;
	clist_init(&abs_patch_addrs);

	struct clist rel_patch_addrs;
	clist_init(&rel_patch_addrs);


	for(int i = optind; i < argc; i++) {
		struct comp_unit *u = comp_unit_read(argv[i]);
		if(u == NULL)
			return -1;

		// for each infile:

		// emit its code
		size_t text_start_off = out_u.text_len;

		array_append_bytes(
			&out_u.text, &out_u.text_len, &out_text_cap,
			u->text, u->text_len);

		// collate export locations
		for(struct clnode *i = clist_first(&u->exported);
		    i != clist_end(&u->exported);
		    i = clnode_next(i)) {
			struct name_addr_node *uex = (struct name_addr_node *)i;

			struct name_addr_node *ex =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)),
					uex->name, uex->addr + text_start_off);

			clist_queue(&out_u.exported, &ex->hdr);
		}

		// collate abs-dep locations
		for(struct clnode *i = clist_first(&u->abs_deps);
		    i != clist_end(&u->abs_deps);
		    i = clnode_next(i)) {
			struct name_addr_node *udep = (struct name_addr_node *)i;

			struct name_addr_node *dep =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)),
					udep->name, udep->addr + text_start_off);

			clist_queue(&abs_patch_addrs, &dep->hdr);
		}

		// collate rel-dep locations
		for(struct clnode *i = clist_first(&u->rel_deps);
		    i != clist_end(&u->rel_deps);
		    i = clnode_next(i)) {
			struct name_addr_node *udep = (struct name_addr_node *)i;

			struct name_addr_node *dep =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)),
					udep->name, udep->addr + text_start_off);

			clist_queue(&rel_patch_addrs, &dep->hdr);
		}

		// collate foreign locations
		for(struct clnode *i = clist_first(&u->foreign_deps);
		    i != clist_end(&u->foreign_deps);
		    i = clnode_next(i)) {
			struct name_addr_node *ufdep = (struct name_addr_node *)i;

			struct name_addr_node *fdep =
				name_addr_node_init(
					malloc(sizeof(struct name_addr_node)),
					ufdep->name, ufdep->addr + text_start_off);

			clist_queue(&out_u.foreign_deps, &fdep->hdr);
		}
	}

	//
	// fixup deps if they can be resolved
	//

	while(! clist_is_empty(&rel_patch_addrs)) {
		struct clnode *i = clnode_remove(clist_first(&rel_patch_addrs));
		struct name_addr_node *patch_dst = (struct name_addr_node *)i;

		struct name_addr_node *patch_src =
			name_addr_node_find(&out_u.exported, patch_dst->name);

		if(! patch_src) { // external dep
			clist_queue(&out_u.rel_deps, &patch_dst->hdr);
		} else {
			jump_t jump_target = patch_src->addr;
			jump_t jump_source = patch_dst->addr;
			jump_t jump = jump_target - jump_source;
			jump = htobe32(jump);
			memcpy(out_u.text + patch_dst->addr, &jump, sizeof(jump));

			free(i);
		}
	}


	while(! clist_is_empty(&abs_patch_addrs)) {
		struct clnode *i = clnode_remove(clist_first(&abs_patch_addrs));
		struct name_addr_node *patch_dst = (struct name_addr_node *)i;

		struct name_addr_node *patch_src =
			name_addr_node_find(&out_u.exported, patch_dst->name);

		if(! patch_src) { // external dep
			clist_queue(&out_u.abs_deps, &patch_dst->hdr);
		} else {
			jump_t jump = patch_src->addr;
			jump = htobe32(jump);
			memcpy(out_u.text + patch_dst->addr, &jump, sizeof(jump));

			free(i);
		}
	}

	//
	// emit object file
	//

	comp_unit_write(&out_u, outfile);

	return 0;
}
