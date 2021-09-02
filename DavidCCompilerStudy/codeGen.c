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
#include "regAlloc.h"

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

void resetRegs(code_generator* codeGen) {
	// Fill stack of registers cause all of them are for use
	// Push First temp Registers
	BOOL containsReg = FALSE;
	for (int i = dcg_COUNT - 1; i >= dcg_r10d; i--) {
		containsReg = FALSE;
		for (int j = 0; j < codeGen->regStack->size; j++) {
			dcg_Reg* temp = dd_get(codeGen->regStack, j);
			if (*temp == i) {
				containsReg = TRUE;
				continue;
			}
		}
		if (!containsReg)
			pushReg(codeGen, i);
	}
	// Push other temp Registers
	containsReg = FALSE;
	for (int j = 0; j < codeGen->regStack->size; j++) {
		dcg_Reg* temp = dd_get(codeGen->regStack, j);
		if (*temp == dcg_ecx) {
			containsReg = TRUE;
			continue;
		}
	}
	if (!containsReg)
		pushReg(codeGen, dcg_ecx);
	containsReg = FALSE;
	for (int j = 0; j < codeGen->regStack->size; j++) {
		dcg_Reg* temp = dd_get(codeGen->regStack, j);
		if (*temp == dcg_eax) {
			containsReg = TRUE;
			continue;
		}
	}
	if (!containsReg)
		pushReg(codeGen, dcg_eax);
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
	codeGen->regAlloc = malloc(sizeof(RegAlloc));
	initRegAlloc(codeGen->regAlloc);
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
	switch (currentNode->type) {
	case(AST_BLOCK):
		printf("<AST_BLOCK>: ");
		break;
	case(AST_DECL):
		printf("<AST_DECL>: ");
		break;
	case(AST_IDENT):
		printf("<AST_IDENT>: ");
		break;
	case(AST_INT):
		printf("<AST_INT>: ");
		break;
	case(AST_BIOP):
		printf("<AST_BIOP>: ");
		break;
	case(AST_UNKNOWN):
		printf("<AST_UNKNOWN>: ");
		break;
	case(AST_RETURN):
		printf("<AST_RETURN>: ");
		break;
	default:
		printf("<Unknown>: ");
		break;
	}
	
	printf("%s\n", currentNode->token->lexeme);
	if (currentNode->children != NULL) {
		for (unsigned int i = 0; i < currentNode->children->size; i++) {
			if ((nextNode = dd_get(currentNode->children, i)) != NULL) {
				traverseAST(codeGen, nextNode, level + 1);
			}
		}
	}
}

void freeOpNode(code_generator* codeGen, OpTreeNode** node) {
	if (*node != NULL) {
		if ((*node)->left != NULL) {
			freeOpNode(codeGen, &((*node)->left));
		}
		if ((*node)->right != NULL) {
			freeOpNode(codeGen, &((*node)->right));
		}

		free(*node);
		*node = NULL;
	}
}

/*
*	If this node was storing a result in a Register
*	This function will push that register back onto the available register stack
*/
void pushUsedReg(code_generator* codeGen, OpTreeNode* node) {
	// Check first 4 bits for the REG arg type
	if (node->arg_struct->arg_types & dcg_OpCodeArgType_REG) {
		pushReg(codeGen, node->arg_struct->OpArg1.arg_reg);
	}
}

/* 
*    arg_types is an unsigned char
*    the first 4 bits of the unsigned char will determine the first arguments type
*    the second 4 bits of the unsigned char will determine the second arguments type
*/
void setArgTypes(unsigned char *arg_types, dcg_OpCodeArgType arg1, dcg_OpCodeArgType arg2) {

	// set first 4 bits
	*arg_types |= arg1;
	// set second 4 bytes
	*arg_types |= (arg2 << 4);
}

