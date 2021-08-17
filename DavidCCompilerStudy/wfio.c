#include <stdio.h>
#include <Windows.h>

#include "wfio.h"


HANDLE d_OpenR(const char* filename)
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

HANDLE d_OpenW(const char* filename)
{
	HANDLE temp = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD error = GetLastError();

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

void d_CloseHandle(HANDLE file) {
	LPBY_HANDLE_FILE_INFORMATION lpFileInfo = malloc(sizeof(BY_HANDLE_FILE_INFORMATION));
	if (GetFileInformationByHandle(file, lpFileInfo) != 0) {
		d_Close(file);
	}
}
