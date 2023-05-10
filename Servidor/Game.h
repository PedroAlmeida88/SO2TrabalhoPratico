#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#define TAM_LANE 20
#define TOTAL_LANES 10


typedef struct {
    //int velocity;               //Velocidade da faixa de rodagem
    TCHAR estrada[TAM_LANE];    //Numero de faixas de rodagem
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


Game* createGame(int lanesNumber, int velocidade);

void initGame(Game* game);

void show(Game* game);

DWORD WINAPI lanesFunction(LPVOID param);

int randNum(int min, int max);

void initCars(Game* game, int laneNumber);

void moveCars(TCHAR* direction, Game* game, int laneNumber, int* nCarros);

#endif