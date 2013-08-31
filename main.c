#include <stdlib.h>
#include <stdio.h>

#include "interp.h"

#if ! defined(DATA_STACK_DEPTH)
#define DATA_STACK_DEPTH 32
#endif
#if ! defined(CALL_STACK_DEPTH)
#define CALL_STACK_DEPTH 16
#endif

data_t data_stack[DATA_STACK_DEPTH];
size_t call_stack[CALL_STACK_DEPTH];
extern instr_t program[];

int main(int argc, char *argv[])
{
	size_t i, j;
	interp_state_t interp_state;

	j = interpret(program, data_stack, call_stack, NULL, &interp_state);

	printf("data stack:\n");
	for(i = 0; i < j; i++) {
		printf("%lu: 0x%llx %llu %lld\n",
	           i,
		       (unsigned long long) data_stack[i],
		       (unsigned long long) data_stack[i],
		       (signed long long) data_stack[i]);
	}

	return 0;
}