/*
* Takes in an AST node and, from it, will build a asm OP tree. The tree is built for 
* Nodes to be written to a .asm file in Post Order Traversal
* The level parameter keeps track of the level within the binary tree
*/
OpTreeNode * buildOpTree(code_generator* codeGen, AST_Node* currentASTNode, OpTreeNode* parent, int level) {
	AST_Node * nextASTNode;
	OpTreeNode* thisOpNode = malloc(sizeof(OpTreeNode));
	AST_Node* left_side_AST_node;
	AST_Node* right_side_AST_node;
	dcg_Reg rightReg;
	dcg_Reg leftReg;

	thisOpNode->left                  = NULL;
	thisOpNode->right                 = NULL;
	thisOpNode->parent                = parent;
	thisOpNode->code                  = dcg_NOP;
	thisOpNode->ast_node              = NULL;
	thisOpNode->arg_struct            = malloc(sizeof(dcg_ArgStruct));
	thisOpNode->arg_struct->arg_types = 0;

	// Build OpNode subtree depending on the AST_Node type
	switch (currentASTNode->type) {
	case(AST_BLOCK):
		OpTreeNode* leftNode = NULL;

		// For each child of the block
		for (unsigned int i = 0; i < currentASTNode->children->size; i++) {

			// I think we can release all registers after each expression?
			// Basicaly make a linked list of OpNodes in order
			AST_Node* childNode = (AST_Node*)dd_get(currentASTNode->children, i);
			thisOpNode = buildOpTree(codeGen, childNode, thisOpNode, level + 1);
			freeAllRegs(codeGen->regAlloc);
			resetRegs(codeGen);

			// Need to traverse all the way down the left side to place node
			if (thisOpNode != NULL) {
				OpTreeNode* temp = thisOpNode;
				while (temp->left != NULL) {
					temp = temp->left;
				}
				temp->left  = leftNode;
				temp->right = NULL;  // Not sure if this is correct
				leftNode = thisOpNode;
			}
		} 
		break;

	case(AST_INT):
		thisOpNode->code = dcg_none;
		thisOpNode->ast_node = currentASTNode;
		/*
		*	Going to keep this commented out for now, don't think we need it
		//thisOpNode->resultReg = popReg(codeGen);
		thisOpNode->ast_node->asmOpNode = thisOpNode;
		thisOpNode->arg_struct->OpArg1.arg_reg = popReg(codeGen);
		thisOpNode->arg_struct->OpArg2.arg_int = thisOpNode->ast_node->int_literal;
		setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_REG, dcg_OpCodeArgType_LIT);
		*/
		break;

	case(AST_IDENT):
		// Check the symbol table to see if IDENT exists
		thisOpNode->code = dcg_MOV;
		//thisOpNode->resultReg = popReg(codeGen);
		thisOpNode->ast_node = currentASTNode;
		thisOpNode->ast_node->asmOpNode = thisOpNode;
		// this will point to a string in a AST_Node
		//thisOpNode->arg_struct->OpArg1.arg_reg = popReg(codeGen);
		thisOpNode->arg_struct->OpArg1.arg_reg = getReg(codeGen->regAlloc, dcg_none, thisOpNode);
		thisOpNode->arg_struct->OpArg2.arg_var = thisOpNode->ast_node->identName;
		setSymbolReg(codeGen->parser->table, thisOpNode->ast_node->identName, thisOpNode->ast_node->nameLength, thisOpNode->arg_struct->OpArg1.arg_reg);
		setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_REG, dcg_OpCodeArgType_STACKOFF);

		// TODO: eventually check if parent node ( or from the =, change the child), so this doesn't
		// produce any instructions if only being used in a = expression
		break;

	case(AST_BIOP):

		// Get the children
		// !! The left and right nodes should contain information about 
		// the location (literal, stack offset, reg) of their respective AST_Node !!
		thisOpNode->left     = buildOpTree(codeGen, dd_get(currentASTNode->children, 0), thisOpNode, level + 1);
		thisOpNode->right    = buildOpTree(codeGen, dd_get(currentASTNode->children, 1), thisOpNode, level + 1);

		// Set the other attributes
		thisOpNode->code = dcg_MOV;
		thisOpNode->ast_node = currentASTNode;
		thisOpNode->ast_node->asmOpNode = thisOpNode;

		/*
		*	If this binary operator is operating on two intergers, just calculate it and replace biop
		*	in AST with the new INT literal
		*
		*	TODO: This should be done in the parser, I think
		*/
		if (thisOpNode->left->ast_node->type == AST_INT && thisOpNode->right->ast_node->type == AST_INT) {

			thisOpNode->ast_node->type = AST_INT;
			// Don't need an opcode because nothing needs to happen
			// Assuming the parent (BIOP or something else) is going to deal with this AST_node directly
			thisOpNode->code = dcg_none;

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

			// Don't need the children nodes because we replaced them with this node
			freeOpNode(codeGen, &(thisOpNode->left));
			freeOpNode(codeGen, &(thisOpNode->right));

		}

		// else, create a OpNode for the BIOP
		else {
			
			/*
			*	These BIOPs look at there children in the AST to decide what to do
			*
			*	'=' Biop specificly looks for a register associated with its left child in the AST
			*	to store the varible into
			*
			*/

			// These all looked the same, so I used a goto
			// I know it's taboo, but it looks cleaner here
			// If I realize a better solution, I will use it, but for now, this seems ok
			switch (currentASTNode->token->type) {
			case(dl_PLUS):
				thisOpNode->code = dcg_ADD;
				goto commutative_arithmatic_biop;
			case(dl_MULT):
				thisOpNode->code = dcg_MUL;

				// All Biop statements so far lead here:
				commutative_arithmatic_biop:

				// Our left and right AST_Nodes can't both be literals because of the precomputation above
				// Check both our right and left children in the AST, find a variable with a register to use as first arg
				left_side_AST_node = dd_get(thisOpNode->ast_node->children, 0);
				right_side_AST_node = dd_get(thisOpNode->ast_node->children, 1);

				rightReg = putbackRegFromOpNode(codeGen->regAlloc, thisOpNode->right);
				leftReg  = putbackRegFromOpNode(codeGen->regAlloc, thisOpNode->left);

				// If the left node is the one we can store our value into
				if (leftReg != dcg_none) {

					// Set first arg
					// Use getReg because it will be our storage register
					dcg_Reg temp = getReg(codeGen->regAlloc, leftReg, thisOpNode);
					thisOpNode->arg_struct->OpArg1.arg_reg = temp;
					setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_REG, dcg_OpCodeArgType_NONE);

					// Set second arg
					switch (right_side_AST_node->type) {
					case(AST_INT):

						// Directly use the int_literal from the AST_INT node

						setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_NONE, dcg_OpCodeArgType_LIT);
						thisOpNode->arg_struct->OpArg2.arg_int = right_side_AST_node->int_literal; // TODO: Sketch, cleanup because this memory is allocated for the symbol in symtbl
						break;

					case(AST_IDENT):
					case(AST_BIOP):
					{

						// Find the storage register for the right AST_Node
						if (rightReg != dcg_none) {
							thisOpNode->arg_struct->OpArg2.arg_reg = rightReg;
							setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_NONE, dcg_OpCodeArgType_REG);
							pushUsedReg(codeGen, right_side_AST_node->asmOpNode);
						}
						else {
							printf("CodeGen: Error, I don't know what to do with this Op arg when generating code for '+/*'\n");
						}
					}
						break;

					default:
						printf("CodeGen: Error Dont know how to add these AST + Children\n");
						break;
					}
							
				}

				// If the right node is the one we can store our value into
				else if (rightReg != dcg_none){
				
					// Set first arg
					dcg_Reg temp = getReg(codeGen->regAlloc, rightReg, thisOpNode);
					thisOpNode->arg_struct->OpArg1.arg_reg = temp;
					setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_REG, dcg_OpCodeArgType_NONE);

					// Set second arg
					switch (left_side_AST_node->type) {

					// Don't check other AST types because they would've been caught in the earlier if checking for a REG to store into
					case(AST_INT):

						setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_NONE, dcg_OpCodeArgType_LIT);
						thisOpNode->arg_struct->OpArg2.arg_int = left_side_AST_node->int_literal; // TODO: Sketch, cleanup because this memory is allocated for the symbol in symtbl
						break;

					default:
						printf("CodeGen: Error Dont know how to add these AST + Children\n");
						break;
					}
				}
				break;

			case(dl_MINUS):

				thisOpNode->code = dcg_SUB;

				// Our left and right AST_Nodes can't both be literals because of the precomputation above
				// Check both our right and left children in the AST, find a variable with a register to use as first arg
				left_side_AST_node = dd_get(thisOpNode->ast_node->children, 0);
				right_side_AST_node = dd_get(thisOpNode->ast_node->children, 1);

				rightReg = peakRegFromOpNode(codeGen->regAlloc, thisOpNode->right);
				leftReg  = putbackRegFromOpNode(codeGen->regAlloc, thisOpNode->left);

				// If the left node is the one we can store our value into
				// Then we could have the right operand be a literal
				if (leftReg != dcg_none) {

					// Set first arg
					// Use getReg because it will be our storage register
					dcg_Reg temp = getReg(codeGen->regAlloc, leftReg, thisOpNode);
					thisOpNode->arg_struct->OpArg1.arg_reg = temp;
					setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_REG, dcg_OpCodeArgType_NONE);

					// Set second arg
					switch (right_side_AST_node->type) {
					case(AST_INT):

						// Directly use the int_literal from the AST_INT node

						setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_NONE, dcg_OpCodeArgType_LIT);
						thisOpNode->arg_struct->OpArg2.arg_int = right_side_AST_node->int_literal; // TODO: Sketch, cleanup because this memory is allocated for the symbol in symtbl
						break;

					case(AST_IDENT):
					case(AST_BIOP):
					{

						// Find the storage register for the right AST_Node
						if (rightReg != dcg_none) {
							thisOpNode->arg_struct->OpArg2.arg_reg = rightReg;
							setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_NONE, dcg_OpCodeArgType_REG);
							pushUsedReg(codeGen, right_side_AST_node->asmOpNode);
						}
						else {
							printf("CodeGen: Error, I don't know what to do with this Op arg when generating code for '+/*'\n");
						}
					}
						break;

					default:
						printf("CodeGen: Error Dont know how to add these AST + Children\n");
						break;
					}
							
					putbackRegFromOpNode(codeGen->regAlloc, thisOpNode->right);
				}

				// If the right node is the one we can store our value into
				// Then we need to store the left operand in a register 
				else if (rightReg != dcg_none){

					// Left child (left side of - ) goes in a register
					OpTreeNode* movOpNode = malloc(sizeof(OpTreeNode));
					movOpNode->left                       = thisOpNode->left;
					movOpNode->left->parent               = movOpNode;
					movOpNode->right                      = thisOpNode->right;
					movOpNode->right->parent              = movOpNode;
					movOpNode->parent                     = thisOpNode;
					movOpNode->code                       = dcg_MOV;
					movOpNode->ast_node                   = thisOpNode->ast_node;
					movOpNode->arg_struct                 = malloc(sizeof(dcg_ArgStruct));
					movOpNode->arg_struct->arg_types      = (dcg_OpCodeArgType_REG | (dcg_OpCodeArgType_LIT << 4));
					movOpNode->arg_struct->OpArg1.arg_reg = getReg(codeGen->regAlloc, dcg_none, thisOpNode);
					movOpNode->arg_struct->OpArg2.arg_int = movOpNode->left->ast_node->int_literal;

					thisOpNode->left = movOpNode;
					thisOpNode->right = NULL;
				
					setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_REG, dcg_OpCodeArgType_REG);

					// Set first arg
					thisOpNode->arg_struct->OpArg1.arg_reg = movOpNode->arg_struct->OpArg1.arg_reg; // TODO: Sketch, cleanup because this memory is allocated for the symbol in symtbl

					// Set second arg
					thisOpNode->arg_struct->OpArg2.arg_reg = rightReg;

					putbackRegFromOpNode(codeGen->regAlloc, movOpNode->right);
				}
				break;

			case(dl_DIVIDE):
				

				// Allocate OpNodes
				OpTreeNode* xorOpNode = malloc(sizeof(OpTreeNode));
				OpTreeNode* movOpNode = malloc(sizeof(OpTreeNode));

				// Get eax
				getReg(codeGen->regAlloc, dcg_eax, thisOpNode);

				// Get edx
				getReg(codeGen->regAlloc, dcg_edx, thisOpNode);

				// Should I put this on the right side? so that they all get done last?
				// Yes, you want these to happen right before the div op, nothing else should have a chance of running

				// Zero edx
				xorOpNode->left                       = thisOpNode->left;
				xorOpNode->left->parent               = xorOpNode;
				xorOpNode->right                      = thisOpNode->right;
				xorOpNode->right->parent              = xorOpNode;
				//xorOpNode->right                      = thisOpNode->right;
				xorOpNode->parent                     = movOpNode;
				xorOpNode->code                       = dcg_XOR;
				xorOpNode->ast_node                   = thisOpNode->ast_node;
				xorOpNode->arg_struct                 = malloc(sizeof(dcg_ArgStruct));
				xorOpNode->arg_struct->arg_types      = (dcg_OpCodeArgType_REG | (dcg_OpCodeArgType_REG << 4));
				xorOpNode->arg_struct->OpArg1.arg_reg = dcg_edx;
				xorOpNode->arg_struct->OpArg2.arg_reg = dcg_edx;

				// Left child (Dividend) goes in eax
				movOpNode->left                       = xorOpNode;
				movOpNode->right                      = NULL;
				movOpNode->parent                     = thisOpNode;
				movOpNode->code                       = dcg_MOV;
				movOpNode->ast_node                   = thisOpNode->ast_node;
				movOpNode->arg_struct                 = malloc(sizeof(dcg_ArgStruct));
				movOpNode->arg_struct->arg_types      = (dcg_OpCodeArgType_REG | (dcg_OpCodeArgType_REG << 4));
				movOpNode->arg_struct->OpArg1.arg_reg = dcg_eax;

				// Set source opperand
				leftReg  = putbackRegFromOpNode(codeGen->regAlloc, xorOpNode->left);

				if (leftReg != dcg_none) {
					movOpNode->arg_struct->OpArg2.arg_reg = leftReg;
				}
				// Assume left OpNode is an int litteral
				else {
					movOpNode->arg_struct->OpArg2.arg_int = thisOpNode->left->ast_node->int_literal;
				}

				thisOpNode->code = dcg_DIV;
				thisOpNode->left = movOpNode;
				thisOpNode->right = NULL;

				// This OpNodes only argument is it's right child
				// !! THIS IS WRONG. ITS TAKING INFORMATION ABOUT THE RIGHTSIDE
				// AND PUTTING IN THE LEFT AS IF IT'S ALREADY HAPPENED
				// This is why I should consider putting the xor and mov on the right side as parents
				// Of whatever was already there
				rightReg  = putbackRegFromOpNode(codeGen->regAlloc, xorOpNode->right);

				if (rightReg != dcg_none) {
				    thisOpNode->arg_struct->arg_types = dcg_OpCodeArgType_REG;
					thisOpNode->arg_struct->OpArg1.arg_reg = rightReg;
				}
				else {
					// Assume right OpNode is an int litteral

					// Need to move it into a register
					OpTreeNode* litMovOpNode = malloc(sizeof(OpTreeNode));
					litMovOpNode->left                       = movOpNode;
					litMovOpNode->left->parent               = litMovOpNode;
					litMovOpNode->right                      = NULL;
					litMovOpNode->parent                     = thisOpNode;
					litMovOpNode->code                       = dcg_MOV;
					litMovOpNode->ast_node                   = thisOpNode->ast_node;
					litMovOpNode->arg_struct                 = malloc(sizeof(dcg_ArgStruct));
					litMovOpNode->arg_struct->arg_types      = (dcg_OpCodeArgType_REG | (dcg_OpCodeArgType_LIT << 4));
					litMovOpNode->arg_struct->OpArg1.arg_reg = getReg(codeGen->regAlloc, dcg_none, thisOpNode);
					litMovOpNode->arg_struct->OpArg2.arg_int = xorOpNode->right->ast_node->int_literal;

					thisOpNode->left = litMovOpNode;
				    thisOpNode->arg_struct->arg_types = dcg_OpCodeArgType_REG;
					// Can do this cause we just set this register and know for sure it's what we want
					thisOpNode->arg_struct->OpArg1.arg_int = litMovOpNode->arg_struct->OpArg1.arg_reg;

					// Can immedietly free this bc we don't need it anymore
					putbackReg(codeGen->regAlloc, litMovOpNode->arg_struct->OpArg1.arg_reg);
				}

				// Only needed it to make sure the previous value didnt get erased, didn't actually store anything (we need rn) in it
				putbackReg(codeGen->regAlloc, dcg_edx);

				// Quotient gets stored in eax ( But this conflicts with how parents grab arguments from their children )
				// Maybe storage register should be determined by searching the register allocator?!!!

				break;

			case(dl_EQUALS):

				// First ARG of the mov op
				// Assume left side of = is a symbol in the symbol table
				left_side_AST_node = dd_get(thisOpNode->ast_node->children, 0);
				d_symbol* leftSymbol = getSymbol(codeGen->parser->table, left_side_AST_node->identName, left_side_AST_node->nameLength);
				setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_STACKOFF, dcg_OpCodeArgType_NONE);
				thisOpNode->arg_struct->OpArg1.arg_var = leftSymbol->symbol_name; // TODO: Sketch, cleanup because this memory is allocated for the symbol in symtbl
				
				// Second ARG of the mov op
				right_side_AST_node = dd_get(thisOpNode->ast_node->children, 1);
				switch (right_side_AST_node->type) {
				case(AST_INT):

					// Directly use the int_literal from the AST_INT node
					setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_NONE, dcg_OpCodeArgType_LIT);
					thisOpNode->arg_struct->OpArg2.arg_int = right_side_AST_node->int_literal; // TODO: Sketch, cleanup because this memory is allocated for the symbol in symtbl
					break;

				case(AST_IDENT):
				case(AST_BIOP):
					{
						// Get the register that the final value of the right AST node was stored in
						dcg_Reg rightReg = putbackRegFromOpNode(codeGen->regAlloc, thisOpNode->right);

						if (rightReg != dcg_none) {
							thisOpNode->arg_struct->OpArg2.arg_reg = rightReg;
							setArgTypes(&(thisOpNode->arg_struct->arg_types), dcg_OpCodeArgType_NONE, dcg_OpCodeArgType_REG);
						}
						else {
							printf("CodeGen: Error, I don't know what to do with this Op arg when generating code for '='\n");
						}
					}

				break;
				}

			break;

			// Dont recognize this AST BIOP
			default:
				fprintf(stderr, "CodeGen: Illegal BIOP Lexeme");
				exit(1);
				return NULL;
			}
		} 
		
		break;
	}
	return thisOpNode;
}

