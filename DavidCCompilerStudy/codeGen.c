#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "codeGen.h"
#include "parse.h"
#include "symbol.h"
#include "dynarray.h"
#include "dinput.h"

void pushReg(code_generator* codeGen, dcg_Reg reg) {
	dcg_Reg* tempReg = malloc(sizeof(dcg_Reg));  // Dont know if this is right
	*tempReg = reg;
	dd_push(codeGen->regStack, tempReg);
}

dcg_Reg popReg(code_generator* codeGen) {
	dcg_Reg * tempReg = dd_pop(codeGen->regStack);
	dcg_Reg temp = *(tempReg);
	free(tempReg);
	return temp;
}

void InitCodeGen(code_generator* codeGen, Parser* parser) {
	if (codeGen == NULL || parser == NULL) {
		fprintf(stderr, "Code Generator initiated with NULL values");
		exit(1);
		return;
	}
	codeGen->parser = parser;
	codeGen->fileState = malloc(sizeof(d_file));
	d_loadFile("MASM\\output.asm", codeGen->fileState, d_fileWrite);
	codeGen->OpTree_Root = NULL;
	codeGen->regStack = dd_makeDynamicArray();
	codeGen->stack_offset = 0;
	// Fill stack of registers cause all of them are for use
	// Push First temp Register
	// Push other temp Registers
	for (int i = dcg_COUNT - 1; i >= dcg_r10d; i--) {
		pushReg(codeGen, i);
	}
	pushReg(codeGen, dcg_ecx);
	pushReg(codeGen, dcg_eax);
}

void traverseAST(code_generator* codeGen, AST_Node* currentNode, int level) {
	AST_Node * nextNode;
	for (int j = 0; j < level; j++) {
		printf("  ");
	}
	
	printf("%s\n", currentNode->token->lexeme);
	if (currentNode->children != NULL) {
		for (int i = 0; i < currentNode->children->size; i++) {
			if ((nextNode = dd_get(currentNode->children, i)) != NULL) {
				traverseAST(codeGen, nextNode, level + 1);
			}
		}
	}
}

void freeOpNode(code_generator* codeGen, OpTreeNode** node) {
	if (*node != NULL) {
		if ((*node)->left != NULL) {
			freeOpNode(codeGen, (*node)->left);
		}
		if ((*node)->right != NULL) {
			freeOpNode(codeGen, (*node)->right);
		}

		free(*node);
		*node = NULL;
	}
}

