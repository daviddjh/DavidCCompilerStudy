#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "dinput.h"
#include "dynarray.h"

#define IS_ALPHANUMERIC(x) (48 <= x && x <= 57) || (65 <= x && x <= 90) || (97 <= x && x <= 122) || x == 95 // asci values for 0-9, A-Z, _, a-z , in order
#define IS_NUMERIC(x) (48 <= x && x <= 57) // asci values for 0-9

void InitLexer(Lexer* lexer) {
	if (!lexer) {
		printf("Lexer was NULL on init");
		return;
	}
	lexer->LineNo = 1;
	lexer->tokens = dd_makeDynamicArray();
	lexer->nextToken = 0;
	return;
}

Lexer* dl_lex(Lexer* lexer, const char * filename) {
    ii_loadFile(filename);
	Token* currentToken = malloc(sizeof(Token));
	*currentToken = makeNextToken(lexer);
	while (currentToken->type != EOFT) {
		dd_push(lexer->tokens, currentToken);
	    currentToken = malloc(sizeof(Token));
		*currentToken = makeNextToken(lexer);
	}
	dd_push(lexer->tokens, currentToken);
}

void dl_destory(Lexer* lex) {
	free(lex);
}

Token makeNextToken(Lexer* lexer) {
	lexer->currentLexeme = calloc(128, sizeof(char));
	for (;;) {
		char c = lookAhead(1);
		switch (c)
		{
			// Scanning error
		case -1:
		{
			Token token = { LEX_ERROR, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		// EOF
		case '\0':
		{
			Token token = { EOFT, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		// Skip white space
		case '\n':
			lexer->LineNo++;
		case ' ':
		case 9:          // tab
			consumeChar();
			break;
		case '(':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "(", 1);
			Token token = { LP, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case ')':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, ")", 1);
			Token token = { RP, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '{':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "{", 1);
			Token token = { LS, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '}':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "}", 1);
			Token token = { RS, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case ';':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, ";", 1);
			Token token = { SEMI, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '+':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "+", 1);
			Token token = { PLUS, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '-':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "-", 1);
			Token token = { MINUS, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '*':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "*", 1);
			Token token = { MULT, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '/':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "/", 1);
			Token token = { DIVIDE, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '!':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "!", 1);
			Token token = { NOT, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '&':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "&", 1);
			Token token = { AND, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '|':
		{
			consumeChar();
			memcpy(lexer->currentLexeme, "|", 1);
			Token token = { OR, lexer->currentLexeme, lexer->LineNo };
			return token;
			break;
		}
		case '>':
		{
			consumeChar();
			if (lookAhead(1) == '=') {
				consumeChar();
				memcpy(lexer->currentLexeme, ">=", 2);
				Token token = { MTOE, lexer->currentLexeme, lexer->LineNo };
				return token;
				break;

			}
			else {
				memcpy(lexer->currentLexeme, ">", 1);
				Token token = { MT, lexer->currentLexeme, lexer->LineNo };
				return token;
				break;
			}
		}
		case '<':
		{
			consumeChar();
			if (lookAhead(1) == '=') {
				consumeChar();
				memcpy(lexer->currentLexeme, "<=", 2);
				Token token = { LTOE, lexer->currentLexeme, lexer->LineNo };
				return token;
				break;

			}
			else {
				memcpy(lexer->currentLexeme, "<", 1);
				Token token = { LT, lexer->currentLexeme, lexer->LineNo };
				return token;
				break;
			}
		}
		case '=':
		{
			consumeChar();
			if (lookAhead(1) == '=') {
				consumeChar();
				memcpy(lexer->currentLexeme, "==", 2);
				Token token = { EQUIV, lexer->currentLexeme, lexer->LineNo };
				return token;
				break;
			}
			else {
				memcpy(lexer->currentLexeme, "=", 1);
				Token token = { ASSIGN, lexer->currentLexeme, lexer->LineNo };
				return token;
				break;
			}
		}
		default:
		{
			if (IS_NUMERIC(c)) {
				return makeNumberToken(lexer);
			}
			else if (IS_ALPHANUMERIC(c)) {
				return makeWordToken(lexer);
			}
			else {
				consumeChar();
				Token token = { LEX_ERROR, lexer->currentLexeme, lexer->LineNo }; // fix this and add the char
				return token;
			}
		}
		}
	}
}

Token makeWordToken(Lexer* lexer) {
	// White space is already gone
	char* currentChar = lexer->currentLexeme;
	while (currentChar - lexer->currentLexeme < 128) {
		char c = lookAhead(1);
		if (IS_ALPHANUMERIC(c)) {
			*currentChar = consumeChar();
			currentChar++;
		}
		else {
			break;
		}
	}
	if (currentChar - (lexer->currentLexeme) == 0) {
		Token token = { LEX_ERROR, lexer->currentLexeme, lexer->LineNo };
		return token;
	}
	else {
		if (strcmp(lexer->currentLexeme, "if") == 0) {

			Token token = { IF, lexer->currentLexeme, lexer->LineNo };
			return token;

		}
		else if (strcmp(lexer->currentLexeme, "int") == 0) {

			Token token = { INT, lexer->currentLexeme, lexer->LineNo };
			return token;

		}
		else if (strcmp(lexer->currentLexeme, "bool") == 0) {

			Token token = { BOOL, lexer->currentLexeme, lexer->LineNo };
			return token;

		}
		else if (strcmp(lexer->currentLexeme, "return") == 0) {

			Token token = { RETURN, lexer->currentLexeme, lexer->LineNo };
			return token;

		}
		else {

			Token token = { IDENTIFIER, lexer->currentLexeme, lexer->LineNo };
			return token;
		}
	}
}

Token makeNumberToken(Lexer* lexer) {
	// White space is already gone
	char* currentChar = lexer->currentLexeme;
	while (currentChar - lexer->currentLexeme < 128) {
		char c = lookAhead(1);
		if (IS_NUMERIC(c)) {
			*currentChar = consumeChar();
			currentChar++;
		}

		// alphanumeric words that start with a number are invalid
		else if (IS_ALPHANUMERIC(c)) { 
			// consume any other Alphanumeric chars
			do {
				*currentChar = consumeChar();
				currentChar++;
				c = lookAhead(1);
			} while (((currentChar - lexer->currentLexeme) < 128) && (IS_ALPHANUMERIC(c)));

			Token token = { LEX_ERROR, lexer->currentLexeme, lexer->LineNo };
			return token;
		}
		else {
			break;
		}
	}
	Token token = { INT_LITERAL, lexer->currentLexeme, lexer->LineNo };
	return token;
}

Token* getNextToken(Lexer* lexer) {
	if (lexer->nextToken != lexer->tokens->capacity - 1) {
		return *((lexer->tokens->array) + lexer->nextToken++);
	}
	else {
		return *((lexer->tokens->array) + lexer->nextToken);
	}

}

Token* peakNextToken(Lexer* lexer) {
	return *((lexer->tokens->array) + lexer->nextToken);
}