/*
*	Pass in a Register to get printed out to the file
*/
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

/*
*	Pass in a x64 asm OP to get printed out to the file
*/
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
		d_appendString(codeGen->fileState, "imul   ");
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

void appendLineEndIdent(code_generator* codeGen, dcg_OpCode opCode, dcg_Reg Reg1, const char* arg2) {
	appendOp(codeGen, opCode);
	if (Reg1 != dcg_none) {
		appendReg(codeGen, Reg1);
	}
	else {
		fprintf(stderr, "Reg1 cant be none in appendLineEndInt\n");
		exit(1);
		return;
	}
	if (arg2 != NULL) {
		d_appendString(codeGen->fileState, ", ");
		d_appendString(codeGen->fileState, "[rbx-");
		d_appendString(codeGen->fileState, arg2);
		d_appendString(codeGen->fileState, "]");
		d_appendString(codeGen->fileState, "\n");
	}
	else {
		fprintf(stderr, "art2 cant be NULL in appendLineEndIdent\n");
		exit(1);
		return;
	}
}

void appendStackOffset(code_generator* codeGen, dcg_OpCode opCode, dcg_Reg Reg1, const char* arg2) {
	appendOp(codeGen, opCode);
	if (Reg1 != dcg_none) {
		appendReg(codeGen, Reg1);
	}
	else {
		fprintf(stderr, "Reg1 cant be none in appendLineEndInt\n");
		exit(1);
		return;
	}
	if (arg2 != NULL) {
		d_appendString(codeGen->fileState, ", ");
		d_appendString(codeGen->fileState, "DWORD PTR ");   // TODO: THIS IS A TYPE THING, WILL CHANGE DEPENDING ON VAR SIZE
		d_appendString(codeGen->fileState, arg2);
		d_appendString(codeGen->fileState, "[rbp]");
		d_appendString(codeGen->fileState, "\n");
	}
	else {
		fprintf(stderr, "art2 cant be NULL in appendLineEndIdent\n");
		exit(1);
		return;
	}
}


