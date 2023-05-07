#include "Game.h"

Game* createGame(int lanesNumber, int velocidade) {
    Game* array = (Game*)malloc(lanesNumber * sizeof(Game));
    if (array == NULL) {
        printf("Erro ao alocar memoria");
        return NULL;
    }
    return array;
}

void initGame(Game *game) {
    for (int i = 0; i < TAM_LANE; i++) {     //Inicializar a primeira e a última via
        game[0].estrada[i] = '_';
        game[TOTAL_LANES - 1].estrada[i] = '-';
    }
    game[0].velocity = 0;
    game[TOTAL_LANES - 1].velocity = 0;

    for (int i = 1; i < TOTAL_LANES - 1; i++) { //Inicializar as vias restantes
        for (int j = 0; j < TAM_LANE - 1; j++) {
            game[i].estrada[j] = ' ';
        }
        game[i].velocity = 5;
    }
}

void show(Game *game) {
	for (int i = 0; i < TOTAL_LANES; i++)
	{
		for (int j = 0; j < TAM_LANE - 1; j++)
		{
            if (game[i].estrada[j] == ' ')
                _tprintf(TEXT("   "));
            if (game[i].estrada[j] == '-')
                _tprintf(TEXT("---"));
            if (game[i].estrada[j] == '_')
                _tprintf(TEXT("___"));
            if (game[i].estrada[j] == 'C')
                _tprintf(TEXT("8=D"));

	        //_tprintf(TEXT("%c"),game[i].estrada[j]);
            
		}            
        _tprintf(TEXT("\n"));
	}
}

DWORD WINAPI lanesFunction(LPVOID param) {
    DadosLanesThread* dados = (DadosLanesThread*)param;
    //int seed = *(int*)param;
    int nCarros = 0;//num de carros na via
    srand((unsigned int)time(NULL) ^ GetCurrentThreadId());
    BOOL firstTime = TRUE;
    while (!dados->terminar) {
        int rand = randNum(0, 19);
        //_tprintf(TEXT("Random number %d\n"), rand);
        //esperamos que o mutex esteja livre
        WaitForSingleObject(dados->hMutex, INFINITE);   
        if (firstTime) {
            initCars(dados->game, dados->laneNumber,&nCarros);
        }else {
            moveCars(TEXT("right"), dados->game, dados->laneNumber, &nCarros);
        }

        system("cls");
        show(dados->game);
        
        _tprintf(TEXT("Thread/lane num %d\n\n\n"),dados->laneNumber);
       
        //libertamos o mutex
        ReleaseMutex(dados->hMutex);
        Sleep(1000);
        firstTime = FALSE;

    }
    /*
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
    */
}

int randNum(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void initCars(Game *game, int laneNumber,int* nCarros) {
    for (int i = 0; i < TAM_LANE; i++) {
        int flag = randNum(0, 4);//25% de chance de meter um carro numa pos
        if (flag == 0) {
            game[laneNumber].estrada[i] = 'C';
            nCarros++;
            if (*nCarros >= 8)
                break;
        }
    }
}

void moveCars(TCHAR *direction, Game* game, int laneNumber, int* nCarros) {
   
    if (_tcscmp(direction, TEXT("left")) == 0) {
        //direction: right
        for (int i = 0; i < TAM_LANE; i++) {
            if (game[laneNumber].estrada[i] == 'C') {
                if (i != 0) {
                    game[laneNumber].estrada[i] = ' ';
                    game[laneNumber].estrada[i - 1] = 'C';
                }
                else {
                    game[laneNumber].estrada[i] = ' ';
                }
            }

        }
        //25% de chance de spwanar um carr
        if (*nCarros < 8) {
            int flag = randNum(0, 4);
            if (flag == 0) {
                game[laneNumber].estrada[TAM_LANE - 1] = 'C';
            }
        }

    }else {
        //direction: left
        for (int i = TAM_LANE - 1; i >= 0; i--) {
            if (game[laneNumber].estrada[i] == 'C') {
                if (i != TAM_LANE - 1) {
                    game[laneNumber].estrada[i] = ' ';
                    game[laneNumber].estrada[i + 1] = 'C';
                }
                else {
                    game[laneNumber].estrada[i] = ' ';
                }
            }
        }
        // 25% de chance de spawnar um carro
        if (*nCarros < 8) {
            int flag = randNum(0, 4);
            if (flag == 0) {
                game[laneNumber].estrada[0] = 'C';
            }
        }
    }
}
