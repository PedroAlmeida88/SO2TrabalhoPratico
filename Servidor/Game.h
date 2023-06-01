#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Dll.h"
#define TAM_LANE 20

Game* createGame(int lanesNumber);

void initGame(Game* game);

void show(Game* game);

DWORD WINAPI lanesFunction(LPVOID param);

void copyGame(Game* original, int size, Game* new);

int randNum(int min, int max);

void initCars(Game* game, int laneNumber);

void moveCars(TCHAR* direction, Game* game, int laneNumber, int* nCarros);

#endif