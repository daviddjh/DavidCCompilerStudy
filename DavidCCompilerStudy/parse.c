#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "parse.h"
#include "lex.h"
#include "dcommon.h"
#include "symbol.h"

void InitParser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->ast_root = NULL;
    initSymbolTable(&(parser->table));
}

// isGlobalBlock
AST_Node* parseBlock(Parser * parser, BOOL isGlobalBlock) {

    // Peak the next token
    parser->currentToken = peakNextToken(parser->lexer);

    // Setup Root block Node
    if (parser->currentToken->type != dl_LS && isGlobalBlock == FALSE) {
        fprintf(stderr, "Error: Block should start with the beginning of the file, or a \'{\'");
        exit(1);
        return NULL;
    }

    // Set up root block node
    AST_Node * root = malloc(sizeof(AST_Node));
    root->children = dd_makeDynamicArray();
    // We can do this because if global block: i want the first token, else: I want the { token
    root->token = parser->currentToken;
    root->type = AST_BLOCK;

    // Loop through statements until next '{'
    while (parser->currentToken->type != dl_RS && parser->currentToken->type != dl_EOFT) {
        AST_Node* newChild = parseStatement(parser);
        dd_push(root->children, newChild);

        // Peak the next token
        parser->currentToken = peakNextToken(parser->lexer);

        // This should be {}, if.., while.. for.. , anything that doesn't end in a ';'
        if (newChild->type != AST_BLOCK) {
            if (parser->currentToken->type == dl_SEMI) {
                // Move on to next token
                eatNextToken(parser->lexer);
                parser->currentToken = peakNextToken(parser->lexer);
            }
            else {
                fprintf(stderr, "Error: This statement didnt end where I think it was, or with a semicolor, line: %d\n", parser->currentToken->lineno);
            }
        }

    }

    return root;

    // Need to include logic somewhare to denode clothed parenthases
    // if currentToken == EOFT && isGlobalBlock == TRUE, Error?, Counter in the parser state?

}

AST_Node* parseStatement(Parser* parser) {
    parser->currentToken = peakNextToken(parser->lexer);
    switch (parser->currentToken->type)
    {
    // If Biop:
    case(dl_EQUALS):
    case(dl_MULT):
    case(dl_DIVIDE):
    case(dl_PLUS):
    case(dl_MINUS):

    // If Identity
    case(dl_IDENT):

    // If Int:
    case(dl_INT_LITERAL):
        return parseExpression(parser, 0, NULL);
        break;

    // If Ident Declare
    // Just int for now
    case(dl_INT):
        return parseIdentDeclare(parser);


    default:
        break;
    }
}

AST_Node* parseIdentity(Parser* parser) {
    AST_Node* root = NULL;
    parser->currentToken = peakNextToken(parser->lexer);
    if (parser->currentToken->type == dl_IDENT) {
        root = malloc(sizeof(AST_Node));
        root->children = NULL;
        root->token = parser->currentToken;
        root->type = AST_IDENT;
        root->nameLength = parser->currentToken->lexeme_length;
        root->identName = malloc(sizeof(char) * root->nameLength);
		// + 1 for \0
        memcpy(root->identName, parser->currentToken->lexeme, root->nameLength + 1);
        eatNextToken(parser->lexer);
        parser->currentToken = peakNextToken(parser->lexer);
        if (parser->currentToken->type == dl_EQUALS) {
            root = parseExpression(parser, -9999, root);
        }
    }
    else {
        root = NULL;
    }
    
    return root;
}

AST_Node* parseIdentDeclare(Parser* parser) {
    AST_Node* root = NULL;
    symbol_type identType = dtype_INV;
    parser->currentToken = peakNextToken(parser->lexer);
    switch (parser->currentToken->type) {
    case(dl_INT):
        eatNextToken(parser->lexer);
        identType = dtype_INT;

    // Other type cases go here without a break

        // This block of code is the same as parseIdentity, but with the addSymbol function for delaration
		parser->currentToken = peakNextToken(parser->lexer);
		if (parser->currentToken->type == dl_IDENT) {
			root = malloc(sizeof(AST_Node));
			root->children = NULL;
			root->token = parser->currentToken;
			root->type = AST_IDENT;
			root->nameLength = parser->currentToken->lexeme_length;
			root->identName = calloc(root->nameLength, sizeof(char));
            // + 1 for \0
			memcpy(root->identName, parser->currentToken->lexeme, root->nameLength + 1);
            if (addSymbol(parser->table, dtype_INT, root->identName, root->nameLength) == FALSE) {
                fprintf(stderr, "The identity being declared on line %u has already been declared\n", parser->currentToken->lineno);
                exit(1);
            }
			eatNextToken(parser->lexer);
			parser->currentToken = peakNextToken(parser->lexer);
			if (parser->currentToken->type == dl_EQUALS) {
				root = parseExpression(parser, -9999, root);
			}
		}
		else {
            fprintf(stderr, "A non identity follows a typename. I cant handle this.");
            exit(1);
            return root;
		}
        break;

    default:
        fprintf(stderr, "Cant find a type in parseIdentDeclare at line %u", parser->currentToken->lineno);
        return NULL;
        break;
    }
    return root;
}

