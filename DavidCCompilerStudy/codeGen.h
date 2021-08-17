#include "parse.h"
#include "dinput.h"
#include "dynarray.h"

typedef enum {
	dcg_rax	  = 0,
	dcg_rbx	  = 1,
	dcg_rcx	  = 2,
	dcg_rdx	  = 3,
	dcg_rsi	  = 4,
	dcg_rdi	  = 5,
	dcg_rbp	  = 6,
	dcg_rsp	  = 7,
	dcg_r8	  = 8,
	dcg_r9	  = 9,
	dcg_r10	  = 10,
	dcg_r11	  = 11,
	dcg_r12	  = 12,
	dcg_r13	  = 13,
	dcg_r14	  = 14,
	dcg_r15	  = 15,
	dcg_eax	  = 16,
	dcg_ebx	  = 17,
	dcg_ecx	  = 18, 
	dcg_edx	  = 19,
	dcg_esi	  = 20,
	dcg_edi	  = 21,
	dcg_ebp	  = 22,
	dcg_esp	  = 23,
	dcg_r8d	  = 24,
	dcg_r9d	  = 25,
	dcg_r10d	  = 26,
	dcg_r11d	  = 27,
	dcg_r12d	  = 28,
	dcg_r13d	  = 29,
	dcg_r14d	  = 30,
	dcg_r15d	  = 31,
	dcg_COUNT = 32,
	dcg_none  = -1,
} dcg_Reg;

typedef enum {
	dcg_MOV = 0,
	dcg_ADD,
	dcg_ADC,
	dcg_SUB,
	dcg_SBC,
	dcg_MUL,
	dcg_DIV,
	dcg_XOR,
	dcg_PUSH,
	dcg_POP,
	dcg_CALL,
	dcg_RET,
	dcg_NOP
} dcg_OpCode;

typedef struct OpTreeNode{
	struct OpTreeNode* left;
	struct OpTreeNode* right;
	dcg_OpCode         code;
	AST_Node*          ast_node;
	dcg_Reg            resultReg;
} OpTreeNode;

typedef struct code_generator {
	Parser* parser;
	d_file* fileState;
	OpTreeNode* OpTree_Root;
	DynamicArray * regStack;
	unsigned int stack_offset;
} code_generator;

void InitCodeGen(code_generator* codeGen, Parser* parser);
void traverseAST(code_generator* codeGen, AST_Node* currentNode, int level);
void GenerateCode(code_generator* codeGen);