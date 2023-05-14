#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
//#include "Utils.h"
#include "Game.h"
#include "Dll.h"

#define TAM_BUFFER 20
#define TAM 100

DWORD WINAPI ThreadLer(LPVOID param) {
    ThreadDadosGame* dados = (ThreadDadosGame*)param;
    while (1) {
        //esperar até que evento desbloqueie
        WaitForSingleObject(dados->hEvent, INFINITE);
        //verifica se é preciso terminar a thread ou nao
        if (dados->terminar)
            break;
        //faço o lock para o mutex
        WaitForSingleObject(dados->hMutex, INFINITE);
        //_tprintf(TEXT("Mensagem recebida: %s\n"), dados->sharedStruct->msg);
        system("cls");
        show(dados->sharedStruct->game);
        
        //faço unlock do mutex
        ReleaseMutex(dados->hMutex);
        Sleep(500);
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


    //temos de usar o mutex para aumentar o nProdutores para termos os ids corretos
    WaitForSingleObject(dados.hMutex, INFINITE);
    //dados.memPar->nProdutores++;
    //dados.id = dados.memPar->nProdutores;
    ReleaseMutex(dados.hMutex);


    //lancamos a thread
    hThread[0] = CreateThread(NULL, 0, sendCommands, &dados, 0, NULL);
    hThread[1] = CreateThread(NULL, 0, ThreadLer, &dadosGame, 0, NULL);
    if (hThread[0] != NULL && hThread[1] != NULL) {
        /*
        _tprintf(TEXT("Escreva qualquer coisa para sair ...\n"));
        _getts_s(comando, 100);
        dados.terminar = 1;
        */
        //esperar que a thread termine
        WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
    }

    UnmapViewOfFile(dados.memPar);
    //CloseHandles ... mas é feito automaticamente quando o processo termina

    return 0;
}