AST_Node *parseExpression(Parser* parser, int pres, AST_Node* left) {

  // Setup Nodes
    AST_Node* root = NULL;   // The node to be created in this function
    //AST_Node* left = NULL;   // The left node to be saved, right node is made recursively

  // Peak the next token
  parser->currentToken = peakNextToken(parser->lexer);

  // Loop until ; (in the future change this to more lexemes)
  while(parser->currentToken->type != dl_SEMI) {

    // if the currentToken type is END OF FILE, then return root=NULL
    if (parser->currentToken->type == dl_EOFT) {
      fprintf(stderr, "Parse error: Expecting ';' token before EOF, line: %d\n", left->token->lineno);
      exit(1);
    }

    // Different paths to take based on the lexeme in the expression
    // Could either be atom ( literal, variable ) or operator ( +, /, = )
    switch (parser->currentToken->type) {

      // If Biop:
      case(dl_MULT):
      case(dl_DIVIDE):
        // root = parseBiop
        // Set root->left to left (in args)
        // Set root->right to parseExpresion
        // root = parseBiop
        // Set root->left to left
        // Set root->right to parseExpresion
        {
          if(pres >= 100) { return left; } else {
            root = parseBiop(parser, left, 100);
            if (root->children != NULL) {
                dd_push(root->children, parseExpression(parser, 100, NULL));
            }
          }
          break;
        }
      case(dl_PLUS):
      case(dl_MINUS):
        {
          if(pres >= 50) { return left; } else {
            root = parseBiop(parser, left, 50);
            if (root->children != NULL) {
                dd_push(root->children, parseExpression(parser, 50, NULL));
            }
          }
          break;
        }
      case(dl_EQUALS):
        {
          if(pres >= 10) { return left; } else {
            root = parseBiop(parser, left, 10);
            if (root->children != NULL) {
                dd_push(root->children, parseExpression(parser, 10, NULL));
            }
          }
          break;
        }
        
      // If Int:
      case(dl_INT_LITERAL):
        root = parseInt(parser);
        // cant do this cause int might be the first number
        //return root; // This needs to return so the loop doesn't keep going and going down the right side
        break;

      case(dl_IDENT):
          root = parseIdentity(parser);
          break;

      // Unknown
      default:
        root = malloc(sizeof(AST_Node));
        root->type = AST_UNKNOWN;
        root->token = eatNextToken(parser->currentToken);
    }
    
    // Set left to root so next node can use it
    left = root;

    // Peak the next token
    parser->currentToken = peakNextToken(parser->lexer);
    //root = NULL;
  }

  return root;

}

// TODO: I think all this is wrong, check 5 - 4 * 3 + 2 *2;
AST_Node * parseBiop(Parser* parser, AST_Node* left, int pres) {
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
    AST_Node* newRoot = malloc(sizeof(AST_Node));

    // Set up this node
    newRoot->children = NULL;
    newRoot->type = AST_BIOP;
    newRoot->token = eatNextToken(parser->lexer);

    // Add the previous token (oldRoot) to the children array of this node
    newRoot->children = dd_makeDynamicArray();
    dd_push(newRoot->children, left);

    // Add the next expresion or operand to the tree
    /* I think this is the wrong place for this
    AST_Node* rightChild = NULL;
    rightChild = parseExpresion(parser, pres);
    dd_push(newRoot->children, rightChild);
    */

    // Second though, might be too much, so maybe just parseInt
    // Ok this doesn't work either because operators need to be able to have operators as children
    /*
    AST_Node* rightChild = NULL;
    // Replace this with parse atom
    rightChild = parseInt(parser);
    dd_push(newRoot->children, rightChild);
    */

    // Rotate tree if the operand is missaligned
    /*
    if (rightChild->precidence < newRoot->precidence) {
    newRoot = leftRotateTree(newRoot, rightChild);
    }
    */

  // Return the new root of this subtree and its precidence
  return newRoot;
}

AST_Node * parseInt(Parser* parser) {
    // Make root AST_Node
    AST_Node * root = malloc(sizeof(AST_Node));
    root->children = dd_makeDynamicArray();
    root->token = eatNextToken(parser->lexer);
    root->type = AST_INT;
    root->int_literal = atoi(root->token->lexeme);

    return root;
}

AST_Node* leftRotateTree(AST_Node* root, AST_Node* pivot) {
    AST_Node* temp;
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
