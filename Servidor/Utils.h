#ifndef UTILS_H
#define UTILS_H

#define SUSPERNDER 1
#define RETOMAR 2
#define REINICIAR 3 
#define SAIR 4

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#define STR_SIZE 100

TCHAR** splitString(TCHAR* str, const TCHAR* delim, int* size);

BOOL isStringANumber(TCHAR* str);

int VerifyCommandConsole(TCHAR* comando);

#endif