#pragma once
#include <stdint.h>
#include <Windows.h>

typedef struct d_file {
	uint32_t fileSize;
	char* fileData;
	char* Eof;
	char* currentChar;
	HANDLE fileHandle;
} d_file;

typedef enum {
	d_fileRead,
	d_fileWrite,
	d_fileReadWrite
} d_fileAccess;

void d_loadFile(const char* fileName, d_file* fileState, d_fileAccess fileAccess);

char consumeChar(d_file* fileState);

char lookAhead(int i, d_file* fileState);

char putbackChar(int i, uint16_t* lineNo, d_file* fileState);

void d_closeFileHandle(d_file* fileState);

void d_appendString(d_file* fileState, const char* stringToWrite);

void d_appendInt(d_file* fileState, int intToWrite);

