#ifndef OPCODES_H
#define OPCODES_H

#define OPCODES \
	X(NOOP) \
	X(DBG) \
	X(HALT) \
	X(RET) \
	X(CALLIND) /* 0x04 */ \
	X(CALL)    /* 0x05 */ \
	X(CALLREL) /* 0x06) NB: CALL + 0x1 must be CALLREL */ \
	X(GOTOIND) /* 0x07 */ \
	X(GOTO)    /* 0x08 */ \
	X(GOTOREL) /* 0x09)  NB: GOTO + 0x1 must be GOTOREL */ \
	X(BRANCHZ) /* 0x0a */ \
	X(BRANCHNZ) \
	X(BRANCHH) \
	X(BRANCHNH) \
	X(WHERE) \
	X(PUSH8) \
	X(PUSH16) \
	X(PUSH32) /* 0x11 */ \
	X(PUSH64) /* 0x12 */ \
	X(FOREIGN) \
	X(POP) \
	X(ROT) \
	X(SWAP) \
	X(COPY) \
	X(SAVE) \
	X(ADD) \
	X(SUB) \
	X(MUL) \
	X(DIV) \
	X(REM) \
	X(SHL) \
	X(SHR) \
	X(SHRA) \
	X(AND) \
	X(OR) \
	X(NOT) \
	X(XOR) \
	X(LOAD8) \
	X(LOAD16) \
	X(LOAD32) \
	X(LOAD64) \
	X(STORE8) \
	X(STORE16) \
	X(STORE32) \
	X(STORE64)

#define X(op) OP_ ## op, 
typedef enum {
	OPCODES
} opcode_t;
#undef X

#endif
