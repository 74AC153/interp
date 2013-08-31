#ifndef INTERP_H
#define INTERP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint64_t data_t;
typedef int64_t s_data_t;
typedef size_t pc_t;
typedef uint8_t instr_t;

typedef void (*foreign_t)(data_t **d_top, uint8_t *g_data);

typedef struct {
	pc_t pc;
	data_t *d_top;
	pc_t *c_top;
} interp_state_t;

bool interpret(
	instr_t *prog,
	data_t *d_stack,
	pc_t *c_stack,
	void *g_data,
	interp_state_t *state);

#endif
