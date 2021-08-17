#pragma once

#include <stdint.h>

#include "lex.h"
#include "dcommon.h"
#include "symbol.h"

typedef enum AST_Type {
	AST_PROG, // Dont think i need this
	AST_BLOCK,
	AST_DECL,
	AST_IDENT,
	AST_INT,
	AST_BIOP,
	AST_RETURN,
	AST_UNKNOWN
} AST_Type;

typedef struct AST_Node {
	Token* token;
	AST_Type type;
	DynamicArray* children;
	int int_literal;
	// change this to identNameLength
	int nameLength;
	char* identName;
} AST_Node;

typedef struct Parser {
	Lexer* lexer;
	AST_Node* ast_root;
	Token* currentToken;
	symbol_table* table;
} Parser;

typedef struct AST {
	AST_Node* root;
} AST;

void  InitParser(Parser* parser, Lexer* lexer);
AST_Node* parseBlock(Parser* parser, BOOL isGlobalBlock);
AST_Node* parseStatement(Parser *parser);
AST_Node* parseExpression(Parser *parser, int);
AST_Node* parseIdentDeclare(parser);
AST_Node* parseInt(Parser* parser);
AST_Node* parseBiop(Parser* parser, AST_Node* oldRoot, int);

AST_Node* leftRotateTree(AST_Node* root, AST_Node* pivot);
AST_Node* rightRotateTree(AST_Node* root, AST_Node* pivot);
