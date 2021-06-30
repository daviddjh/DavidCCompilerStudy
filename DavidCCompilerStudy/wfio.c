#include <Windows.h>

#include "wfio.h"


HANDLE d_Open(const char* filename)
{
	HANDLE temp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (temp == INVALID_HANDLE_VALUE) {
		printf("Error opening file : %s", filename);
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			printf(" - File not found");
		printf("\n");
	}

	return temp;
}

int d_Close(HANDLE file)
{
	return CloseHandle(file);
}

int d_Read(HANDLE file, void * buffer, int need)
{
	unsigned long numBytesRead;

	if (!ReadFile(file, buffer, need, &numBytesRead, NULL))
		return -1;
	else
		return numBytesRead;
}

void d_GetFileSize(HANDLE file, uint32_t* fileSize){
	GetFileSizeEx(file, fileSize);
}
