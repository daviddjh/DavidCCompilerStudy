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
	lexer->fileState = malloc(sizeof(d_file));
	return;
}

Lexer* dl_lex(Lexer* lexer, const char * filename) {
    d_loadFile(filename, lexer->fileState, d_fileRead);
	// Don't need it since we loaded it into memory
	d_closeFileHandle(lexer->fileState);
	Token* currentToken = malloc(sizeof(Token));
	*currentToken = makeNextToken(lexer);
	while (currentToken->type != dl_EOFT) {
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
		char c = lookAhead(1, lexer->fileState);
		switch (c)
		{
			// Scanning error
		case -1:
		{
			Token token = { dl_LEX_ERROR, lexer->currentLexeme, lexer->LineNo, 0};
			return token;
			break;
		}
		// EOF
		case '\0':
		{
			Token token = { dl_EOFT, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		// Skip white space
		case '\n':
			lexer->LineNo++;
		case '\r':
		case ' ':
		case 9:          // tab
			consumeChar(lexer->fileState);
			break;
		case '(':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "(", 1);
			Token token = { dl_LP, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case ')':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, ")", 1);
			Token token = { dl_RP, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '{':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "{", 1);
			Token token = { dl_LS, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '}':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "}", 1);
			Token token = { dl_RS, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case ';':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, ";", 1);
			Token token = { dl_SEMI, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '+':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "+", 1);
			Token token = { dl_PLUS, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '-':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "-", 1);
			Token token = { dl_MINUS, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '*':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "*", 1);
			Token token = { dl_MULT, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '/':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "/", 1);
			Token token = { dl_DIVIDE, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '!':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "!", 1);
			Token token = { dl_NOT, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '&':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "&", 1);
			Token token = { dl_AND, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '|':
		{
			consumeChar(lexer->fileState);
			memcpy(lexer->currentLexeme, "|", 1);
			Token token = { dl_OR, lexer->currentLexeme, lexer->LineNo, 1};
			return token;
			break;
		}
		case '>':
		{
			consumeChar(lexer->fileState);
			if (lookAhead(1, lexer->fileState) == '=') {
				consumeChar(lexer->fileState);
				memcpy(lexer->currentLexeme, ">=", 2);
				Token token = { dl_MTOE, lexer->currentLexeme, lexer->LineNo, 2};
				return token;
				break;

			}
			else {
				memcpy(lexer->currentLexeme, ">", 1);
				Token token = { dl_MT, lexer->currentLexeme, lexer->LineNo, 1};
				return token;
				break;
			}
		}
		case '<':
		{
			consumeChar(lexer->fileState);
			if (lookAhead(1, lexer->fileState) == '=') {
				consumeChar(lexer->fileState);
				memcpy(lexer->currentLexeme, "<=", 2);
				Token token = { dl_LTOE, lexer->currentLexeme, lexer->LineNo, 2};
				return token;
				break;

			}
			else {
				memcpy(lexer->currentLexeme, "<", 1);
				Token token = { dl_LT, lexer->currentLexeme, lexer->LineNo, 1};
				return token;
				break;
			}
		}
		case '=':
		{
			consumeChar(lexer->fileState);
			if (lookAhead(1, lexer->fileState) == '=') {
				consumeChar(lexer->fileState);
				memcpy(lexer->currentLexeme, "==", 2);
				Token token = { dl_EQUIV, lexer->currentLexeme, lexer->LineNo, 2};
				return token;
				break;
			}
			else {
				memcpy(lexer->currentLexeme, "=", 1);
				Token token = { dl_EQUALS, lexer->currentLexeme, lexer->LineNo, 1};
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
				consumeChar(lexer->fileState);
				Token token = { dl_LEX_ERROR, lexer->currentLexeme, lexer->LineNo, 0}; // fix this and add the char
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
		char c = lookAhead(1, lexer->fileState);
		if (IS_ALPHANUMERIC(c)) {
			*currentChar = consumeChar(lexer->fileState);
			currentChar++;
		}
		else {
			break;
		}
	}
	if (currentChar - (lexer->currentLexeme) == 0) {
		Token token = { dl_LEX_ERROR, lexer->currentLexeme, lexer->LineNo };
		return token;
	}
	else {
		if (strcmp(lexer->currentLexeme, "if") == 0) {

			Token token = { dl_IF, lexer->currentLexeme, lexer->LineNo, 2};
			return token;

		}
		else if (strcmp(lexer->currentLexeme, "int") == 0) {

			Token token = {dl_INT, lexer->currentLexeme, lexer->LineNo, 3};
			return token;

		}
		else if (strcmp(lexer->currentLexeme, "bool") == 0) {

			Token token = { dl_BOOL, lexer->currentLexeme, lexer->LineNo, 4};
			return token;

		}
		else if (strcmp(lexer->currentLexeme, "return") == 0) {

			Token token = { dl_RETURN, lexer->currentLexeme, lexer->LineNo, 6};
			return token;

		}
		else {

			Token token = { dl_IDENT, lexer->currentLexeme, lexer->LineNo, currentChar - lexer->currentLexeme};
			return token;
		}
	}
}

Token makeNumberToken(Lexer* lexer) {
	// White space is already gone
	char* currentChar = lexer->currentLexeme;
	while (currentChar - lexer->currentLexeme < 128) {
		char c = lookAhead(1, lexer->fileState);
		if (IS_NUMERIC(c)) {
			*currentChar = consumeChar(lexer->fileState);
			currentChar++;
		}

		// alphanumeric words that start with a number are invalid
		else if (IS_ALPHANUMERIC(c)) { 
			// consume any other Alphanumeric chars
			do {
				*currentChar = consumeChar(lexer->fileState);
				currentChar++;
				c = lookAhead(1, lexer->fileState);
			} while (((currentChar - lexer->currentLexeme) < 128) && (IS_ALPHANUMERIC(c)));

			Token token = { dl_LEX_ERROR, lexer->currentLexeme, lexer->LineNo, 0};
			return token;
		}
		else {
			break;
		}
	}
	Token token = { dl_INT_LITERAL, lexer->currentLexeme, lexer->LineNo, currentChar - lexer->currentLexeme};
	return token;
}

Token* eatNextToken(Lexer* lexer) {
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
