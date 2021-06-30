#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "lfio.h"

FILE *d_Open(const char* filename)
{
  FILE *fp = fopen(filename, "rb");

	if (fp == NULL) {
		printf("Error opening file : %s", filename);
    /*
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			printf(" - File not found");
    */
		printf("\n");
	}

	return fp;
}

int d_Close(FILE* file)
{
	return fclose(file);
}

int d_Read(FILE* file, char* buffer, int need)
{
  // Get file descriptor from file pointer 
  int fd = fileno(file);

  return fread(buffer, sizeof(char), need, file);
}

void d_GetFileSize(FILE* file, uint32_t* fileSize){
  fseek(file, 0, SEEK_END); // seek to end of file
  *fileSize = ftell(file);  // get current file pointer
  rewind(file);             // seek back to begining of file
}
