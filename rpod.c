#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "rputils.h"
#include "interp.h"
#include "clist.h"
#include "opcodes.h"

#define X(op) #op, 
const char *opnames[256] = {
	OPCODES
};
#undef X

#include "rputils.h"

int main(int argc, char *argv[])
{
	if(argc != 2) {
		printf("usage: %s <infile>\n", argv[0]);
		return -1;
	}

	struct comp_unit *cu = comp_unit_read(argv[1]);
	if(! cu)
		return -1;

	printf("exported functions:\n");
	for(struct clnode *i = clist_first(&cu->exported);
	    i != clist_end(&cu->exported);
	    i = clnode_next(i)) {
		struct name_addr_node *nan = (struct name_addr_node *)i;
		printf("%s @ 0x%x\n", nan->name, nan->addr);
	}

	printf("foreign functions:\n");
	for(struct clnode *i = clist_first(&cu->foreign_deps);
	    i != clist_end(&cu->foreign_deps);
	    i = clnode_next(i)) {
		struct name_addr_node *nan = (struct name_addr_node *)i;
		printf("%s @ 0x%x\n", nan->name, nan->addr);
	}

	printf("external abs-jump functions:\n");
	for(struct clnode *i = clist_first(&cu->abs_deps);
	    i != clist_end(&cu->abs_deps);
	    i = clnode_next(i)) {
		struct name_addr_node *nan = (struct name_addr_node *)i;
		printf("%s @ 0x%x\n", nan->name, nan->addr);
	}

	printf("external rel-jump functions:\n");
	for(struct clnode *i = clist_first(&cu->rel_deps);
	    i != clist_end(&cu->rel_deps);
	    i = clnode_next(i)) {
		struct name_addr_node *nan = (struct name_addr_node *)i;
		printf("%s @ 0x%x\n", nan->name, nan->addr);
	}


	// disassembly
	printf("program text:\n");
	unsigned char *cursor = cu->text;
	while((size_t)(cursor - cu->text) < cu->text_len) {
		const char *opname = opnames[*cursor];
		printf("0x%.8lx: %s",
		       cursor - cu->text,
		       opname ? opname : "(undefined)");

		switch(*cursor++) {
		case OP_NOOP:
		case OP_DBG:
		case OP_HALT:
		case OP_RET:
		case OP_CALLIND:
		case OP_GOTOIND:
		case OP_WHERE:
#if 0
		case OP_POP:
		case OP_ROT:
		case OP_SWAP:
		case OP_COPY:
		case OP_SAVE:
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_REM:
		case OP_SHL:
		case OP_SHR:
		case OP_SHRA:
		case OP_AND:
		case OP_OR:
		case OP_NOT:
		case OP_XOR:
		case OP_LOAD8:
		case OP_LOAD16:
		case OP_LOAD32:
		case OP_LOAD64:
		case OP_STORE8:
		case OP_STORE16:
		case OP_STORE32:
		case OP_STORE64:
		case OP_SEX8:
		case OP_SEX16:
		case OP_SEX32:
#endif
			break;

		case OP_PUSH8:
			{
				uint8_t arg;
				memcpy(&arg, cursor, sizeof(arg));
				printf(" 0x%.2x", arg);
				cursor += sizeof(arg);
			}
			break;

		case OP_PUSH16:
			{
				uint16_t arg;
				memcpy(&arg, cursor, sizeof(arg));
				printf(" 0x%.4x", be16toh(arg));
				cursor += sizeof(arg);
			}
			break;

		case OP_PUSH32:
		case OP_CALL:
		case OP_GOTO:
		case OP_FOREIGN:
			{
				uint32_t arg;
				memcpy(&arg, cursor, sizeof(arg));
				printf(" 0x%.8x", be32toh(arg));
				cursor += sizeof(arg);
			}
			break;

		case OP_CALLREL:
		case OP_GOTOREL:
		case OP_BRANCHZ:
		case OP_BRANCHNZ:
		case OP_BRANCHH:
		case OP_BRANCHNH:
			{
				int32_t arg;
				memcpy(&arg, cursor, sizeof(arg));
				arg = be32toh(arg);
				printf(" 0x%.8x (%s%d)", (unsigned) arg, arg > 0 ? "+" : "", arg);
				cursor += sizeof(arg);
			}
			break;

		case OP_PUSH64:
			{
				uint64_t arg;
				memcpy(&arg, cursor, sizeof(arg));
				printf(" 0x%.16llx", (unsigned long long)(be64toh(arg)));
				cursor += sizeof(arg);
			}
			break;
		}
		printf("\n");
	}

	return 0;
}
