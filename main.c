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
pc_t call_stack[CALL_STACK_DEPTH];
extern instr_t program[];

int main(void)
{
	size_t i;
	bool status;
	interp_state_t interp_state = {
		.pc = 0,
		.d_top = data_stack,
		.c_top = call_stack
	};
	data_t *ds_ptr;

	status = interpret(program, NULL, &interp_state);
	if(status)
		printf("abnormal termination\n");

	printf("data stack:\n");
	for(i = 0, ds_ptr = interp_state.d_top; ds_ptr >= data_stack; ds_ptr--, i++) {
		printf("%lu: 0x%llx %llu %lld\n",
		       i,
		       (unsigned long long) *ds_ptr,
		       (unsigned long long) *ds_ptr,
		       (signed long long) *ds_ptr);
	}

	return 0;
}
