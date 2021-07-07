#include <stdlib.h>
#include <stdio.h>
#include "dinput.h"
#include "lex.h"
#include "parse.h"

int main()
{
    /*
    ii_loadFile("Example.txt");
    Lexer lexer;
    InitLexer(&lexer);
    Token m_Token;
    do {
        m_Token = getNextToken(&lexer);
        printf("TOKEN: lexeme: \"%s\"\n    T Type: %d\n    Line Number: %d\n",m_Token.lexeme, m_Token.type, m_Token.lineno);
    } while (m_Token.type != EOFT && m_Token.type != LEX_ERROR); // while the end of file hasn't been reached, print out tokens
    printf("\n");
    if (m_Token.type == LEX_ERROR) {
        printf("Lex Error with: \"%s\" at line: %d\n", m_Token.lexeme, m_Token.lineno);
    }
    system("pause");
    return 0;
    */
    /*
    Lexer lexer;
    InitLexer(&lexer);
    dl_lex(&lexer, "Example1.txt");
    Token *token = malloc(sizeof(Token));
    token = getNextToken(&lexer);
    while (token->type != EOFT) {
        printf("Lexem: %s\n", token->lexeme);
        token = getNextToken(&lexer);
    }
    */
    Lexer lexer;
    InitLexer(&lexer);
    dl_lex(&lexer, "Example.txt");
    Parser parser;
    InitParser(&parser, &lexer);
    Node* ast_root;
    ast_root = parseExpresion(&parser);
    if (ast_root != NULL) {
		if(ast_root->token != NULL)
			printf("Node token Lexeme: %s\n", ast_root->token->lexeme);
		else 
			printf("Node token Lexeme: NULL\n");
		printf("    Node token Type: %d\n", ast_root->type);
    }
    else {
        printf("AST Root is NULL\n");
    }
    Node* child = dd_get(ast_root->children, 0);
    printf("Left Node: %s\n", child->token->lexeme);
    child = dd_get(ast_root->children, 1);
    printf("Right Node: %s\n", child->token->lexeme);

	printf("\n");
}
