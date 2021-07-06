#pragma once
#include <stdint.h>

#include "dynarray.h"

typedef enum TOKEN_TYPE {
	EOFT = -1,   // 'EOF' Token
	LP,          // '('
	RP,          // ')'
	LS,          // '{'
	RS,          // '}'
	SEMI,        // ';'
	PLUS,        // +
	MINUS,       // -
	MULT,        // *
	DIVIDE,      // /
	NOT,         // !
	MT,          // >
	LT,          // <
	ASSIGN,      // =
	AND,         // &
	OR,          // |
	MTOE,        // >=
	LTOE,        // <=
	EQUIV,       // ==
	IF,          // 'if'
	INT,         // int
	BOOL,        // bool
	RETURN,      // return
	INT_LITERAL, // 0-9+
	IDENTIFIER,  // function name or variable
    LEX_ERROR    // Internal way of expressing scanning error
} TOKEN_TYPE;

typedef struct Token {

	TOKEN_TYPE type;
  // This could just be a pointer to the buffer in dinput.c
  // but that means the lifetime of the buffer in dinput.c
  // should last the amount of time the tokens are alive
	char * lexeme;
	uint16_t lineno;

} Token;

typedef struct Lexer {
	char * currentLexeme;
	uint16_t LineNo; // 65536 lines till overflow
	DynamicArray * tokens;
	int nextToken;
} Lexer;

void InitLexer(Lexer* lexer);

Lexer* dl_lex(Lexer* , const char *);
void dl_destroy(Lexer* );

Token makeNextToken(Lexer* lexer);
Token makeWordToken(Lexer* lexer);
Token makeNumberToken(Lexer* lexer);
Token* getNextToken(Lexer* lexer);
Token* peakNextToken(Lexer* lexer);
