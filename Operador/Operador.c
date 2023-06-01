#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Game.h"
#include "Dll.h"

#define TAM_BUFFER 20
#define TAM 100

DWORD WINAPI ThreadLer(LPVOID param) {
    ThreadDadosGame* dados = (ThreadDadosGame*)param;
    while (1) {
        
        WaitForSingleObject(dados->hEvent, INFINITE);
        
        if (dados->terminar)
            break;
        
        WaitForSingleObject(dados->hMutex, INFINITE);
        
        system("cls");
        show(dados->sharedStruct->game);
        
        
        ReleaseMutex(dados->hMutex);
        Sleep(250);
    }
    return 0;
}

int _tmain(int argc, LPTSTR argv[]) {
    HANDLE hFileMap, hFileMapControl; //handle para o file map
    HANDLE hThread[2];
    ControlData dados;
    TCHAR comando[100];;
    ThreadDadosGame dadosGame;


#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    hFileMapControl = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "SO2_MEM_PART");
    if (hFileMapControl == NULL) {
        _tprintf(TEXT("[Erro] O Servidor ainda não foi iniciado!\n"));
        
        return 1;
    }
    srand((unsigned int)time(NULL));
    initMemAndSync(&dados);
    initMemAndSyncGame(&dadosGame);
 
    WaitForSingleObject(dados.hMutex, INFINITE);
    
    ReleaseMutex(dados.hMutex);

    hThread[0] = CreateThread(NULL, 0, sendCommands, &dados, 0, NULL);
    hThread[1] = CreateThread(NULL, 0, ThreadLer, &dadosGame, 0, NULL);
    if (hThread[0] != NULL && hThread[1] != NULL) {
        
        WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
    }

    UnmapViewOfFile(dados.memPar);
   

    return 0;
}