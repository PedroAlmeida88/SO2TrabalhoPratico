#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Dll.h"
#define TAM_LANE 20
//#define TOTAL_LANES 10

/*
typedef struct {
    int total_lanes;            //num de estradas
    TCHAR estrada[TAM_LANE];    
}Game;

typedef struct {
    int terminar;
    int laneNumber;
    int* currDirection; //1->right 0->left
    int* velocity;
    BOOL stop;
    HANDLE hMutex;
    Game* game;
}DadosLanesThread;
*/
Game* createGame(int lanesNumber);

void initGame(Game* game);

void show(Game* game);

DWORD WINAPI lanesFunction(LPVOID param);

void copyGame(Game* original, int size, Game* new);

int randNum(int min, int max);

void initCars(Game* game, int laneNumber);

void moveCars(TCHAR* direction, Game* game, int laneNumber, int* nCarros);

#endif