/*
* Takes in an AST node and, from it, will build a asm OP tree. The tree is built for 
* Nodes to be written to a .asm file in Post Order Traversal
* The level parameter keeps track of the level within the binary tree
*/
OpTreeNode * buildOpTree(code_generator* codeGen, AST_Node* currentASTNode, int level) {
	AST_Node * nextASTNode;
	OpTreeNode* thisOpNode = malloc(sizeof(OpTreeNode));

	thisOpNode->left     = NULL;
	thisOpNode->right    = NULL;
	thisOpNode->code     = dcg_NOP;
	thisOpNode->ast_node = NULL;

	switch (currentASTNode->type) {
	case(AST_BLOCK):
		// Only generate the first child of a ast_block
		OpTreeNode* leftNode = NULL;
		//for (int i = currentASTNode->children->size - 1; i >= 0; i--) {
		for (int i = 0; i < currentASTNode->children->size; i++) {
			// Basicaly make a linked list of OpNodes in order
			AST_Node* childNode = (AST_Node*)dd_get(currentASTNode->children, i);
			thisOpNode = buildOpTree(codeGen, childNode, level + 1);
			// if There is an OpNode returned, currently isnt with variable declarations
			if (thisOpNode != NULL) {
				thisOpNode->left  = leftNode;
				thisOpNode->right = NULL;
				leftNode = thisOpNode;
			}
		}
		// Have to do this cause the last "thisOpNode" could have been NULL
		thisOpNode = leftNode;
		break;

	case(AST_INT):
		thisOpNode->code = dcg_MOV;
		thisOpNode->resultReg = popReg(codeGen);
		thisOpNode->ast_node = currentASTNode;

		break;
	case(AST_BIOP):

		// Set the Register for where the result goes
		thisOpNode->resultReg = dcg_none;

		// Get the children
		thisOpNode->left     = buildOpTree(codeGen, dd_get(currentASTNode->children, 0), level + 1);
		thisOpNode->right    = buildOpTree(codeGen, dd_get(currentASTNode->children, 1), level + 1);

		// Set the other attributes
		thisOpNode->code = dcg_MOV;
		thisOpNode->ast_node = currentASTNode;

		// If this binary operator is operating on two intergers, just calculate it and replace biop
		// in AST with the new INT literal
		if (thisOpNode->left->ast_node->type == AST_INT && thisOpNode->right->ast_node->type == AST_INT) {

			thisOpNode->ast_node->type = AST_INT;

			switch (currentASTNode->token->type) {
			case(dl_PLUS):
				thisOpNode->ast_node->int_literal = thisOpNode->left->ast_node->int_literal + thisOpNode->right->ast_node->int_literal;
				break;
			case(dl_MINUS):
				thisOpNode->ast_node->int_literal = thisOpNode->left->ast_node->int_literal - thisOpNode->right->ast_node->int_literal;
				break;
			case(dl_MULT):
				thisOpNode->ast_node->int_literal = thisOpNode->left->ast_node->int_literal * thisOpNode->right->ast_node->int_literal;
				break;
			case(dl_DIVIDE):
				if (thisOpNode->right->ast_node->int_literal != 0) {
					thisOpNode->ast_node->int_literal = thisOpNode->left->ast_node->int_literal / thisOpNode->right->ast_node->int_literal;
				}
				else {
					fprintf(stderr, "CodeGen: Illegal Divide by zero, line: %u\n", thisOpNode->right->ast_node->token->lineno);
				}
				break;
			default:
				fprintf(stderr, "CodeGen: Illegal BIOP Lexeme\n");
				exit(1);
				return NULL;
			}
			thisOpNode->ast_node->type = AST_INT;
			pushReg(codeGen, thisOpNode->right->resultReg);
			pushReg(codeGen, thisOpNode->left->resultReg);
			freeOpNode(codeGen, &(thisOpNode->left));
			freeOpNode(codeGen, &(thisOpNode->right));

		}
		// else, create a OpNode for the BIOP
		else {

			switch (currentASTNode->token->type) {
			case(dl_PLUS):
				thisOpNode->code = dcg_ADD;
				break;
			case(dl_MINUS):
				thisOpNode->code = dcg_SUB;
				break;
			case(dl_MULT):
				thisOpNode->code = dcg_MUL;
				break;
			case(dl_DIVIDE):
				thisOpNode->code = dcg_DIV;
				break;
			default:
				fprintf(stderr, "CodeGen: Illegal BIOP Lexeme");
				exit(1);
				return NULL;
			}
			pushReg(codeGen, thisOpNode->right->resultReg);
			pushReg(codeGen, thisOpNode->left->resultReg);
		} 
		// Assign a register for the result
		// Dont need the childrens registers anymore (since theyre just literals)
		// so we can push them back on, then pop one off for the biop
		thisOpNode->resultReg = popReg(codeGen);

		break;

	case(AST_IDENT):
		free(thisOpNode);
		thisOpNode = NULL;
		break;
	}
	return thisOpNode;
}

