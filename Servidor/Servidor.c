#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Reg.h"
#include "Game.h"
#include "Utils.h"
#include "Dll.h"
#include "Pipe.h"

#define TAM 200
#define MAX_LANES 10
#define TAM_BUFFER 10
#define NUM_THREADS 10

typedef struct {
    DadosLanesThread* dadosLane;
    ControlData* dados;
    Game* game;
    int continuar;
}LerComandos;
//Thread para ler os comandos do servidor
DWORD WINAPI ThreadLerComandos(LPVOID param)
{
    TCHAR comando[TAM_BUFFER];
    LerComandos* dadosComandos = (LerComandos*)param;

    while (dadosComandos->continuar) {
        Sleep(100);
        _tprintf(TEXT("[SERVIDOR] Commando: "));
        _fgetts(comando, TAM_BUFFER, stdin);
        comando[_tcslen(comando)] = '\0';
        _tprintf(TEXT("escreveu %s\n"), comando);

        int option = VerifyCommandConsole(comando);
        if (option == 1) {
            //suspender
            _tprintf(comando);
            for (int i = 1; i < dadosComandos->game->total_lanes - 1; i++) {
                dadosComandos->dadosLane[i].suspende = TRUE;
            }
        }
        if (option == 2) {
            //retomar
            _tprintf(comando);
            for (int i = 1; i < dadosComandos->game->total_lanes - 1; i++) {
                dadosComandos->dadosLane[i].suspende = FALSE;
            }
        }
        if (option == 3) {
            //reiniciar
            /*_tprintf(comando);
            initGame(&dadosComandos->game);*/
            //initCars(&game, gameData.lanesNumber, 0);
        }
        if (option == 4) {
            //sair
            _tprintf(comando);
            for (int i = 1; i < dadosComandos->game->total_lanes - 1; i++) {
                dadosComandos->dadosLane[i].terminar = 1;
            }
            for (int i = 0; i < NUM_THREADS; i++)
                dadosComandos->dados[i].terminar = 1;
            dadosComandos->continuar = 0;
        }
    }
}

//Thread para mandar o jogo para o opreador (mem-partilhada)
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

    HANDLE hFileMap;
    HANDLE hThread[NUM_THREADS], hSemaphoreUnique,hTheadGameShare,hThreadLer, hThreadLerComandos;
    ControlData dados[NUM_THREADS];
    TCHAR comando[100];
    ThreadDadosGame dadosGame;
    Game game[MAX_LANES];    
    DadosLanesThread dadosLanes[MAX_LANES - 2]; 
    HANDLE mutex[MAX_LANES - 2];
    LerComandos lerComandos;

    /////////PIPE////////////
    ThreadDadosPipe dadosPipe;
    HANDLE hPipe, hThreadPipe, hEventTemp;
    int i, numClientes = 0;
    DWORD offset, nBytes;

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


    //comandos recebidos do operador
    hThread[0] = CreateThread(NULL, 0, recieveCommands, &dados, 0, NULL);
    if (hThread[0] == NULL) {
        return;
    }
    
    //enviar o jogo para o operador
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
        dadosLanes[i].suspende = FALSE;
        hThread[i] = CreateThread(NULL, 0, lanesFunction, &dadosLanes[i], 0, NULL);
    }

    lerComandos.continuar = 1;
    lerComandos.dados = &dados;
    lerComandos.dadosLane = &dadosLanes;
    lerComandos.game = &game;
    hThreadLerComandos = CreateThread(NULL, 0, ThreadLerComandos, &lerComandos, 0, NULL);
    if (hThreadLerComandos == NULL) {
        return;
    }

    //-----------------------PIPE-----------------------//
    dadosPipe.terminar = 0;
    dadosPipe.hMutex = CreateMutex(NULL, FALSE, NULL);

    if (dadosPipe.hMutex == NULL)
    {
        _tprintf(TEXT("[ERRO] Criar Mutex! (CreateMutex)"));
        exit(-1);
    }

    for (i = 0; i < MAX_CLI; i++)
    {
        _tprintf(TEXT("[ESCRITOR] Criar uma copia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_NAME);

        hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (hEventTemp == NULL)
        {
            _tprintf(TEXT("[ERRO] Criar Evento! (CreateEvent)"));
            exit(-1);
        }

        hPipe = CreateNamedPipe(PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            MAX_CLI, sizeof(game) * sizeof(TCHAR), sizeof(game) * sizeof(TCHAR), 1000, NULL);

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            _tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
            exit(-1);
        }

        ZeroMemory(&dadosPipe.hPipes[i].overlap, sizeof(dadosPipe.hPipes[i].overlap));
        dadosPipe.hPipes[i].hInstancia = hPipe;
        dadosPipe.hPipes[i].overlap.hEvent = hEventTemp;
        dadosPipe.hPipes[i].activo = FALSE;
        dadosPipe.hEvents[i] = hEventTemp;

        if (ConnectNamedPipe(hPipe, &dadosPipe.hPipes[i].overlap))
        {
            _tprintf(TEXT("[ERRO] Ligacao ao leitor! (ConnectNamedPipe)\n"));
            exit(-1);
        }
    }

    //dadosPipe.hMutexJogo;
    dadosPipe.game = &game;
    show(dadosPipe.game);
    hThreadPipe = CreateThread(NULL, 0, ThreadMensagens, &dadosPipe, 0, NULL);
    show(dadosPipe.game);

    if (hThreadPipe == NULL)
    {
        _tprintf(TEXT("[ERRO] Criar Thread! (CreateThread)"));
        exit(-1);
    }

    while (!dadosPipe.terminar && numClientes < MAX_CLI)
    {
        _tprintf(TEXT("[ESCRITOR] Esperar ligacao de um leitor...\n"));

        offset = WaitForMultipleObjects(MAX_CLI, dadosPipe.hEvents, FALSE, INFINITE);
        i = offset - WAIT_OBJECT_0;

        if (i >= 0 && i < MAX_CLI)
        {
            _tprintf(TEXT("[ESCRITOR] Leitor %d chegou...\n"), i);

            if (GetOverlappedResult(dadosPipe.hPipes[i].hInstancia,
                &dadosPipe.hPipes[i].overlap, &nBytes, FALSE))
            {
                ResetEvent(dadosPipe.hEvents[i]);

                WaitForSingleObject(dadosPipe.hMutex, INFINITE);
                dadosPipe.hPipes[i].activo = TRUE;
                ReleaseMutex(dadosPipe.hMutex);
            }

            numClientes++;
        }
    }

    WaitForSingleObject(hThreadPipe, INFINITE);

    for (i = 0; i < MAX_CLI; i++)
    {
        _tprintf(TEXT("[ESCRITOR] Desligar o pipe (DisconnectNamedPipe)\n"));

        if (!DisconnectNamedPipe(dadosPipe.hPipes[i].hInstancia))
        {
            _tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
            exit(-1);
        }

        CloseHandle(dadosPipe.hPipes[i].hInstancia);
    }


    for (int i = 1; i < game->total_lanes - 1; i++) {
        WaitForSingleObject(hThread[i], INFINITE);
        UnmapViewOfFile(dadosGame.sharedStruct);
    }
    WaitForSingleObject(hThread[0], INFINITE);
    WaitForSingleObject(hThreadLerComandos, INFINITE);
    UnmapViewOfFile(dados[0].memPar);

    return 0;
}