#ifndef PIPE_H
#define PIPE_H
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Game.h"

#define MAX_CLI 2
#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")

typedef struct {
	HANDLE hInstancia;
	OVERLAPPED overlap;
	BOOL activo;
} DadosPipe;

typedef struct {
	DadosPipe hPipes[MAX_CLI];
	HANDLE hEvents[MAX_CLI];
	HANDLE hMutex;
	Game* game;
	int terminar;
} ThreadDadosPipe;

DWORD WINAPI ThreadMensagens(LPVOID param);

#endif