void appendReg(code_generator* codeGen, dcg_Reg Reg1) {
	switch (Reg1) {
	case(dcg_rax):
		d_appendString(codeGen->fileState, "rax");
		break;
	case(dcg_rbx):
		d_appendString(codeGen->fileState, "rbx");
		break;
	case(dcg_rcx):
		d_appendString(codeGen->fileState, "rcx");
		break;
	case(dcg_rdx):
		d_appendString(codeGen->fileState, "rdx");
		break;
	case(dcg_rsi):
		d_appendString(codeGen->fileState, "rsi");
		break;
	case(dcg_rdi):
		d_appendString(codeGen->fileState, "rdi");
		break;
	case(dcg_rbp):
		d_appendString(codeGen->fileState, "rbp");
		break;
	case(dcg_rsp):
		d_appendString(codeGen->fileState, "rsp");
		break;
	case(dcg_r8):
		d_appendString(codeGen->fileState, "r8");
		break;
	case(dcg_r9):
		d_appendString(codeGen->fileState, "r9");
		break;
	case(dcg_r10):
		d_appendString(codeGen->fileState, "r10");
		break;
	case(dcg_r11):
		d_appendString(codeGen->fileState, "r11");
		break;
	case(dcg_r12):
		d_appendString(codeGen->fileState, "r12");
		break;
	case(dcg_r13):
		d_appendString(codeGen->fileState, "r13");
		break;
	case(dcg_r14):
		d_appendString(codeGen->fileState, "r14");
		break;
	case(dcg_r15):
		d_appendString(codeGen->fileState, "r15");
		break;
	case(dcg_eax):
		d_appendString(codeGen->fileState, "eax");
		break;
	case(dcg_ebx):
		d_appendString(codeGen->fileState, "ebx");
		break;
	case(dcg_ecx):
		d_appendString(codeGen->fileState, "ecx");
		break;
	case(dcg_edx):
		d_appendString(codeGen->fileState, "edx");
		break;
	case(dcg_esi):
		d_appendString(codeGen->fileState, "esi");
		break;
	case(dcg_edi):
		d_appendString(codeGen->fileState, "edi");
		break;
	case(dcg_ebp):
		d_appendString(codeGen->fileState, "ebp");
		break;
	case(dcg_esp):
		d_appendString(codeGen->fileState, "esp");
		break;
	case(dcg_r8d):
		d_appendString(codeGen->fileState, "r8d");
		break;
	case(dcg_r9d):
		d_appendString(codeGen->fileState, "r9d");
		break;
	case(dcg_r10d):
		d_appendString(codeGen->fileState, "r10d");
		break;
	case(dcg_r11d):
		d_appendString(codeGen->fileState, "r11d");
		break;
	case(dcg_r12d):
		d_appendString(codeGen->fileState, "r12d");
		break;
	case(dcg_r13d):
		d_appendString(codeGen->fileState, "r13d");
		break;
	case(dcg_r14d):
		d_appendString(codeGen->fileState, "r14d");
		break;
	case(dcg_r15d):
		d_appendString(codeGen->fileState, "r15d");
		break;
	}
}
void appendOp(code_generator* codeGen, dcg_OpCode opCode) {
	switch (opCode) {
	case(dcg_MOV):
		d_appendString(codeGen->fileState, "mov   ");
		break;
	case(dcg_ADD):
		d_appendString(codeGen->fileState, "add   ");
		break;
	case(dcg_SUB):
		d_appendString(codeGen->fileState, "sub   ");
		break;
	case(dcg_MUL):
		d_appendString(codeGen->fileState, "mul   ");
		break;
	case(dcg_DIV):
		d_appendString(codeGen->fileState, "div   ");
		break;
	case(dcg_XOR):
		d_appendString(codeGen->fileState, "xor   ");
		break;
	case(dcg_PUSH):
		d_appendString(codeGen->fileState, "push  ");
		break;
	case(dcg_POP):
		d_appendString(codeGen->fileState, "pop   ");
		break;
	case(dcg_CALL):
		d_appendString(codeGen->fileState, "call  ");
		break;
	case(dcg_RET):
		d_appendString(codeGen->fileState, "ret   ");
		break;
	case(dcg_NOP):
		d_appendString(codeGen->fileState, "nop   ");
		break;
	}
}

void appendLineEndReg(code_generator* codeGen, dcg_OpCode opCode, dcg_Reg Reg1, dcg_Reg Reg2) {
	appendOp(codeGen, opCode);
	if (Reg1 != dcg_none)
		appendReg(codeGen, Reg1);
	if (Reg2 != dcg_none) {
		d_appendString(codeGen->fileState, ", ");
		appendReg(codeGen, Reg2);
	}
	d_appendString(codeGen->fileState, "\n");
}

void appendLineEndInt(code_generator* codeGen, dcg_OpCode opCode, dcg_Reg Reg1, int arg2) {
	appendOp(codeGen, opCode);
	if (Reg1 != dcg_none) {
		appendReg(codeGen, Reg1);
	}
	else {
		fprintf(stderr, "Reg1 cant be none in appendLineEndInt\n");
		exit(1);
		return;
	}
	d_appendString(codeGen->fileState, ", ");
	d_appendInt(codeGen->fileState, arg2);
	d_appendString(codeGen->fileState, "\n");
}

void printHeader(code_generator* codeGen) {
	/*
	const char * header = "extrn ExitProcess: PROC   ; external functions in system libraries\n"
						   ".code\n"
						   "Start PROC\n"
						   "push  rbp\n"
						   "mov   rbp, rsp\n"
						   "sub   rsp, 32\n";
	*/
	const char* header = "extrn ExitProcess: PROC   ; external functions in system libraries\n"
					     ".code\n";

	d_appendString(codeGen->fileState, header);
}

void printStartProc(code_generator* codeGen) {
	const char* proc = "Start PROC\n";

	d_appendString(codeGen->fileState, proc);
}

