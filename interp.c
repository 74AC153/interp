#include <string.h>
#include "interp.h"

bool interpret(
	instr_t *prog,
	data_t *d_stack,
	pc_t *c_stack,
	void *g_data,
	interp_state_t *state)
{
	bool status = true;
	pc_t pc = 0;
	data_t *d_top = d_stack - 1;
	pc_t *c_top = c_stack - 1;
	data_t temp = 0;

	/* if you change this, you must also change opcodes.h */
	static const void *const dispatch[] = {
		&&do_err, &&do_halt,
		&&do_call, &&do_callind, &&do_ret, &&do_where, &&do_goto, &&do_gotoind,
		&&do_skipz, &&do_skipnz, &&do_skip,
		&&do_push8, &&do_push16, &&do_push32, &&do_push64,
		&&do_pop, &&do_rot, &&do_swap, &&do_copy, &&do_save,
		&&do_add, &&do_sub, &&do_mul, &&do_div, &&do_rem,
		&&do_shl, &&do_shr, &&do_shra,
		&&do_and, &&do_or, &&do_not, &&do_xor,
		&&do_load8, &&do_load16, &&do_load32, &&do_load64,
		&&do_store8, &&do_store16, &&do_store32, &&do_store64,
		&&do_sex8, &&do_sex16, &&do_sex32,
		&&do_foreign, &&do_foreignind,
	};
#if defined(FASTER)
	#define CYCLE goto *dispatch[prog[pc++]]

	CYCLE;
#else /* SMALLER */
	#define CYCLE goto cycle

	cycle:
		goto *dispatch[prog[pc++]];
#endif
	/* data stack operations */
	#define ARG(N) (*(d_top - (N)))
	#define SHIFT() do { ++d_top; } while(0)
	#define UNSHIFT(N) do { d_top -= (N); } while(0)
	#define PUSH(VAL) do { *(++d_top) = (VAL); } while(0)
	#define READ1() do { *(++d_top) = *(uint8_t *)(prog + pc); } while(0)
	#define READ2() do { *(++d_top) = *(uint16_t *)(prog + pc); } while(0)
	#define READ4() do { *(++d_top) = *(uint32_t *)(prog + pc); } while(0)
	#define READ8() do { *(++d_top) = *(uint64_t *)(prog + pc); } while(0)
	#define READPC() do { *(++d_top) = *(pc_t *)(prog + pc); } while(0)
	#define READPTR() do { *(++d_top) = *(uintptr_t *)(prog + pc); } while(0)

	/* data stack <--> pc operations */
	#define WHERE() (void) (*(++d_top) = pc)
	#define GOTO() (void) (pc = *(d_top--))

	/* pc operations */
	#define SKIP(N) (void) (pc += (N))

	/* call stack operations */
	#define SAVE() (void) (*(++c_top) = pc)
	#define RESTORE() (void) (pc = (*(c_top--)))

	do_err:
		status = false;

	do_halt:
		goto finish;

	do_call:
		READPC();
		/* goto do_callind; */
		
	do_callind:
		SAVE();
		GOTO();
		CYCLE;

	do_ret:
		RESTORE();
		CYCLE;

	do_where:
		WHERE();
		CYCLE;

	do_goto:
		READPC();
		/* goto do_gotoind; */

	do_gotoind:
		GOTO();
		CYCLE;

	do_skipz:
		if(! ARG(0)) {
			SKIP((signed) prog[pc]);
		}
		SKIP(1);
		UNSHIFT(1);
		CYCLE;

	do_skipnz:
		if(ARG(0)) {
			SKIP((signed) prog[pc]);
		}
		SKIP(1);
		UNSHIFT(1);
		CYCLE;

	do_skip:
		SKIP((signed) prog[pc]);
		SKIP(1);
		CYCLE;

	do_push8:
		READ1();
		CYCLE;

	do_push16:
		READ2();
		CYCLE;

	do_push32:
		READ4();
		CYCLE;

	do_push64:
		READ8();
		CYCLE;

	do_pop:
		UNSHIFT(1);
		CYCLE;

	do_rot:
		temp = ARG(2);
		ARG(2) = ARG(1);
		ARG(1) = ARG(0);
		ARG(0) = temp;
		CYCLE;

	do_swap:
		temp = ARG(0);
		ARG(0) = ARG(1);
		ARG(1) = temp;
		CYCLE;

	do_copy:
		ARG(0) = ARG(ARG(0));
		CYCLE;

	do_save:
		ARG(ARG(0)) = ARG(1);
		CYCLE;

	do_add:
		ARG(1) = ARG(0) + ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_sub:
		ARG(1) = ARG(0) - ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_mul:
		ARG(1) = ARG(0) * ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_div:
		ARG(1) = ARG(0) / ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_rem:
		ARG(1) = ARG(0) % ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_shl:
		ARG(1) = ARG(0) << ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_shr:
		ARG(1) = ARG(0) >> ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_shra:
		ARG(1) = (s_data_t) ARG(0) >> ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_and:
		ARG(1) = ARG(0) & ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_or:
		ARG(1) = ARG(0) | ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_not:
		ARG(0) = ~ARG(0);
		CYCLE;

	do_xor:
		ARG(1) = ARG(0) ^ ARG(1);
		UNSHIFT(1);
		CYCLE;

	do_load8:
		SHIFT();
		ARG(0) = *(uint8_t *)(g_data + ARG(1));
		CYCLE;

	do_load16:
		SHIFT();
		ARG(0) = *(uint16_t *)(g_data + ARG(1));
		CYCLE;

	do_load32:
		SHIFT();
		ARG(0) = *(uint32_t *)(g_data + ARG(1));
		CYCLE;

	do_load64:
		SHIFT();
		ARG(0) = *(uint64_t *)(g_data + ARG(1));
		CYCLE;
	
	do_store8:
		*(uint8_t*)(g_data + ARG(0)) = ARG(1);
		CYCLE;

	do_store16:
		*(uint16_t*)(g_data + ARG(0)) = ARG(1);
		CYCLE;

	do_store32:
		*(uint32_t*)(g_data + ARG(0)) = ARG(1);
		CYCLE;

	do_store64:
		*(uint64_t*)(g_data + ARG(0)) = ARG(1);
		CYCLE;

	do_sex8:
		ARG(0) = ((s_data_t)ARG(0) << 56) >> 56;
		CYCLE;

	do_sex16:
		ARG(0) = ((s_data_t)ARG(0) << 48) >> 48;
		CYCLE;

	do_sex32:
		ARG(0) = ((s_data_t)ARG(0) << 32) >> 32;
		CYCLE;

	do_foreign:
		READPTR();
		/* goto do_foreignind; */

	do_foreignind:
		temp = ARG(0);	
		UNSHIFT(1);
		(*(foreign_t)temp)(&d_top, g_data);
		CYCLE;

finish:
	state->pc = pc;
	state->d_top = d_top;
	state->c_top = c_top;
	return status;
}
