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
    DadosLanesThread* dadosLanes;
}ControlData;


void executeCommand(TCHAR* command, ControlData* dados);

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
        dados->memPar->posE = 0;
        dados->memPar->posL = 0;
    }
    dados->terminar = 0;

}

DWORD WINAPI recieveCommands(LPVOID param) {
    ControlData* dados = (ControlData*)param;
    BufferCell cel;
    int contador = 0;
    TCHAR comando[64];

    while (!dados->terminar) {
        //aqui entramos na logica da aula teorica

        //esperamos por uma posicao para lermos
        WaitForSingleObject(dados->hSemLeitura, INFINITE);

        //vamos copiar da proxima posicao de leitura do buffer circular para a nossa variavel cel
        CopyMemory(&cel, &dados->memPar->buffer[dados->memPar->posL], sizeof(BufferCell));
        dados->memPar->posL++; //incrementamos a posicao de leitura para o proximo consumidor ler na posicao seguinte
        _tcscpy_s(comando, 64, cel.command);
        //se apos o incremento a posicao de leitura chegar ao fim, tenho de voltar ao inicio
        if (dados->memPar->posL == TAM_BUFFER)
            dados->memPar->posL = 0;

        //libertamos o semaforo. temos de libertar uma posicao de escrita
        ReleaseSemaphore(dados->hSemEscrita, 1, NULL);

        contador++;
        //soma += cel.val;
        _tprintf(TEXT("C%d recebeu o comando %s.\n"), dados->id, cel.command);
        //executeCommand(comando, &dados);
        ////////////////////////////////////////////////////
        TCHAR* token1 = NULL;
        TCHAR* token2 = NULL;
        TCHAR* token3 = NULL;
        TCHAR* nextToken = NULL;
        TCHAR* space = NULL;
        int numTokens = 0;

        // Obter o primeiro token
        token1 = _tcstok_s(comando, TEXT(" "), &nextToken);
        if (token1 == NULL) {
            // Se não houver um primeiro token, ocorreu um erro
            _tprintf_s(TEXT("[ERRO] Não foi possível obter o primeiro token.\n"));
            return;
        }
        numTokens++;

        // Obter o segundo token
        token2 = _tcstok_s(NULL, TEXT(" "), &nextToken);
        if (token2 == NULL) {
            // Se não houver um segundo token, ocorreu um erro
            _tprintf_s(TEXT("[ERRO] Não foi possível obter o segundo token.\n"));
            return;
        }
        numTokens++;

        // Obter o terceiro token, se houver
        token3 = _tcstok_s(NULL, TEXT(" "), &nextToken);
        if (token3 != NULL) {
            numTokens++;
        }

        // Imprimir os tokens obtidos
        _tprintf_s(TEXT("Token 1: %s\n"), token1);
        _tprintf_s(TEXT("Token 2: %s\n"), token2);
        if (numTokens == 3) {
            _tprintf_s(TEXT("Token 3: %s\n"), token3);
        }

        // Executar a ação correspondente ao comando
        if (_tcscmp(token1, TEXT("stop")) == 0) {
            if (numTokens != 2) {
                _tprintf_s(TEXT("[ERRO] O comando 'stop' deve ter dois argumentos.\n"));
                return;
            }
            int lane = atoi(token2);
            if (lane > 0 && lane < TOTAL_LANES) {
                dados->dadosLanes[lane].stop = TRUE;
            }

        }
        else if (_tcscmp(token1, TEXT("put")) == 0) {
            if (numTokens != 3) {
                _tprintf_s(TEXT("[ERRO] O comando 'put' deve ter três argumentos.\n"));
                return;
            }
            int x = _tstoi(token2);
            int y = _tstoi(token3);
            y--;
            if (x > 0 && x < TOTAL_LANES - 1) {
                if ((y >= 0 && y < TAM_LANE - 1))
                    _tprintf(TEXT("Indice na estrada %d .\n"), y);

                dados->dadosLanes[2].game[x].estrada[y] = 'O';
            }

        }
        else if (_tcscmp(token1, TEXT("turn")) == 0) {
            if (numTokens != 2) {
                _tprintf_s(TEXT("[ERRO] O comando 'stop' deve ter dois argumentos.\n"));
                return;
            }
            int lane = atoi(token2);
            if (lane > 0 && lane < TOTAL_LANES) {
                if (dados->dadosLanes[lane].currDirection == 0) {
                    dados->dadosLanes[lane].currDirection = 1;
                }
                else {
                    dados->dadosLanes[lane].currDirection = 0;
                }
            }
        }
    }
    _tprintf(TEXT("Recevi %d comandos.\n"), contador);

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
    /*dados[0].memPar->nConsumidores++;
    dados[0].id = dados[0].memPar->nConsumidores;*/
    dados[0].dadosLanes = dadosLanes;
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
    for (int i = 1; i < TOTAL_LANES - 1; i++) {
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


    for (int i = 1; i < TOTAL_LANES - 1; i++) {
        WaitForSingleObject(hThread[i], INFINITE);
    }

    WaitForSingleObject(hThread[0], INFINITE);
    UnmapViewOfFile(dados[0].memPar);
    //CloseHandles ... mas é feito automaticamente quando o processo termina

    return 0;
}
/**/
void executeCommand(TCHAR* command, ControlData* dados) {
    TCHAR* token1 = NULL;
    TCHAR* token2 = NULL;
    TCHAR* token3 = NULL;
    TCHAR* nextToken = NULL;
    TCHAR* space = NULL;
    int numTokens = 0;

    // Obter o primeiro token
    token1 = _tcstok_s(command, TEXT(" "), &nextToken);
    if (token1 == NULL) {
        // Se não houver um primeiro token, ocorreu um erro
        _tprintf_s(TEXT("[ERRO] Não foi possível obter o primeiro token.\n"));
        return;
    }
    numTokens++;

    // Obter o segundo token
    token2 = _tcstok_s(NULL, TEXT(" "), &nextToken);
    if (token2 == NULL) {
        // Se não houver um segundo token, ocorreu um erro
        _tprintf_s(TEXT("[ERRO] Não foi possível obter o segundo token.\n"));
        return;
    }
    numTokens++;

    // Obter o terceiro token, se houver
    token3 = _tcstok_s(NULL, TEXT(" "), &nextToken);
    if (token3 != NULL) {
        numTokens++;
    }

    // Imprimir os tokens obtidos
    _tprintf_s(TEXT("Token 1: %s\n"), token1);
    _tprintf_s(TEXT("Token 2: %s\n"), token2);
    if (numTokens == 3) {
        _tprintf_s(TEXT("Token 3: %s\n"), token3);
    }

    // Executar a ação correspondente ao comando
    if (_tcscmp(token1, TEXT("stop")) == 0) {
        if (numTokens != 2) {
            _tprintf_s(TEXT("[ERRO] O comando 'stop' deve ter dois argumentos.\n"));
            return;
        }
        // Executar a ação correspondente
        _tprintf_s(TEXT("Executando ação correspondente ao comando 'stop'...\n"));
    }
    else if (_tcscmp(token1, TEXT("put")) == 0) {
        if (numTokens != 3) {
            _tprintf_s(TEXT("[ERRO] O comando 'put' deve ter três argumentos.\n"));
            return;
        }
        // Executar a ação correspondente
        _tprintf_s(TEXT("Executando ação correspondente ao comando 'put'...\n"));
    }
    else if (_tcscmp(token1, TEXT("turn")) == 0) {
        if (numTokens != 2) {
            _tprintf_s(TEXT("[ERRO] O comando 'stop' deve ter dois argumentos.\n"));
            return;
        }
        /*
        if (dados->dadosLanes[5].currDirection == 0) {
            dados->dadosLanes[5].currDirection == 1;
        }
        else {
            dados->dadosLanes[5].currDirection == 0;
        }
        */
        dados->dadosLanes[5].currDirection == 0;


        // Executar a ação correspondente
        /*
        if (_tcscmp(dados->dadosLanes[3].currDirection, TEXT("right")) == 0) {
            //_tcscpy_s(dados->dadosLanes[atoi(token2)].currDirection, 64, TEXT("left"));
        }
        /*else {
            _tcscpy_s(dados->dadosLanes[atoi(token2)].currDirection, 64, TEXT("right"));
        }
        */
        /*
} else{
    _tprintf_s(TEXT("[ERRO] Comando desconhecido: %s.\n"), token1);
}*/
/*
TCHAR* token1 = NULL;
TCHAR* token2 = NULL;
TCHAR* token3 = NULL;
TCHAR* nextToken = NULL;
TCHAR* space = NULL;
int numTokens = 0;

// Obter o primeiro token
token1 = _tcstok_s(command, TEXT(" "), &nextToken);
if (token1 == NULL) {
    // Se não houver um primeiro token, ocorreu um erro
    _tprintf_s(TEXT("[ERRO] Não foi possível obter o primeiro token.\n"));
    return;
}
numTokens++;

// Encontrar o espaço em branco que separa as palavras, se houver
space = _tcschr(nextToken, ' ');
if (space == NULL) {
    // Se não houver espaço em branco, assumimos que há apenas um token
    token2 = nextToken;
    numTokens++;
}
else {
    // Substituir o espaço em branco por um terminador de string nulo
    *space = TEXT('\0');
    // Obter o segundo token
    token2 = space + 1;
    numTokens++;
    // Encontrar o espaço em branco que separa as palavras, se houver
    space = _tcschr(nextToken, ' ');
    if (space == NULL) {
        // Se não houver espaço em branco, assumimos que há apenas dois tokens
        token3 = NULL;
        numTokens++;
    }
    else {
        // Substituir o espaço em branco por um terminador de string nulo
        *space = TEXT('\0');
        // Obter o terceiro token
        token3 = space + 1;
        numTokens++;
    }
}

// Imprimir os tokens obtidos
_tprintf_s(TEXT("Token 1: %s\n"), token1);
_tprintf_s(TEXT("Token 2: %s\n"), token2);
if (numTokens == 3) {
    if(token3 != NULL)
        _tprintf_s(TEXT("Token 3: %s\n"), token3);
}

// Executar a ação correspondente ao comando
if (_tcscmp(token1, TEXT("stop")) == 0) {
    if (numTokens != 2) {
        _tprintf_s(TEXT("[ERRO] O comando 'stop' deve ter dois argumentos.\n"));
        return;
    }
    // Executar a ação correspondente
    _tprintf_s(TEXT("Executando ação correspondente ao comando 'stop'...\n"));
}
else if (_tcscmp(token1, TEXT("put")) == 0) {
    if (numTokens != 3) {
        _tprintf_s(TEXT("[ERRO] O comando 'put' deve ter três argumentos.\n"));
        return;
    }
    // Executar a ação correspondente
    _tprintf_s(TEXT("Executando ação correspondente ao comando 'put'...\n"));
}
else {
    _tprintf_s(TEXT("[ERRO] Comando desconhecido: %s.\n"), token1);
}
*/
/*
TCHAR* token1 = NULL;
TCHAR* token2 = NULL;
TCHAR* nextToken = NULL;
TCHAR* space = NULL;

// Obter o primeiro token
token1 = _tcstok_s(command, TEXT(" "), &nextToken);
if (token1 == NULL) {
    // Se não houver um primeiro token, ocorreu um erro
    _tprintf_s(TEXT("[ERRO] Não foi possível obter o primeiro token.\n"));
    return 1;
}

// Encontrar o espaço em branco que separa as palavras
space = _tcschr(nextToken, ' ');
if (space == NULL) {
    // Se não houver espaço em branco, assumimos que há apenas uma palavra
    token2 = nextToken;
}
else {
    // Substituir o espaço em branco por um terminador de string nulo
    *space = TEXT('\0');
    // Obter o segundo token
    token2 = space + 1;
}

// Imprimir os tokens obtidos
_tprintf_s(TEXT("Token 1: %s\n"), token1);
_tprintf_s(TEXT("Token 2: %s\n"), token2);

if (_tcscmp(token1, TEXT("stop")) == 0) {
    return;
}
*/
/*
TCHAR** commandArray = NULL;
const TCHAR delim[2] = TEXT(" ");
unsigned int nrArguments = 0;
//int cordX, cordY;

commandArray = splitString(command, delim, &nrArguments);

if (commandArray == NULL) {
    return ;
}





/*

/*
if (_tcscmp(commandArray[0], TEXT("turn")) == 0) {

    if (_tcscmp(dados->dadosLanes[atoi(commandArray[1])].currDirection, TEXT("right")) == 0) {
        _tcscpy_s(dados->dadosLanes[atoi(commandArray[1])].currDirection, 64, TEXT("left"));
    }
    else {
        _tcscpy_s(dados->dadosLanes[atoi(commandArray[1])].currDirection, 64, TEXT("right"));
    }

    return;
}
*/
    }
}