/*
*	Take an opnode and print it out to the file
*	Prints the x64 ASM Operator and its arguments if any
*/
void appendOpLine(code_generator* codeGen, OpTreeNode* node) {
	appendOp(codeGen, node->code);

	// First arg
	switch ((node->arg_struct->arg_types) & 0x0F) {
	case(dcg_OpCodeArgType_REG):
		if (node->arg_struct->OpArg1.arg_reg != dcg_none) {
			appendReg(codeGen, node->arg_struct->OpArg1.arg_reg);
		}
		break;
	case(dcg_OpCodeArgType_STACKOFF):
		if (node->arg_struct->OpArg1.arg_var != NULL) {
			//d_appendString(codeGen->fileState, ", ");
			d_appendString(codeGen->fileState, "DWORD PTR ");   // TODO: THIS IS A TYPE THING, WILL CHANGE DEPENDING ON VAR SIZE
			d_appendString(codeGen->fileState, node->arg_struct->OpArg1.arg_var);
			d_appendString(codeGen->fileState, "[rbp]");
		}
		break;
	default:
		printf("Code Gen: Error, Dont know how to handle this first arg for an OP");
		break;
	}

	// Second arg
	switch (((node->arg_struct->arg_types) & 0xF0) >> 4) {
	case(dcg_OpCodeArgType_NONE):
		break;
	case(dcg_OpCodeArgType_LIT):
		d_appendString(codeGen->fileState, ", ");
		d_appendInt(codeGen->fileState, node->arg_struct->OpArg2.arg_int);	// TODO: THIS IS A TYPE THING, WILL CHANGE DEPENDING ON TYPE
		break;
	case(dcg_OpCodeArgType_REG):
		if (node->arg_struct->OpArg2.arg_reg != dcg_none) {
			d_appendString(codeGen->fileState, ", ");
			appendReg(codeGen, node->arg_struct->OpArg2.arg_reg);
		}
		break;
	case(dcg_OpCodeArgType_STACKOFF):
		if (node->arg_struct->OpArg2.arg_var != NULL) {
			d_appendString(codeGen->fileState, ", ");
			d_appendString(codeGen->fileState, "DWORD PTR ");   // TODO: THIS IS A TYPE THING, WILL CHANGE DEPENDING ON VAR SIZE
			d_appendString(codeGen->fileState, node->arg_struct->OpArg2.arg_var);
			d_appendString(codeGen->fileState, "[rbp]");
		}
		break;
	default:
		printf("Code Gen: Error, Dont know how to handle this second arg for an OP");
		break;
	}

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
	d_appendInt(codeGen->fileState, codeGen->parser->table->stack_offset);
	d_appendString(codeGen->fileState, "\n");
}

