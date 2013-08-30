#ifndef INTERP_H
#define INTERP_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t data_t;
typedef int64_t s_data_t;
typedef size_t pc_t;
typedef uint8_t instr_t;

typedef void (*foreign_t)(data_t **d_top, uint8_t *g_data);

typedef struct {
	size_t pc;
	data_t *d_top;
	size_t *c_top;
} interp_state_t;

bool interpret(
	instr_t *prog,
	data_t *d_stack,
	size_t *c_stack,
	uint8_t *g_data,
	interp_state_t *state);

#endif
