#pragma once

#include <stdint.h>
#include <handleapi.h>

// Open a file for reading. must exist
HANDLE d_OpenR(const char* filename);

// Open a file for writing. Doesn't have to exist, will make one if needed
HANDLE d_OpenW(const char* filename);

int d_Close(HANDLE file);

int d_Read(HANDLE file, void* buffer, int need);

void d_GetFileSize(HANDLE file, uint32_t* fileSize);
