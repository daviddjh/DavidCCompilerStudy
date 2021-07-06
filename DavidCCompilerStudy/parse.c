#include <stdlib.h>
#include <stdint.h>
#include "parse.h"
#include "lex.h"

void InitParser(Parser* parser, Lexer* lexer) {
	parser->lexer = lexer;
	parser->ast_root = NULL;
}

Node* parseExpresions() {
	
}

Node* parseExpresion(Parser* parser, uint32_t pres) {
	// Get token
	Token* token;
	token = getNextToken(parser->lexer);

	if (token->type == EOFT) {
		return NULL;
	}

	// Make root node
	Node* root = malloc(sizeof(Node));
	root->children = NULL;
	root->token = token;

	switch (token->type) {
	case(INT):
		parseInt(parser, root);
		Token* token = peakNextToken(parser->lexer);
		if (token->type >= PLUS || token->type <= DIVIDE) {
			parseBiop(parser, root);
		}
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
		root->type = AST_UNKNOWN;
	}
	return root;
}

Node* parseBiop(Parser* parser, Node* oldRoot) {
	if (ast_int->type != AST_INT) {
		printf("Parse error, operand to a BIOP not an int, line: %d\n", ast_int->token->lineno);
		exit(1);
	}
	Node* newRoot = malloc(sizeof(Node));
	newRoot->children = NULL;
	newRoot->token = getNextToken(parser->lexer);
	switch (newRoot->token->type) {
	case(MULT):
	case(DIVIDE):
		newRoot->type = AST_BIOP;
		dd_push(newRoot->children, oldRoot);
		
		break;
	case(PLUS):
	case(MINUS):
		root->type = AST_BIOP;
		break;
	default:
		root->type = AST_UNKNOWN;
	}
}

parseInt(Parser* parser, Node* ast_int) {
	ast_int->type = AST_INT;
}

