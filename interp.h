#ifndef INTERP_H
#define INTERP_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t data_t;
typedef int64_t s_data_t;
typedef size_t pc_t;
typedef uint8_t instr_t;

size_t interpret(uint8_t *prog, data_t *d_stack, size_t *c_stack);

#endif
