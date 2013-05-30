#include <stdint.h>
#include "opcodes.h"
#include "interp.h"

instr_t program[] = 
{
	OP_PUSH8, 0x20,
	OP_PUSH8, 0x32,
	OP_ADD,
	OP_HALT,
};
