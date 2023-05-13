#include "Dll.h"


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
    int total_lanes = dados->dadosLanes[2].game->total_lanes;
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
            if (lane > 0 && lane < total_lanes) {
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
            if (x > 0 && x < total_lanes - 1) {
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
            if (lane > 0 && lane < total_lanes) {
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

BOOL initMemAndSyncGame(ThreadDadosGame* dados) {
    dados->hFileMap = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(SharedStruct), // alterar o tamanho do filemapping
        "SO2_MEM_PART"); //nome do file mapping, tem de ser único
    if (dados->hFileMap == NULL) {
        _tprintf(TEXT("Erro no OpenFileMapping\n"));
        //CloseHandle(hFile); //recebe um handle e fecha esse handle , no entanto o handle é limpo sempre que o processo termina
        return 1;
    }
    //mapeia bloco de memoria para espaço de endereçamento
    dados->sharedStruct = (SharedStruct*)MapViewOfFile(dados->hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedStruct));

    if (dados->sharedStruct == NULL) {
        _tprintf(TEXT("Erro no MapViewOfFile\n"));
        return 1;
    }

    dados->hEvent = CreateEvent(
        NULL,
        TRUE,
        FALSE,
        TEXT("SO2_EVENTO"));
    if (dados->hEvent == NULL) {
        _tprintf(TEXT("Erro no CreateEvent\n"));
        UnmapViewOfFile(dados->sharedStruct);
        return 1;
    }
    dados->hMutex = CreateMutex(
        NULL,
        FALSE,
        TEXT("SO2_MUTEX"));
    if (dados->hMutex == NULL) {
        _tprintf(TEXT("Erro no CreateMutex\n"));
        UnmapViewOfFile(dados->sharedStruct);
        return 1;
    }

    dados->terminar = 0;

}

DWORD WINAPI sendCommands(LPVOID param) {
    ControlData* dados = (ControlData*)param;
    BufferCell cel;
    int contador = 0;

    while (!dados->terminar) {
        //ler uma frase
        do {
            _tprintf(TEXT("[OPERADOR] Commando: "));
            _fgetts(cel.command, TAM_BUFFER, stdin);
            cel.command[_tcslen(cel.command) - 1] = '\0';
            _tprintf(TEXT("escreveu %s\n"), cel.command);
            if (verifyCommand(cel.command) == 1)
                break;
        } while (1);

        _tprintf(TEXT("escreveu %s\n"), cel.command);

        //esperamos por uma posicao para escrevermos
        WaitForSingleObject(dados->hSemEscrita, INFINITE);

        //esperamos que o mutex esteja livre
        WaitForSingleObject(dados->hMutex, INFINITE);

        //vamos copiar a variavel cel para a memoria partilhada (para a posição de escrita)
        CopyMemory(&dados->memPar->buffer[dados->memPar->posE], &cel, sizeof(BufferCell));
        dados->memPar->posE++; //incrementamos a posicao de escrita para o proximo produtor escrever na posicao seguinte

        //se apos o incremento a posicao de escrita chegar ao fim, tenho de voltar ao inicio
        if (dados->memPar->posE == TAM_BUFFER)
            dados->memPar->posE = 0;

        //libertamos o mutex
        ReleaseMutex(dados->hMutex);

        //libertamos o semaforo. temos de libertar uma posicao de leitura
        ReleaseSemaphore(dados->hSemLeitura, 1, NULL);

        contador++;
        _tprintf(TEXT("Enviei o comando %s.\n"), cel.command);
        //Sleep(num_aleatorio(2, 4) * 1000);
    }
    _tprintf(TEXT("Enviei %d comandos.\n"), contador);

    return 0;
}

int verifyCommand(TCHAR* command) {
    TCHAR** commandArray = NULL;
    const TCHAR delim[2] = TEXT(" ");
    unsigned int nrArguments = 0;
    int cordX, cordY;
    TCHAR commandAux[60];
    _tcscpy_s(commandAux, 60, command);
    commandArray = splitString(commandAux, delim, &nrArguments);

    // Se a funcao splitString retornar NULL é porque o comando estava vazio
    if (commandArray == NULL) {
        _tprintf(TEXT("[ERRO] Argumentos inválidos!\n"));
        return 0;
    }
    //stop
    if (_tcscmp(commandArray[0], TEXT("stop")) == 0) {
        if (nrArguments != 2) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: stop <time>\n"));
            return 0;
        }
        //garante que é um numero
        if (!isStringANumber(commandArray[1])) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: stop <time>\n"));
            return 0;
        }
        _tprintf(TEXT("Commando conhecido!\n"));
        return 1;
    }
    //put 
    if (_tcscmp(commandArray[0], TEXT("put")) == 0) {
        if (nrArguments != 3) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: put <laneNumber> <posInLane>\n"));
            return 0;
        }
        //garante que é um numero
        if (!isStringANumber(commandArray[1]) || !isStringANumber(commandArray[2])) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: put <laneNumber> <posInLane>\n"));
            return 0;
        }
        _tprintf(TEXT("Commando conhecido!\n"));
        return 1;
    }
    //reverse
    if (_tcscmp(commandArray[0], TEXT("turn")) == 0) {
        if (nrArguments != 2) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: stop <time>\n"));
            return 0;
        }
        //garante que é um numero
        if (!isStringANumber(commandArray[1])) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: stop <time>\n"));
            return 0;
        }
        _tprintf(TEXT("Commando conhecido!\n"));
        return 1;
    }

    return 0;
}

//Função que divide uma string e retorna um array de strings
TCHAR** splitString(TCHAR* str, const TCHAR* delim, int* size) {
    TCHAR* nextToken = NULL, ** temp, ** returnArray = NULL;
    TCHAR* token = _tcstok_s(str, delim, &nextToken);

    if (str == NULL || _tcslen(str) == 0) {
        _tprintf(TEXT("[ERRO] String vazia!"));
        return NULL;
    }

    *size = 0;

    while (token != NULL) {
        temp = realloc(returnArray, sizeof(TCHAR*) * (*size + 1));

        if (temp == NULL) {
            _tprintf(TEXT("[ERRO] Impossível alocar memória para string!"));
            return NULL;
        }

        returnArray = temp;
        returnArray[(*size)++] = token;

        token = _tcstok_s(NULL, delim, &nextToken);
    }

    return returnArray;
}

BOOL isStringANumber(TCHAR* str) {
    int i;

    for (i = 0; i < _tcslen(str); i++) {
        if (!_istdigit(str[i])) return FALSE;
    }

    return TRUE;
}