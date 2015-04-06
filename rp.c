#include <string.h>
#include <endian.h>
#include <stdlib.h>

#include "rputils.h"

static long _linux_syscall_0(
	long num)
{
	long status;
	__asm__ ("movq %1, %%rax;"
	     "syscall;"
	     "movq %%rax, %0;"
	      : "=r"(status)
	      : "g"(num)
	      : "%rcx", "%r11", "%rax");
	return status;
}

static long _linux_syscall_1(
	long num, long a1)
{
	long status;
	__asm__ ("movq %1, %%rax;"
	    "movq %2, %%rdi;"
	    "syscall;"
	    "movq %%rax, %0;"
	    : "=r"(status)
	    : "g"(num), "g"(a1)
	    : "%rcx", "%r11", "%rax", "%rdi");
	return status;
}

static long _linux_syscall_2(
	long num, long a1, long a2)
{
	long status;
	__asm__("movq %1, %%rax;"
	    "movq %2, %%rdi;"
	    "movq %3, %%rsi;"
	    "syscall;"
	    "movq %%rax, %0;"
	    : "=r"(status)
	    : "g"(num), "g"(a1), "g"(a2)
	    : "%rcx", "%r11", "%rax", "%rdi", "%rsi");
	return status;
}

static long _linux_syscall_3(
	long num, long a1, long a2, long a3)
{
	long status;
	__asm__("movq %1, %%rax;"
	    "movq %2, %%rdi;"
	    "movq %3, %%rsi;"
	    "movq %4, %%rdx;"
	    "syscall;"
	    "movq %%rax, %0;"
	    : "=r"(status)
	    : "g"(num), "g"(a1), "g"(a2), "g"(a3)
	    : "%rcx", "%r11", "%rax", "%rdi", "%rsi", "%rdx");
	return status;
}

static long _linux_syscall_4(
	long num, long a1, long a2, long a3, long a4)
{
	long status;
	__asm__("movq %1, %%rax;"
	    "movq %2, %%rdi;"
	    "movq %3, %%rsi;"
	    "movq %4, %%rdx;"
	    "movq %5, %%rcx;"
	    "syscall;"
	    "movq %%rax, %0;"
	    : "=r"(status)
	    : "g"(num), "g"(a1), "g"(a2), "g"(a3), "g"(a4)
	    : "%rcx", "%r11", "%rax", "%rdi", "%rsi", "%rdx", "%rcx");
	return status;
}

static long _linux_syscall_5(
	long num, long a1, long a2, long a3, long a4, long a5)
{
	long status;
	__asm__("movq %1, %%rax;"
	    "movq %2, %%rdi;"
	    "movq %3, %%rsi;"
	    "movq %4, %%rdx;"
	    "movq %5, %%rcx;"
	    "movq %6, %%r8;"
	    "syscall;"
	    "movq %%rax, %0;"
	    : "=r"(status)
	    : "g"(num), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5)
	    : "%rcx", "%r11", "%rax", "%rdi", "%rsi", "%rdx", "%rcx", "%r8");
	return status;
}

