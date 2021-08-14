#pragma once
#include <stdint.h>

#include "dynarray.h"
#include "dinput.h"

typedef enum dl_TOKEN_TYPE {
	dl_EOFT = -1,   // 'EOF' Token
	dl_LP,          // '('
	dl_RP,          // ')'
	dl_LS,          // '{'
	dl_RS,          // '}'
	dl_SEMI,        // ';'
	dl_PLUS,        // +
	dl_MINUS,       // -
	dl_MULT,        // *
	dl_DIVIDE,      // /
	dl_NOT,         // !
	dl_MT,          // >
	dl_LT,          // <
	dl_EQUALS,      // =
	dl_AND,         // &
	dl_OR,          // |
	dl_MTOE,        // >=
	dl_LTOE,        // <=
	dl_EQUIV,       // ==
	dl_IF,          // 'if'
	dl_INT,         // int
	dl_BOOL,        // bool
	dl_RETURN,      // return
	dl_INT_LITERAL, // 0-9+
	dl_IDENT,       // function name or variable
    dl_LEX_ERROR    // Internal way of expressing scanning error
} dl_TOKEN_TYPE;

typedef struct Token {

	dl_TOKEN_TYPE type;
  // This could just be a pointer to the buffer in dinput.c
  // but that means the lifetime of the buffer in dinput.c
  // should last the amount of time the tokens are alive
	char * lexeme;
	uint16_t lineno;
	uint8_t lexeme_length;

} Token;

typedef struct Lexer {
	char * currentLexeme;
	uint16_t LineNo; // 65536 lines till overflow
	DynamicArray * tokens;
	int nextToken;
	d_file * fileState;
} Lexer;

void InitLexer(Lexer* lexer);

Lexer* dl_lex(Lexer* , const char *);
void dl_destroy(Lexer* );

Token makeNextToken(Lexer* lexer);
Token makeWordToken(Lexer* lexer);
Token makeNumberToken(Lexer* lexer);
Token* eatNextToken(Lexer* lexer);
Token* peakNextToken(Lexer* lexer);
