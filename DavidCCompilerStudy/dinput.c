#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include "wfio.h"
#elif defined(__unix__)
#include "lfio.h"
#define HANDLE FILE*
#endif

// not thread safe!
uint32_t fileSize = 0;
char* fileData = NULL;
char* Eof = NULL;
char* currentChar = NULL;

void ii_loadFile(const char * fileName) {

	// Open the file and get it's size
	HANDLE file = d_Open(fileName);
	d_GetFileSize(file, &fileSize);

	// If there was a previos file loaded, free the memory to avoid mem leaks
	if (fileData != NULL)
		free(fileData);

	// Allocate space to fill with file
	fileData = malloc(sizeof(char) * fileSize + 1);

	// Read the file into memory
	d_Read(file, fileData, fileSize);

	// Set Eof pointer
	Eof = fileData + fileSize;
	*Eof = '\0';

	// Close the handle
	d_Close(file);

	// Set current char to the beginning of file
	currentChar = fileData;
}

char consumeChar() {
	if (currentChar < Eof)
		return *currentChar++;
	else if (currentChar == Eof)
		return '\0';
	else
		return -1;
}

char lookAhead(int i) {
	if ((currentChar + i - 1) >= fileData && (currentChar + i - 1) < Eof)
		return *(currentChar + i - 1);
	else if ((currentChar + i -1 ) == Eof)
		return '\0';
	else
		return -1;
}

char putbackChar(int i, uint16_t * lineNo) {
	if (currentChar - i >= fileData){
    for(int j = 0; j < i; j++){
      if(*currentChar == '\n') *(lineNo)--;
      currentChar--;
    }
		return *(currentChar);
  }
	else
		return -1;
}


