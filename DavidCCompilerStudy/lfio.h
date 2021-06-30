#pragma once

#include <stdio.h>

FILE *d_Open(const char* filename);

int d_Close(FILE* file);

int d_Read(FILE* file, char* buffer, int need);

void d_GetFileSize(FILE* file, uint32_t* fileSize);
