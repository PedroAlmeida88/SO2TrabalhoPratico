#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Reg.h"
#include "Game.h"
//#include "Dll.h"

#define TAM 200
#define TAM_BUFFER 10
#define NUM_THREADS 10

typedef struct {
    //int id;
    //int val;
    TCHAR command[100];
}BufferCell;

//representa a nossa memoria partilhada
typedef struct {
    int nProdutores;
    int nConsumidores;
    int posE; //proxima posicao de escrita
    int posL; //proxima posicao de leitura
    BufferCell buffer[TAM_BUFFER]; //buffer circular em si (array de estruturas)
}SharedMem;


//estrutura de apoio
typedef struct {
    SharedMem* memPar; //ponteiro para a memoria partilhada
    HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
    HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
    HANDLE hMutex;
    HANDLE hMapFile;
    int terminar; // 1 para sair, 0 em caso contrário
    int id;
}ControlData;

BOOL initMemAndSync(ControlData* dados) {
    BOOL  primeiroProcesso = FALSE;

    //criar semaforo que conta as escritas
    dados->hSemEscrita = CreateSemaphore(NULL, TAM_BUFFER, TAM_BUFFER, TEXT("SO2_SEMAFORO_ESCRITA"));

    //criar semaforo que conta as leituras
    //0 porque nao ha nada para ser lido e depois podemos ir até um maximo de 10 posicoes para serem lidas
    dados->hSemLeitura = CreateSemaphore(NULL, 0, TAM_BUFFER, TEXT("SO2_SEMAFORO_LEITURA"));

    //criar mutex para os produtores
    dados->hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_CONSUMIDOR"));

    if (dados->hSemEscrita == NULL || dados->hSemLeitura == NULL || dados->hMutex == NULL) {
        _tprintf(TEXT("Erro no CreateSemaphore ou no CreateMutex\n"));
        return -1;
    }

    //o openfilemapping vai abrir um filemapping com o nome que passamos no lpName
    //se devolver um HANDLE ja existe e nao fazemos a inicializacao
    //se devolver NULL nao existe e vamos fazer a inicializacao

    dados->hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"));
    if (dados->hMapFile == NULL) {
        primeiroProcesso = TRUE;
        //criamos o bloco de memoria partilhada
        dados->hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(SharedMem), //tamanho da memoria partilhada
            TEXT("SO2_MEM_PARTILHADA"));//nome do filemapping. nome que vai ser usado para partilha entre processos

        if (dados->hMapFile == NULL) {
            _tprintf(TEXT("Erro no CreateFileMapping\n"));
            return -1;
        }
    }

    //mapeamos o bloco de memoria para o espaco de enderaçamento do nosso processo
    dados->memPar = (SharedMem*)MapViewOfFile(dados->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);


    if (dados->memPar == NULL) {
        _tprintf(TEXT("Erro no MapViewOfFile\n"));
        return -1;
    }

    if (primeiroProcesso == TRUE) {
        dados->memPar->nConsumidores = 0;
        dados->memPar->nProdutores = 0;
        dados->memPar->posE = 0;
        dados->memPar->posL = 0;
    }
    dados->terminar = 0;

}

DWORD WINAPI recieveCommands(LPVOID param) {
    ControlData* dados = (ControlData*)param;
    BufferCell cel;
    int contador = 0;
    //int soma = 0;

    while (!dados->terminar) {
        //aqui entramos na logica da aula teorica

        //esperamos por uma posicao para lermos
        WaitForSingleObject(dados->hSemLeitura, INFINITE);

        //esperamos que o mutex esteja livre
        //WaitForSingleObject(dados->hMutex, INFINITE);


        //vamos copiar da proxima posicao de leitura do buffer circular para a nossa variavel cel
        CopyMemory(&cel, &dados->memPar->buffer[dados->memPar->posL], sizeof(BufferCell));
        dados->memPar->posL++; //incrementamos a posicao de leitura para o proximo consumidor ler na posicao seguinte

        //se apos o incremento a posicao de leitura chegar ao fim, tenho de voltar ao inicio
        if (dados->memPar->posL == TAM_BUFFER)
            dados->memPar->posL = 0;

        //libertamos o mutex
        //ReleaseMutex(dados->hMutex);

        //libertamos o semaforo. temos de libertar uma posicao de escrita
        ReleaseSemaphore(dados->hSemEscrita, 1, NULL);

        contador++;
        //soma += cel.val;
        _tprintf(TEXT("C%d recebeu o comando %s.\n"), dados->id, cel.command);
    }
    _tprintf(TEXT("C%d recebeu %d comandos.\n"), dados->id, contador);

    return 0;
}


int _tmain(int argc, LPTSTR argv[]) {
    GameData gameData;

    HANDLE hFileMap; //handle para o file map
    HANDLE hThread[NUM_THREADS], hSemaphoreUnique;
    ControlData dados[NUM_THREADS];
    TCHAR comando[100];

   // Game dadosLanes;
    Game game[TOTAL_LANES];     //mapa do jogo
    DadosLanesThread dadosLanes[TOTAL_LANES - 2];//Dados enviados para a thread

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif
    initRand();
    //Semaforo para cotrolar o numero de servidores ativos
    hSemaphoreUnique = CreateSemaphore(
        NULL,
        1,
        2,
        TEXT("CONTROL")
    );

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        _ftprintf(stderr, TEXT("[ERRO] Já existe um servidor aberto!\n"));
        return -1;
    }

    registryCheck(argv, &gameData);
    _tprintf_s(TEXT("\nVelocidade %d | Numero de faixas de rodagem %d\n"), gameData.starterVelocity, gameData.lanesNumber);

    initMemAndSync(&dados);


    //temos de usar o mutex para aumentar o nConsumidores para termos os ids corretos
    WaitForSingleObject(dados[0].hMutex, INFINITE);
    dados[0].memPar->nConsumidores++;
    dados[0].id = dados[0].memPar->nConsumidores;
    ReleaseMutex(dados[0].hMutex);

    //lancamos a thread
    hThread[0] = CreateThread(NULL, 0, recieveCommands, &dados, 0, NULL);
    if (hThread[0] == NULL) {
        /*
        _tprintf(TEXT("Escreva qualquer coisa para sair ...\n"));
        _getts_s(comando, 100);
        dados[0].terminar = 1;

        //esperar que a thread termine
        WaitForSingleObject(hThread[0], INFINITE);
        */
    }

    initGame(&game);
    HANDLE mutex[TOTAL_LANES - 2];
    //inicializar os dados das threads e criar uma thread para cada via
    for (int i = 1; i < TOTAL_LANES - 1; i++){
        dadosLanes[i].terminar = 0;
        dadosLanes[i].game = &game;
        mutex[i] = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_LANES"));
        dadosLanes[i].hMutex = mutex[i];
        dadosLanes[i].laneNumber = i;
        hThread[i] = CreateThread(NULL, 0, lanesFunction, &dadosLanes[i], 0, NULL);
    }

    WaitForSingleObject(hThread[0], INFINITE);
    for (int i = 1; i < TOTAL_LANES - 1; i++) {
        WaitForSingleObject(hThread[i], INFINITE);
    }
    UnmapViewOfFile(dados[0].memPar);
    //CloseHandles ... mas é feito automaticamente quando o processo termina

    return 0;
}