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
	bool status;
	interp_state_t interp_state;
	data_t *ds_ptr;

	status = interpret(program, 0, data_stack, call_stack, NULL, &interp_state);

	printf("data stack:\n");
	for(ds_ptr = interp_state.ds_top; ds_ptr >= data_stack; ds_ptr--) {
		printf("%lu: 0x%llx %llu %lld\n",
	           i,
		       (unsigned long long) *ds_ptr,
		       (unsigned long long) *ds_ptr,
		       (signed long long) *ds_ptr);
	}

	return 0;
}
