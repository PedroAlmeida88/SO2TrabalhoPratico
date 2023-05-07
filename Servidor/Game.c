#include "Game.h"

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
			_tprintf(TEXT("%c"),game[i].estrada[j]);
		}
		_tprintf(TEXT("\n"));
	}
}

DWORD WINAPI lanesFunction(LPVOID param) {
    DadosLanesThread* dados = (DadosLanesThread*)param;
    //while (1) {
        //esperamos que o mutex esteja livre
        WaitForSingleObject(dados->hMutex, INFINITE);
        int rand = randNum(0, 19);
        _tprintf(TEXT("Random number %d\n"), rand);
        dados->game[dados->laneNumber].estrada[dados->laneNumber] = 'C';
        //system("cls");
        show(dados->game);
        
        _tprintf(TEXT("Thread/lane num %d\n\n\n"),dados->laneNumber);
       
        //libertamos o mutex
        ReleaseMutex(dados->hMutex);
        Sleep(1000);
    //}
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

void initRand() {
    srand(time(NULL));
}