void printEndOfScope(code_generator* codeGen) {

	const char* eos  = "add  rsp, ";

	d_appendString(codeGen->fileState, eos);
	d_appendInt(codeGen->fileState, codeGen->parser->table->stack_offset);
	d_appendString(codeGen->fileState, "\n");
}

void printFooter(code_generator* codeGen) {

	const char * footer = //"mov   rcx, rax    ; Stores the result\n"
					      "call ExitProcess\n"
		                  "Start ENDP\n"
		                  "END\n";

	d_appendString(codeGen->fileState, footer);
}

/*
*	Given an OpTree node, this function recursivley writes its children, then the root node to a file
*/
void writeOpTree(code_generator * codeGen, OpTreeNode * node) {

	// Recursively write children
	if(node->left != NULL)
		writeOpTree(codeGen, node->left);
	if(node->right != NULL)
		writeOpTree(codeGen, node->right);

	// Depending on the OpCode of the OpTree Node, do specific things
	switch (node->code) {

	case(dcg_DIV):

		// TODO: Division is currently broken, need to fix
		//appendLineEndReg(codeGen, dcg_XOR, dcg_edx, dcg_edx); // TODO: this really needs to be moved to buildOpTree
		appendOpLine(codeGen, node);
		break;

	case(dcg_XOR):
	case(dcg_MOV):
	case(dcg_ADD):
	case(dcg_SUB):
	case(dcg_MUL):

		// Just append the node
		appendOpLine(codeGen, node);
		break;
	}
}

