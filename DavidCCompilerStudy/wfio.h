#pragma once

#include <stdint.h>
#include <handleapi.h>

HANDLE d_Open(const char* filename);

int d_Close(HANDLE file);

int d_Read(HANDLE file, void* buffer, int need);

void d_GetFileSize(HANDLE file, uint32_t* fileSize);
