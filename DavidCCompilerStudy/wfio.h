#pragma once

#include <handleapi.h>

HANDLE d_Open(const char* filename);

int d_Close(HANDLE file);

int d_Read(HANDLE file, void* buffer, int need);
