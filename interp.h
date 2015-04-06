#ifndef INTERP_H
#define INTERP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint64_t data_t;
typedef int64_t s_data_t;
typedef uint32_t pc_t;
typedef int32_t jump_t;
typedef uint8_t instr_t;

typedef void (*foreign_t)(data_t **d_top);

typedef struct {
	pc_t pc;  // FIXME: this should be an address, not a uint32_t
	data_t *d_top;
	pc_t *c_top;
} interp_state_t;

int interpret(
	instr_t *prog,
	foreign_t *funcs,
	interp_state_t *state);

#endif
