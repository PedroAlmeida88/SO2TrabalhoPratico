#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#define TAM_LANE 20

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#define TAM_BUFFER 10


DLL_IMP_API typedef struct {
    int total_lanes;            //num de estradas
    TCHAR estrada[TAM_LANE];
}Game;

DLL_IMP_API typedef struct {
    int terminar;
    int laneNumber;
    int* currDirection; //1->right 0->left
    int* velocity;
    BOOL stop;
    HANDLE hMutex;
    Game* game;
}DadosLanesThread;

DLL_IMP_API typedef struct {
    TCHAR command[100];
}BufferCell;


//representa a nossa memoria partilhada
DLL_IMP_API typedef struct {
    int posE; //proxima posicao de escrita
    int posL; //proxima posicao de leitura
    BufferCell buffer[TAM_BUFFER]; //buffer circular em si (array de estruturas)
}SharedMem;


//estrutura de apoio
DLL_IMP_API typedef struct {
    SharedMem* memPar; //ponteiro para a memoria partilhada
    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
    HANDLE hMutex;
    HANDLE hMapFile;
    int terminar; // 1 para sair, 0 em caso contrário
    int id;
    DadosLanesThread* dadosLanes;
}ControlData;

DLL_IMP_API typedef struct {
    TCHAR msg[100];
    Game game[10];
}SharedStruct;

DLL_IMP_API typedef struct {
    SharedStruct* sharedStruct;
    HANDLE hEvent;
    HANDLE hMutex;
    HANDLE hFileMap;
    Game* game;
    int terminar;
}ThreadDadosGame;

DLL_IMP_API BOOL initMemAndSync(ControlData* dados);         //prepara mem.par p/comandos (Servidor/operador)

DLL_IMP_API DWORD WINAPI recieveCommands(LPVOID param);      //recebe comandos(Servidor)

DLL_IMP_API BOOL initMemAndSyncGame(ThreadDadosGame* dados); //prepara mem.par p/jogo (Servidor/operador)

DLL_IMP_API DWORD WINAPI sendCommands(LPVOID param);         //envia comandos(Operador) 

DLL_IMP_API int verifyCommand(TCHAR* command);

DLL_IMP_API TCHAR** splitString(TCHAR* str, const TCHAR* delim, int* size);

DLL_IMP_API BOOL isStringANumber(TCHAR* str);
//DLL_IMP_API double factor;

//DLL_IMP_API double applyFactor(double v);