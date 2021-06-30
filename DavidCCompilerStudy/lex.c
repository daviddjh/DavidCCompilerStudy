#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "dinput.h"

#define IS_ALPHANUMERIC(x) (48 <= x <= 57) || (65 <= x <= 90) || (97 <= x <= 122) || x == 95 // asci values for 0-9, A-Z, _, a-z , in order
#define IS_NUMERIC(x) (48 <= x <= 57) // asci values for 0-9

void InitLexer(Lexer* lexer) {
	if (!lexer) {
		printf("Lexer was NULL on init");
		return;
	}
	lexer->LineNo = 1;
	return;
}

Token getNextToken(Lexer* lexer) {
  lexer->currentLexeme = (char*)calloc(128, sizeof(char));
  for(;;){
    char c = lookAhead(1);
    switch (c)
    {
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
      Token token = { LP, lexer->currentLexeme, lexer->LineNo};
      return token;
      break;
    }
    case ')':
    {
      consumeChar();
      memcpy(lexer->currentLexeme, ")", 1);
      Token token = { RP, lexer->currentLexeme, lexer->LineNo};
      return token;
      break;
    }
    case '{':
    {
      consumeChar();
      memcpy(lexer->currentLexeme, "{", 1);
      Token token = { LS, lexer->currentLexeme, lexer->LineNo};
      return token;
      break;
    }
    case '}':
    {
      consumeChar();
      memcpy(lexer->currentLexeme, "}", 1);
      Token token = { RS, lexer->currentLexeme, lexer->LineNo};
      return token;
      break;
    }
    case ';':
    {
      consumeChar();
      memcpy(lexer->currentLexeme, ";", 1);
      Token token = { SEMI, lexer->currentLexeme, lexer->LineNo};
      return token;
      break;
    }
    case '&':
    {
      consumeChar();
      memcpy(lexer->currentLexeme, "&", 1);
      Token token = { AND, lexer->currentLexeme, lexer->LineNo};
      return token;
      break;
    }
    case '|':
    {
      consumeChar();
      memcpy(lexer->currentLexeme, "|", 1);
      Token token = { OR, lexer->currentLexeme, lexer->LineNo};
      return token;
      break;
    }
    case '>':
    {
      consumeChar();
      if(lookAhead(1) == '='){
        consumeChar();
        memcpy(lexer->currentLexeme, ">=", 2);
        Token token = { MTOE, lexer->currentLexeme, lexer->LineNo};
        return token;
        break;

      } else {
        memcpy(lexer->currentLexeme, ">", 1);
        Token token = { MT, lexer->currentLexeme, lexer->LineNo};
        return token;
        break;
      }
    }
    case '<':
    {
      consumeChar();
      if(lookAhead(1) == '='){
        consumeChar();
        memcpy(lexer->currentLexeme, "<=", 2);
        Token token = { LTOE, lexer->currentLexeme, lexer->LineNo};
        return token;
        break;

      } else {
        memcpy(lexer->currentLexeme, "<", 1);
        Token token = { LT, lexer->currentLexeme, lexer->LineNo};
        return token;
        break;
      }
    }
    case '=':
    {
      consumeChar();
      if(lookAhead(1) == '='){
        consumeChar();
        memcpy(lexer->currentLexeme, "==", 2);
        Token token = { EQUIV, lexer->currentLexeme, lexer->LineNo};
        return token;
        break;
      } else {
        memcpy(lexer->currentLexeme, "=", 1);
        Token token = { ASSIGN, lexer->currentLexeme, lexer->LineNo};
        return token;
        break;
      }
    }
    default:
    {
      if(IS_NUMERIC(c)){
        //return getNumberToken(lexer);
      } else if (IS_ALPHANUMERIC(c)){
        return getWordToken(lexer);
      } else {
        consumeChar();
        Token token = {LEX_ERROR, lexer->currentLexeme, lexer->LineNo}; // fix this and add the char
        return token;
      }
    }
    }
	}
}

Token getWordToken(Lexer* lexer){
  // White space is already gone
  char * currentChar = lexer->currentLexeme;
  while(currentChar-lexer->currentLexeme < 128){
    char c = lookAhead(1);
    if(IS_ALPHANUMERIC(c)){
      *currentChar = consumeChar();  
      currentChar++;
    } else {
      break;
    }
  }
  if(currentChar-(lexer->currentLexeme)== 0){
    Token token = {LEX_ERROR, lexer->currentLexeme, lexer->LineNo};
    return token;
  } else {
    if( strcmp(lexer->currentLexeme, "if") == 0 ){

      Token token = {LEX_ERROR, lexer->currentLexeme, lexer->LineNo};
      return token;

    } else if(strcmp(lexer->currentLexeme, "int") == 0){

      Token token = {LEX_ERROR, lexer->currentLexeme, lexer->LineNo};
      return token;
       
    } else if(strcmp(lexer->currentLexeme, "bool") == 0){
       
      Token token = {LEX_ERROR, lexer->currentLexeme, lexer->LineNo};
      return token;
       
    } else if(strcmp(lexer->currentLexeme, "return") == 0){
       
      Token token = {LEX_ERROR, lexer->currentLexeme, lexer->LineNo};
      return token;
       
    } else {

      Token token = {LEX_ERROR, lexer->currentLexeme, lexer->LineNo};
      return token;
    }
  }
}
