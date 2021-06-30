#pragma once
#include <stdint.h>

typedef enum TOKEN_TYPE {
	LP,         // '('
	RP,         // ')'
	LS,         // '{'
	RS,         // '}'
	SEMI,       // ';'
	MT,         // >
	LT,         // <
	ASSIGN,     // =
	AND,        // &
	OR,         // |
	MTOE,       // >=
	LTOE,       // <=
	EQUIV,       // ==
	IF,         // 'if'
	INT,        // int
	BOOL,       // bool
	IDENTIFIER, // function name or variable
	RETURN,     // return
  LEX_ERROR   // Internal way of expressing scanning error
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
} Lexer;

void InitLexer(Lexer* lexer);

Token getNextToken(Lexer* lexer);
Token getWordToken(Lexer* lexer);
