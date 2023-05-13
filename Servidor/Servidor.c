#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Reg.h"
#include "Game.h"
//#include "Dll.h"

#define TAM 200
#define MAX_LANES 10
#define TAM_BUFFER 10
#define NUM_THREADS 10


DWORD WINAPI ThreadEscreverGame(LPVOID param) {
    TCHAR msg[20];
    ThreadDadosGame* dados = (ThreadDadosGame*)param;
    SharedStruct sharedStruct;
    while (!dados->terminar) {
        WaitForSingleObject(dados->hMutex, INFINITE);
        for (int i = 0; i < dados->sharedStruct->game->total_lanes; i++)
        {
            for (int j = 0; j < TAM_LANE; j++)
            {
                dados->sharedStruct->game[i].estrada[j] = dados->game[i].estrada[j];

            }
        }
        ReleaseMutex(dados->hMutex);
        SetEvent(dados->hEvent);
        Sleep(500);
        ResetEvent(dados->hEvent);
    }
    return 0;
}

int _tmain(int argc, LPTSTR argv[]) {
    GameData gameData;

    HANDLE hFileMap; //handle para o file map
    HANDLE hThread[NUM_THREADS], hSemaphoreUnique,hTheadGameShare;
    ControlData dados[NUM_THREADS];
    TCHAR comando[100];
    ThreadDadosGame dadosGame;
    Game game[MAX_LANES];    
    DadosLanesThread dadosLanes[MAX_LANES - 2]; 
    HANDLE mutex[MAX_LANES - 2];


#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    srand(time(NULL));
    //Semaforo para cotrolar o numero de servidores ativos
    hSemaphoreUnique = CreateSemaphore(NULL,1,2,TEXT("CONTROL"));

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        _ftprintf(stderr, TEXT("[ERRO] Já existe um servidor aberto!\n"));
        return -1;
    }

    registryCheck(argv, &gameData);
    _tprintf_s(TEXT("\nVelocidade %d | Numero de faixas de rodagem %d\n"), gameData.starterVelocity, gameData.lanesNumber);

    initMemAndSync(&dados);
    initMemAndSyncGame(&dadosGame);

    game->total_lanes = gameData.lanesNumber;

    WaitForSingleObject(dados[0].hMutex, INFINITE);
    dados[0].dadosLanes = dadosLanes;
    ReleaseMutex(dados[0].hMutex);

    hThread[0] = CreateThread(NULL, 0, recieveCommands, &dados, 0, NULL);
    if (hThread[0] == NULL) {
        return;
    }
    
    dadosGame.game = &game;
    hTheadGameShare = CreateThread(NULL, 0, ThreadEscreverGame, &dadosGame, 0, NULL);
    if (hTheadGameShare == NULL) {
        return;
    }
    
    initGame(&game);
    
    WaitForSingleObject(dadosGame.hMutex, INFINITE);
    dadosGame.sharedStruct->game->total_lanes = gameData.lanesNumber;
    ReleaseMutex(dadosGame.hMutex);

    //inicializar os dados das threads e criar uma thread para cada via
    for (int i = 1; i < game->total_lanes - 1; i++) {
        dadosLanes[i].terminar = 0;
        dadosLanes[i].velocity = gameData.starterVelocity;
        dadosLanes[i].currDirection = 1;
        dadosLanes[i].game = &game;
        mutex[i] = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_LANES"));
        dadosLanes[i].hMutex = mutex[i];
        dadosLanes[i].laneNumber = i;
        dadosLanes[i].stop = FALSE;        
        hThread[i] = CreateThread(NULL, 0, lanesFunction, &dadosLanes[i], 0, NULL);
    }

    for (int i = 1; i < game->total_lanes - 1; i++) {
        WaitForSingleObject(hThread[i], INFINITE);
    }


    WaitForSingleObject(hThread[0], INFINITE);
    UnmapViewOfFile(dados[0].memPar);

    return 0;
}