void print_local_var_offset_definition(code_generator* codeGen, const char * var_name, unsigned int stack_offset) {
	d_appendString(codeGen->fileState, var_name);
	d_appendString(codeGen->fileState, " = ");
	// This number will determine the amount of variables you can have
	/*
	char* num = calloc(10, sizeof(char));
	// ceil( log10 (..))) calcs length of a number
	// + 1 on end accounts for \0 char
	sprintf_s(num, (size_t)ceil(log10((double)var_size + 1)) + 1, "%u", codeGen->stack_offset);
	d_appendString(codeGen->fileState, num);
	*/
	d_appendString(codeGen->fileState, "-");
	d_appendInt(codeGen->fileState, stack_offset);
	d_appendString(codeGen->fileState, "\n");
}

void DefineLocalVars(code_generator* codeGen) {
	for (unsigned int i = 0; i < codeGen->parser->table->d_array->size; i++) {
		d_symbol* symbol = dd_get(codeGen->parser->table->d_array, i);
		if (symbol->type == dtype_INT) {
			print_local_var_offset_definition(codeGen, symbol->symbol_name, symbol->stack_offset);
		}
	}
}

void GenCodeFromAST(code_generator* codeGen, AST_Node * currentNode) {

	// Build an op tree from the AST
	codeGen->OpTree_Root = buildOpTree(codeGen, currentNode, NULL, 0);

	// Write op tree to output file
	writeOpTree(codeGen, codeGen->OpTree_Root);

	// mov the value in the last used register into the return code register
	// THIS ISN'T CORRECT, JUST FOR NOW / TILL I MAKE A RETURN KEYWORD !!!!!!!!!!!!!

	// First arg
	switch ((codeGen->OpTree_Root->arg_struct->arg_types) & 0x0F) {
	case(dcg_OpCodeArgType_REG):
		appendLineEndReg(codeGen, dcg_MOV, dcg_ecx, codeGen->OpTree_Root->arg_struct->OpArg1.arg_reg);
		break;
	case(dcg_OpCodeArgType_STACKOFF):
		appendStackOffset(codeGen, dcg_MOV, dcg_ecx, codeGen->OpTree_Root->arg_struct->OpArg1.arg_var);
		break;
	default:
		printf("Code Gen: Error, Dont know how to finish this function");
		break;
	}
}

void GenerateCode(code_generator* codeGen) {
	/*
	extrn ExitProcess: PROC   ; external functions in system libraries
	.code
	*/
	printHeader(codeGen);

	/*
	Start PROC
	*/
	printStartProc(codeGen);

	/*
	i = -4
	*/
	DefineLocalVars(codeGen);

	/*
	push  rbp
	mov   rbp, rsp
	sub   rsp, 4
	*/
	printBeginningOfScope(codeGen);

	/*
	   Pass in root of AST
	   recursivly build OpTree from AST
	   recursivly write OpTree
	*/
	GenCodeFromAST(codeGen, codeGen->parser->ast_root);

	/*
	add  rsp, 16
	*/
	printEndOfScope(codeGen);

	/*
	call ExitProcess
	Start ENDP
	END
	*/
	printFooter(codeGen);
}
