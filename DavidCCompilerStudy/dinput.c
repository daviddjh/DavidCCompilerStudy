#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "dinput.h"

#ifdef _WIN32
#include <windows.h>
#include "wfio.h"
#elif defined(__unix__)
#include "lfio.h"
#define HANDLE FILE*
#endif

void d_loadFile(const char * fileName, d_file* fileState, d_fileAccess fileAccess) {

	// Init fileState
	fileState->currentChar = NULL;
	fileState->fileData = NULL;
	fileState->Eof = NULL;

	// Open the file and get it's size
	if (fileAccess == d_fileRead) {
		fileState->fileHandle = d_OpenR(fileName);
	}
	else if (fileAccess == d_fileWrite) {
		fileState->fileHandle = d_OpenW(fileName);
	}
	else {
		fprintf(stderr, "Cant do that yet lol DO IT AHAHAHAH 7/20\n");
		exit(1);
		return;
	}
	d_GetFileSize(fileState->fileHandle, &(fileState->fileSize));

	// If there was a previos file loaded, free the memory to avoid mem leaks
	if (fileState->fileData != NULL)
		free(fileState->fileData);

	// Allocate space to fill with file
	fileState->fileData = malloc(sizeof(char) * (fileState->fileSize + 1));

	// Read the file into memory
	d_Read(fileState->fileHandle, fileState->fileData, fileState->fileSize);

	// Set Eof pointer
	fileState->Eof = fileState->fileData + fileState->fileSize;
	*(fileState->Eof) = '\0';

	// Close the handle if the file was open for reading (we dont need it anymore,
	// it's in memory)
	//if (fileAccess == d_fileRead) {
	//	d_Close(file);
	//}

	// Set current char to the beginning of file
	fileState->currentChar = fileState->fileData;
}

char consumeChar(d_file* fileState) {
	if (fileState->currentChar < fileState->Eof)
		return *(fileState->currentChar)++;
	else if (fileState->currentChar == fileState->Eof)
		return '\0';
	else
		return -1;
}

char lookAhead(int i, d_file* fileState) {
	if ((fileState->currentChar + i - 1) >= fileState->fileData && (fileState->currentChar + i - 1) < fileState->Eof)
		return *(fileState->currentChar + i - 1);
	else if ((fileState->currentChar + i -1 ) == fileState->Eof)
		return '\0';
	else
		return -1;
}

char putbackChar(int i, uint16_t * lineNo, d_file* fileState) {
	if (fileState->currentChar - i >= fileState->fileData){
    for(int j = 0; j < i; j++){
      if(*(fileState->currentChar) == '\n') *(lineNo)--;
      (fileState->currentChar)--;
    }
		return *(fileState->currentChar);
  }
	else
		return -1;
}

void d_closeFileHandle(d_file* fileState) {
	d_CloseHandle(fileState->fileHandle);
}

void d_appendString(d_file* fileState, const char* stringToWrite) {
	DWORD numBytesWritten = 0;
	WriteFile(fileState->fileHandle, stringToWrite, strlen(stringToWrite), &numBytesWritten, NULL);
}

void d_appendInt(d_file* fileState, int intToWrite) {
	// Limits to ints 256 numbers long
	char* stringToWrite = calloc(256, sizeof(char));
	// Todo: make this better, why does this need to be 256?
	sprintf_s(stringToWrite, 256, "%d", intToWrite);
	d_appendString(fileState, stringToWrite);
}


