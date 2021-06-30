#include <stdint.h>
#include "dinput.h"

typedef enum TOKEN_TYPE {
	LP, // '('
	RP, // ')'
	LS, // '{'
	RS, // '}'
	IF, // 'if'
	SEMI, // ';'
	MT, // >
	MTOE, // >=
	LT, // <
	LTOE, // <=
	EQIV, // ==
	ASSIGN, // =
	AND, // &
	OR, // |
	INT, // int
	BOOL, // bool
	IDENTIFIER, // function name or variable
	RETURN // return
} TOKEN_TYPE;

typedef struct Token {

	TOKEN_TYPE type;
	char lexeme[128];
	uint16_t lineno;

} Token;

typedef struct Lexer {
	uint16_t LineNo; // 65536 lines till overflow
} Lexer;

void InitLexer(Lexer* lexer) {
	if (!lexer) {
		printf("Lexer was NULL on init");
		return;
	}
	lexer->LineNo = 1;
	return;
}

Token getNextToken(Lexer* lexer) {
	char c = consumeChar();
	switch (c)
	{
	// Skip white space
	case '(':
	{
		Token token = { LP, "(", lexer->LineNo};
		return token;
	}
	default:
		break;
	}
}