#include <stdlib.h>
#include <stdio.h>
#include "dinput.h"
#include "lex.h"

int main()
{
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
}
