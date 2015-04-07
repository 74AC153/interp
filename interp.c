#include <endian.h>

#include "interp.h"

data_t read_be(unsigned char *loc, unsigned sz)
{
	data_t out = *loc++;
	while(--sz)
		out = (out << 8) | *loc++;
	return out;
}

void write_be(unsigned char *loc, unsigned sz, data_t val)
{
	while(sz--) {
		loc[sz] = val;
		val >>= 8;
	}
}

int interpret(
	instr_t *prog,
	foreign_t *funcs,
	interp_state_t *state)
{
	int status = 0;

	pc_t pc = state->pc;
	data_t *d_top = state->d_top;
	pc_t *c_top = state->c_top;

	data_t data;
	jump_t jump;
	

	/* if you change this, you must also change opcodes.h */
	static const void *const dispatch[] = {
		&&do_noop, &&do_dbg, &&do_halt,
		&&do_ret,
		&&do_call_ind, &&do_call, &&do_call_rel,
		&&do_goto_ind, &&do_goto, &&do_goto_rel,
		&&do_branch_z, &&do_branch_nz, &&do_branch_h, &&do_branch_nh,
		&&do_where,
		&&do_push8, &&do_push16, &&do_push32, &&do_push64,
		&&do_foreign,
		&&do_pop, &&do_rot, &&do_swap, &&do_copy, &&do_save,
		&&do_add, &&do_sub, &&do_mul, &&do_div, &&do_rem,
		&&do_shl, &&do_shr, &&do_shra,
		&&do_and, &&do_or, &&do_not, &&do_xor,
		&&do_load8, &&do_load16, &&do_load32, &&do_load64,
		&&do_store8, &&do_store16, &&do_store32, &&do_store64,
	};
#if defined(FASTER)
	#define CYCLE goto *dispatch[prog[pc++]]

	CYCLE;
#else /* SMALLER */
	#define CYCLE goto cycle

	cycle:
		goto *dispatch[prog[pc++]];
#endif

	do_noop:
		CYCLE;

	do_dbg:
		status = -1;

	do_halt:
		goto finish;
		
	do_ret:
		pc = *c_top--; // pc <- pop c_top
		CYCLE;

	do_call_ind:
		*(++c_top) = pc; // push pc -> c_top
		pc = *(d_top--); // pc <- pop d_top
		CYCLE;

	do_call:
		*(++c_top) = pc + 4; // push pc -> c_top
		jump = read_be(prog + pc, 4);
		pc = jump; // pc <- pop d_top
		CYCLE;

	do_call_rel:
		*(++c_top) = pc + 4; // push pc -> c_top
		jump = read_be(prog + pc, 4);
		pc += jump; // pc <- pop d_top
		CYCLE;

	do_goto_ind:
		pc = *(d_top--); // pc = pop d_top
		CYCLE;

	do_goto:
		jump = read_be(prog + pc, 4);
		pc = jump;
		CYCLE;

	do_goto_rel:
		jump = read_be(prog + pc, 4);
		pc += jump;
		CYCLE;

	do_branch_z:
		if(! *d_top) {
			goto do_goto_rel;
		}
		pc += sizeof(jump_t); // jump over immediate
		CYCLE;
		
	do_branch_nz:
		if(*d_top) {
			goto do_goto_rel;
		}
		pc += sizeof(jump_t);
		CYCLE;

	do_branch_h:
		if((s_data_t) *d_top < 0) {
			goto do_goto_rel;
		}
		pc += sizeof(jump_t);
		CYCLE;

	do_branch_nh:
		if((s_data_t) *d_top >= 0) {
			goto do_goto_rel;
		}
		pc += sizeof(jump_t);
		CYCLE;

	do_where:
		*(++d_top) = pc; // push pc -> d_top
		CYCLE;

	do_push8:
		*(++d_top) = read_be(prog + pc, 1);
		pc += 1;
		CYCLE;

	do_push16:
		*(++d_top) = read_be(prog + pc, 2);
		pc += 2;
		CYCLE;

	do_push32:
		*(++d_top) = read_be(prog + pc, 4);
		pc += 4;
		CYCLE;

	do_push64:
		*(++d_top) = read_be(prog + pc, 8);
		pc += 8;
		CYCLE;

	do_foreign:
		data = read_be(prog + pc, 4);
		funcs[data](&d_top);
		pc += 4;
		CYCLE;

	do_pop:
		d_top--;
		CYCLE;

	do_rot:
		data = d_top[-2];
		d_top[-2] = d_top[-1];
		d_top[-1] = d_top[0];
		d_top[0] = data;
		CYCLE;

	do_swap:
		data = d_top[0];
		d_top[0] = d_top[-1];
		d_top[-1] = data;
		CYCLE;

	do_copy:
		data = *d_top;
		*d_top = d_top[-(s_data_t)data];
		CYCLE;

	do_save:
		data = *d_top;
		d_top[-(s_data_t)data] = d_top[-1];
		CYCLE;

	do_add:
		d_top[-1] += d_top[0];
		d_top--;
		CYCLE;

	do_sub:
		d_top[-1] -= d_top[0];
		d_top--;
		CYCLE;

	do_mul:
		d_top[-1] *= d_top[0];
		d_top--;
		CYCLE;

	do_div:
		d_top[-1] /= d_top[0];
		d_top--;
		CYCLE;

	do_rem:
		d_top[-1] %= d_top[0];
		d_top--;
		CYCLE;

	do_shl:
		d_top[-1] <<= d_top[0];
		d_top--;
		CYCLE;

	do_shr:
		d_top[-1] >>= d_top[0];
		d_top--;
		CYCLE;

	do_shra:
		d_top[-1] = ((s_data_t*)d_top)[-1] >> d_top[0];
		d_top--;
		CYCLE;

	do_and:
		d_top[-1] &= d_top[0];
		d_top--;
		CYCLE;

	do_or:
		d_top[-1] |= d_top[0];
		d_top--;
		CYCLE;

	do_not:
		d_top[0] = ~d_top[0];
		CYCLE;

	do_xor:
		d_top[-1] ^= d_top[0];
		d_top--;
		CYCLE;

	do_load8:
		// addr -> addr val    val = mem[addr]
		++d_top;
		d_top[0] = read_be(prog + d_top[-1], 1);
		CYCLE;

	do_load16:
		++d_top;
		d_top[0] = read_be(prog + d_top[-1], 2);
		CYCLE;

	do_load32:
		++d_top;
		d_top[0] = read_be(prog + d_top[-1], 4);
		CYCLE;

	do_load64:
		++d_top;
		d_top[0] = read_be(prog + d_top[-1], 8);
		CYCLE;
	
	do_store8:
		// addr val -> addr val    mem[addr] = val
		write_be(prog + d_top[-1], 1, d_top[0]);
		CYCLE;

	do_store16:
		write_be(prog + d_top[-1], 2, d_top[0]);
		CYCLE;

	do_store32:
		write_be(prog + d_top[-1], 4, d_top[0]);
		CYCLE;

	do_store64:
		write_be(prog + d_top[-1], 8, d_top[0]);
		CYCLE;

finish:
	state->pc = pc;
	state->d_top = d_top;
	state->c_top = c_top;
	return status;
}
