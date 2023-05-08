#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#define STR_SIZE 100


TCHAR** splitString(TCHAR* str, const TCHAR* delim, int* size);

BOOL isStringANumber(TCHAR* str);

#endif