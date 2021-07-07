#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "parse.h"
#include "lex.h"

void InitParser(Parser* parser, Lexer* lexer) {
	parser->lexer = lexer;
	parser->ast_root = NULL;
}

Node* parseExpresions() {
	
}

Node *parseExpresion(Parser* parser) {
	// Get token
	parser->currentToken = getNextToken(parser->lexer);

	if (parser->currentToken->type == EOFT) {
		return -9999;
	}
	
	Node* root = NULL;

	switch (parser->currentToken->type) {
	case(INT_LITERAL):
		root = parseInt(parser);
		Token* token = peakNextToken(parser->lexer);
		if (token->type >= PLUS && token->type <= DIVIDE) {
			root = parseBiop(parser, root);
		}
		return root;
		break;
	case(MULT):
	case(DIVIDE):
		root->type = AST_BIOP;
		break;
	case(PLUS):
	case(MINUS):
		root->type = AST_BIOP;
		break;
	case(RETURN):
		root->type = AST_RETURN;
		break;
	default:
		root = malloc(sizeof(Node));
		root->type = AST_UNKNOWN;
		root->precidence = -9999;
		root->token = parser->currentToken;
	}
	return root;
}

Node * parseBiop(Parser* parser, Node* oldRoot) {
	if (oldRoot == NULL) {
		fprintf(stderr, "Error, parseBiop got a NULL oldRoot\n");
		exit(1);
	}
	// check the old root
	if (oldRoot->type != AST_INT) {
		fprintf(stderr, "Parse error, operand to a BIOP not an int, line: %d\n", oldRoot->token->lineno);
		exit(1);
	}

	// This node is the bianary operator
	Node* newRoot = malloc(sizeof(Node));

	// Set up this node
	newRoot->children = NULL;
	newRoot->type = AST_BIOP;
	newRoot->token = getNextToken(parser->lexer);
	newRoot->precidence = 0;

	// Add the previous token (oldRoot) to the children array of this node
	newRoot->children = dd_makeDynamicArray();
	dd_push(newRoot->children, oldRoot);

  // Set the precidence
	switch (newRoot->token->type) {
	case(MULT):
		newRoot->precidence = 400;
    break;
	case(DIVIDE):
		newRoot->precidence = 200;
		break;
	case(PLUS):
		newRoot->precidence = 100;
		break;
	case(MINUS):
		newRoot->precidence = 50;
		break;
	default:
		newRoot->type = AST_UNKNOWN;
		newRoot->precidence = -9999;
    break;
	}

  // Add the next expresion or operand to the tree
  Node* rightChild = NULL;
  rightChild = parseExpresion(parser);
  dd_push(newRoot->children, rightChild);

  // Rotate tree if the operand is missaligned
  if (rightChild->precidence < newRoot->precidence) {
    newRoot = leftRotateTree(newRoot, rightChild);
  }

  // Return the new root of this subtree and its precidence
  return newRoot;
}

Node * parseInt(Parser* parser) {
	// Make root node
	Node * root = malloc(sizeof(Node));
	root->children = NULL;
	root->token = parser->currentToken;
	root->type = AST_INT;
	root->precidence = 9999;

	return root;
}

Node* leftRotateTree(Node* root, Node* pivot) {
	Node* temp;
	if (pivot->children != NULL){
		temp = dd_get(pivot->children, 0);
		dd_set(pivot->children, 0, root);
	}
	else {
		temp = NULL;
		pivot->children = dd_makeDynamicArray();
		dd_set(pivot->children, 0, root);
	}
	dd_set(root->children, 1, temp);
	return pivot;
}
