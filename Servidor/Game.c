#include "Game.h"

Game* createGame(int lanesNumber) {
    Game* array = (Game*)malloc(lanesNumber * sizeof(Game));
    if (array == NULL) {
        printf("Erro ao alocar memoria");
        return NULL;
    }
    return array;
}

void initGame(Game* game) {
    for (int i = 0; i < TAM_LANE; i++) {     //Inicializar a primeira e a última via
        game[0].estrada[i] = '_';
        game[game->total_lanes - 1].estrada[i] = '-';
    }
    //game[0].velocity = 0;
    //game[TOTAL_LANES - 1].velocity = 0;

    for (int i = 1; i < game->total_lanes - 1; i++) { //Inicializar as vias restantes
        for (int j = 0; j < TAM_LANE - 1; j++) {
            game[i].estrada[j] = ' ';
        }
        //game[i].velocity = 5;
    }
    int posrand1;
    int posrand2;
    do {
        posrand1 = randNum(1, TAM_LANE - 2);
        posrand2 = randNum(1, TAM_LANE - 2);
    } while (posrand1 == posrand2);
    game[game->total_lanes - 1].estrada[posrand1] = 'S';
    game[game->total_lanes - 1].estrada[posrand2] = 'S';
}

void show(Game* game) {
    for (int i = 0; i < game->total_lanes; i++)
    {   

        for (int j = 0; j < TAM_LANE -1; j++)
            _tprintf(TEXT("---"));
        _tprintf(TEXT("\n"));

        for (int j = 0; j < TAM_LANE - 1; j++)
        {
            if (game[i].estrada[j] == ' ')
                _tprintf(TEXT("   "));
            if (game[i].estrada[j] == '-')
                _tprintf(TEXT("   "));
            if (game[i].estrada[j] == '_')
                _tprintf(TEXT("   "));
            if (game[i].estrada[j] == 'C')
                _tprintf(TEXT("8=D"));
            if (game[i].estrada[j] == 'O')
                _tprintf(TEXT(" O "));
            if (game[i].estrada[j] == 'S')
                _tprintf(TEXT(" S "));
            //_tprintf(TEXT("%c"),game[i].estrada[j]);
        }
        _tprintf(TEXT("\n"));
    }
    for (int j = 0; j < TAM_LANE - 1; j++)
        _tprintf(TEXT("---"));
    _tprintf(TEXT("\n"));
}

void copyGame(Game* original, int size, Game* new) {
    TCHAR road[TAM_LANE];
    for (int i = 0; i < size; i++) {
        new[i].total_lanes = original[i].total_lanes;
        _tcscpy_s(new[i].estrada, TAM_LANE, original[i].estrada);
        //_tcscpy_s(rad, TAM_LANE, original[i].estrada);
    }
}

DWORD WINAPI lanesFunction(LPVOID param) {
    DadosLanesThread* dados = (DadosLanesThread*)param;
    //int seed = *(int*)param;
    int nCarros = 0;//num de carros na via
    int starterVelocity = dados->velocity;
    int velocity;
    srand((unsigned int)time(NULL) ^ GetCurrentThreadId());
    BOOL firstTime = TRUE;
    int count = 0;
    while (!dados->terminar) {
        //int rand = randNum(0, 19);
        //_tprintf(TEXT("Random number %d\n"), rand);
        //esperamos que o mutex esteja livre
        WaitForSingleObject(dados->hMutex, INFINITE);
        if (firstTime) {
            initCars(dados->game, dados->laneNumber, &nCarros);
            velocity = starterVelocity;
        }
        else {
            if (!dados->stop) {
                moveCars(dados->currDirection, dados->game, dados->laneNumber, &nCarros);
                velocity = dados->velocity;
            }
            else {
                /*
                LARGE_INTEGER dueTime;
                dueTime.QuadPart = -50000000LL; //5 segundos (5000 milissegundos)
                HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
                SetWaitableTimer(timer, &dueTime, 0, NULL, NULL, 0);
                WaitForSingleObject(timer, INFINITE);
                dados->stop = FALSE;
                CloseHandle(timer);
                */
                count++;
                if (count >= 4) {
                    dados->stop = FALSE;
                    count = 0;
                }
            }
        }

        system("cls");
        show(dados->game);

        _tprintf(TEXT("Thread/lane num %d\n\n\n"), dados->laneNumber);

        ReleaseMutex(dados->hMutex);
        Sleep(1000 - (10 * velocity));
        firstTime = FALSE;

    }
   
}

int randNum(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void initCars(Game* game, int laneNumber, int* nCarros) {
    for (int i = 0; i < TAM_LANE; i++) {
        int flag = randNum(0, 3);//33% de chance de meter um carro numa pos
        if (flag == 0) {
            game[laneNumber].estrada[i] = 'C';
            nCarros++;
            if (*nCarros >= 8)
                break;
        }
    }
}

void moveCars(int direction, Game* game, int laneNumber, int* nCarros) {

    if (direction == 0) {
        //direction: right
        for (int i = 0; i < TAM_LANE; i++) {
            if (game[laneNumber].estrada[i] == 'C') {
                if (i != 0) {
                    if (game[laneNumber].estrada[i - 1] != 'O') {
                        game[laneNumber].estrada[i] = ' ';
                        game[laneNumber].estrada[i - 1] = 'C';
                    }
                    else {
                        game[laneNumber].estrada[i] = ' ';
                    }
                }
                else {
                    game[laneNumber].estrada[i] = ' ';
                }
            }

        }
        //25% de chance de spwanar um carr
        if (*nCarros < 8) {
            int flag = randNum(0, 4);
            if (flag == 0 && game[laneNumber].estrada[TAM_LANE - 1] != 'O') {
                game[laneNumber].estrada[TAM_LANE - 1] = 'C';
            }
        }

    }
    else {
        //direction: left
        for (int i = TAM_LANE - 1; i >= 0; i--) {
            if (game[laneNumber].estrada[i] == 'C') {
                if (i != TAM_LANE - 1) {
                    if (game[laneNumber].estrada[i + 1] != 'O') {
                        game[laneNumber].estrada[i] = ' ';
                        game[laneNumber].estrada[i + 1] = 'C';
                    }
                    else {
                        game[laneNumber].estrada[i] = ' ';
                    }
                }
                else {
                    game[laneNumber].estrada[i] = ' ';
                }
            }
        }
        // 25% de chance de spawnar um carro
        if (*nCarros < 8) {
            int flag = randNum(0, 4);
            if (flag == 0 && game[laneNumber].estrada[0] != 'O') {
                game[laneNumber].estrada[0] = 'C';
            }
        }
    }
}