static long _linux_syscall_6(
	long num, long a1, long a2, long a3, long a4, long a5, long a6)
{
	long status;
	__asm__("movq %1, %%rax;"
	    "movq %2, %%rdi;"
	    "movq %3, %%rsi;"
	    "movq %4, %%rdx;"
	    "movq %5, %%rcx;"
	    "movq %6, %%r8;"
	    "movq %7, %%r9;"
	    "syscall;"
	    "movq %%rax, %0;"
	    : "=r"(status)
	    : "g"(num), "g"(a1), "g"(a2), "g"(a3), "g"(a4), "g"(a5), "g"(a6)
	    : "%rcx", "%r11", "%rax", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9");
	return status;
}

static void linux_x86_64_sys0(data_t **d_top)
{
	(*d_top)[0] =
		_linux_syscall_0((*d_top)[0]);
}

static void linux_x86_64_sys1(data_t **d_top)
{
	(*d_top)[0] =
		_linux_syscall_1((*d_top)[0],
			(*d_top)[-1]);
}

static void linux_x86_64_sys2(data_t **d_top)
{
	(*d_top)[0] =
		_linux_syscall_2((*d_top)[0],
			(*d_top)[-1],
			(*d_top)[-2]);
}

static void linux_x86_64_sys3(data_t **d_top)
{
	(*d_top)[0] =
		_linux_syscall_3((*d_top)[0],
			(*d_top)[-1],
			(*d_top)[-2],
			(*d_top)[-3]);
}

static void linux_x86_64_sys4(data_t **d_top)
{
	(*d_top)[0] =
		_linux_syscall_4((*d_top)[0],
			(*d_top)[-1],
			(*d_top)[-2],
			(*d_top)[-3],
			(*d_top)[-4]);
}

static void linux_x86_64_sys5(data_t **d_top)
{
	(*d_top)[0] =
		_linux_syscall_5((*d_top)[0],
			(*d_top)[-1],
			(*d_top)[-2],
			(*d_top)[-3],
			(*d_top)[-4],
			(*d_top)[-5]);
}

static void linux_x86_64_sys6(data_t **d_top)
{
	(*d_top)[0] =
		_linux_syscall_6((*d_top)[0],
			(*d_top)[-1],
			(*d_top)[-2],
			(*d_top)[-3],
			(*d_top)[-4],
			(*d_top)[-5],
			(*d_top)[-6]);
}

#define ARRLEN(X) ( sizeof(X) / sizeof((X)[0]) )

char *builtin_names[] = {
	"linux_x86_64_sys0",
	"linux_x86_64_sys1",
	"linux_x86_64_sys2",
	"linux_x86_64_sys3",
	"linux_x86_64_sys4",
	"linux_x86_64_sys5",
	"linux_x86_64_sys6",
	NULL
};

foreign_t builtin_funcs[] = {
	linux_x86_64_sys0,
	linux_x86_64_sys1,
	linux_x86_64_sys2,
	linux_x86_64_sys3,
	linux_x86_64_sys4,
	linux_x86_64_sys5,
	linux_x86_64_sys6,
};

int main(int argc, char *argv[])
{
	if(argc < 2) {
		printf("usage: %s <infile> args...\n", argv[0]);
		return -1;
	}

	struct comp_unit *u = comp_unit_read(argv[1]);

	// we can't execute if there are any unsresolved syms

	// patch up foreign function addrs

	for(struct clnode *i = clist_first(&u->foreign_deps);
	    i != clist_end(&u->foreign_deps);
	    i = clnode_next(i)) {
		struct name_addr_node *dep = (struct name_addr_node *)i;

		// find matching label
		uint32_t j;
		for(j = 0; j < ARRLEN(builtin_names); j++) {
			if(strcmp(builtin_names[j], dep->name) == 0)
				break;
		}

		if(j == ARRLEN(builtin_names)) {
			// unresolved foreign -- can't execute
			printf("error unresolved foreign: %s\n", dep->name);
			return -1;
		} else {
			// match found: symbol is internal:
			// patch up jump target and make jump relative
			jump_t src = dep->addr;
			j = htobe32(j);
			memcpy(u->text + src, &j, sizeof(j));
		}
	}

	// find entrypoint
	pc_t entry = 0;
	struct clnode *i;
	for(i = clist_first(&u->exported);
	    i != clist_end(&u->exported);
	    i = clnode_next(i)) {
		struct name_addr_node *dep = (struct name_addr_node *)i;
		if(strcmp(dep->name, "exec-start") == 0) {
			entry = dep->addr;
			break;
		}
	}
	if(i == clist_end(&u->exported)) {
		printf("no exec-start export specified\n");
		return -1;
	}

	// run
	interp_state_t state;
	data_t d_stack[1024];
	pc_t c_stack[1024];
	state.pc = entry;
	state.d_top = &d_stack[0] - 1;
	state.c_top = &c_stack[0] - 1;

	if(interpret(u->text, builtin_funcs, &state))
		abort();

	return d_stack[0];
}
