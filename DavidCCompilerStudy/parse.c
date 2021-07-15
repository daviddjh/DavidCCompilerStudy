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

Node *parseExpresion(Parser* parser, int pres) {


  // Setup Nodes
	Node* root = NULL;
	Node* left = NULL;

  // Peak the next token
  parser->currentToken = peakNextToken(parser->lexer);
  //root = NULL;

  // Loop until ;
  while(parser->currentToken->type != SEMI) {

    // if the currentToken type is END OF FILE, then return root=NULL
    if (parser->currentToken->type == EOFT) {
      fprintf(stderr, "Parse error: Expecting ';' token before EOF, line: %d\n", left->token->lineno);
      exit(1);
    }

    switch (parser->currentToken->type) {

      // If Biop:
      case(MULT):
      case(DIVIDE):
        // root = parseBiop
        // Set root->left to left (in args)
        // Set root->right to parseExpresion
        // root = parseBiop
        // Set root->left to left
        // Set root->right to parseExpresion
        {
          if(pres > 100) { return left; } else {
            root = parseBiop(parser, left, 100);
          }
          break;
        }
      case(PLUS):
      case(MINUS):
        {
          if(pres > 50) { return left; } else {
            root = parseBiop(parser, left, 50);
          }
          break;
        }
        
      // If Int:
      case(INT_LITERAL):
        root = parseInt(parser);
        break;

      // Unknown
      default:
        root = malloc(sizeof(Node));
        root->type = AST_UNKNOWN;
        root->precidence = -9999;
        root->token = parser->currentToken;
    }
    
    // Set left to root so next node can use it
    left = root;

    // Peak the next token
    parser->currentToken = peakNextToken(parser->lexer);
    //root = NULL;
  }

  return root;

}

Node * parseBiop(Parser* parser, Node* left, int pres) {
	if (left == NULL) {
		fprintf(stderr, "Error, parseBiop got a NULL oldRoot\n");
		exit(1);
	}
	// check the old root
  // TODO this is type checking, idk if we should do this yet
  /*
	if (left ->type != AST_INT) {
		fprintf(stderr, "Parse error, operand to a BIOP not an int, line: %d\n", left->token->lineno);
		exit(1);
	}
  */

	// This node is the bianary operator
	Node* newRoot = malloc(sizeof(Node));

	// Set up this node
	newRoot->children = NULL;
	newRoot->type = AST_BIOP;
	newRoot->token = eatNextToken(parser->lexer);
	newRoot->precidence = 0;

	// Add the previous token (oldRoot) to the children array of this node
	newRoot->children = dd_makeDynamicArray();
	dd_push(newRoot->children, left);

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
  rightChild = parseExpresion(parser, pres);
  dd_push(newRoot->children, rightChild);

  // Rotate tree if the operand is missaligned
  /*
  if (rightChild->precidence < newRoot->precidence) {
    newRoot = leftRotateTree(newRoot, rightChild);
  }
  */

  // Return the new root of this subtree and its precidence
  return newRoot;
}

Node * parseInt(Parser* parser) {
	// Make root node
	Node * root = malloc(sizeof(Node));
	root->children = NULL;
	root->token = eatNextToken(parser->lexer);
	root->type = AST_INT;

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