void printBeginningOfScope(code_generator* codeGen) {

	const char* bos  = "push  rbp\n"
					   "mov   rbp, rsp\n"
					   "sub   rsp, ";

	d_appendString(codeGen->fileState, bos);
	d_appendInt(codeGen->fileState, codeGen->stack_offset);
	d_appendString(codeGen->fileState, "\n");
}

void printFooter(code_generator* codeGen) {
	const char * footer = //"mov   rcx, rax    ; Stores the result\n"
					      "call ExitProcess\n"
		                  "Start ENDP\n"
		                  "END\n";

	d_appendString(codeGen->fileState, footer);
}

void writeOpTree(code_generator * codeGen, OpTreeNode * node) {
	if(node->left != NULL)
		writeOpTree(codeGen, node->left);
	if(node->right != NULL)
		writeOpTree(codeGen, node->right);

	switch (node->code) {
	case(dcg_MOV):
		// If were just moving an int literal into a register
		// This node could have children, this is because a BIOP node could've had both children been literal
		// so just evaluating the expression at compile time in the buildOpTree function. The children were not removed,
		// the biop AST_Node's type was just changed to AST_INT
		

		// NOTE: This may need to be moved down eventually. This is do ADD/SUB can do something like:
		// add  rax, 2
		// Dont think that can be done right now
		if (node->ast_node->type == AST_INT) {
			appendLineEndInt(codeGen, node->code, node->resultReg, node->ast_node->int_literal);
			break;
		}
	case(dcg_ADD):
	case(dcg_SUB):
		if (node->left->resultReg == node->resultReg) {
			// EX: ADD rax, r10  ; Code: 1+2;
			appendLineEndReg(codeGen, node->code, node->resultReg, node->right->resultReg);
		}
		else {
			appendLineEndReg(codeGen, node->code, node->left->resultReg, node->right->resultReg);
			appendLineEndReg(codeGen, dcg_MOV, node->resultReg, node->left->resultReg);
		}
		break;
	
	case(dcg_DIV):
		// zero edx register
		appendLineEndReg(codeGen, dcg_XOR, dcg_edx, dcg_edx);
	case(dcg_MUL):
		if (node->left->resultReg == node->resultReg) {
			// EX: ADD rax, r10  ; Code: 1+2;
			appendLineEndReg(codeGen, node->code, node->resultReg, node->right->resultReg);
		}
		else {
			appendLineEndReg(codeGen, node->code, node->left->resultReg, node->right->resultReg);
			appendLineEndReg(codeGen, dcg_MOV, node->resultReg, node->left->resultReg);
		}
		break;
	}

}

void print_local_var_offset_definition(code_generator* codeGen, const char * var_name, unsigned int var_size) {
	d_appendString(codeGen->fileState, var_name);
	d_appendString(codeGen->fileState, " = DWORD PTR ");
	codeGen->stack_offset += var_size;
	// This number will determine the amount of variables you can have
	/*
	char* num = calloc(10, sizeof(char));
	// ceil( log10 (..))) calcs length of a number
	// + 1 on end accounts for \0 char
	sprintf_s(num, (size_t)ceil(log10((double)var_size + 1)) + 1, "%u", codeGen->stack_offset);
	d_appendString(codeGen->fileState, num);
	*/
	d_appendInt(codeGen->fileState, var_size);
	d_appendString(codeGen->fileState, "\n");
}

void DefineLocalVars(code_generator* codeGen) {
	for (int i = 0; i < codeGen->parser->table->d_array->size; i++) {
		d_symbol* symbol = dd_get(codeGen->parser->table->d_array, i);
		if (symbol->type == dtype_INT) {
			print_local_var_offset_definition(codeGen, symbol->symbol_name, 4);
		}
	}
}

void GenCodeFromAST(code_generator* codeGen, AST_Node * currentNode) {
	// Build an op tree from the AST
	codeGen->OpTree_Root = buildOpTree(codeGen, currentNode, 0);
	// Write op tree to output file
	writeOpTree(codeGen, codeGen->OpTree_Root);
	// mov the value in the last used register into the return code register
	appendLineEndReg(codeGen, dcg_MOV, dcg_ecx, codeGen->OpTree_Root->resultReg);
}

void GenerateCode(code_generator* codeGen) {
	printHeader(codeGen);
	printStartProc(codeGen);
	DefineLocalVars(codeGen);
	printBeginningOfScope(codeGen);
	GenCodeFromAST(codeGen, codeGen->parser->ast_root);
	printFooter(codeGen);
}
