#pragma once

#include <stdint.h>

#include "lex.h"

typedef enum AST_Type {
	AST_PROG,
	AST_INT,
	AST_BIOP,
	AST_RETURN,
	AST_UNKNOWN
} AST_Type;

typedef struct Node {
	Token* token;
	AST_Type type;
	DynamicArray* children;
	uint16_t precidence;
} Node;

typedef struct Parser {
	Lexer* lexer;
	Node* ast_root;
	Token* currentToken;
} Parser;

typedef struct AST {
	Node* root;
} AST;

void InitParser(Parser* parser, Lexer* lexer);
Node* parseExpresions();
Node * parseExpresion(Parser *parser);
Node * parseInt(Parser* parser);
Node * parseBiop(Parser* parser, Node* oldRoot);
Node* leftRotateTree(Node* root, Node* pivot);
