#include <windows.h>
#include <stdint.h>

#include "wfio.h"

// not thread safe!
uint32_t fileSize = 0;
char* fileData = NULL;
char* Eof = NULL;
char* currentChar = NULL;

void ii_loadFile(char * fileName) {

	// Open the file and get it's size
	HANDLE file = d_Open(fileName);
	GetFileSizeEx(file, &fileSize);

	// If there was a previos file loaded, free the memory to avoid mem leaks
	if (fileData != NULL)
		free(fileData);

	// Allocate space to fill with file
	fileData = malloc(sizeof(char) * fileSize);

	// Read the file into memory
	d_Read(file, fileData, fileSize);

	// Set Eof pointer
	Eof = fileData + fileSize - 1;

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
	if ((currentChar + i) >= fileData && (currentChar + i) <= Eof)
		return *(currentChar + i);
	else
		return -1;
}

char putbackChar(int i) {
	if (currentChar - i > fileSize)
		return *(currentChar -= i);
	else
		return -1;
}


