#include "Game.h"

void show(Game* game) {
    for (int i = 0; i < game->total_lanes; i++)
    {

        for (int j = 0; j < TAM_LANE - 1; j++)
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

            
           // _tprintf(TEXT("%c"), game[i].estrada[j]);
        }
        _tprintf(TEXT("\n"));
    }
    for (int j = 0; j < TAM_LANE - 1; j++)
        _tprintf(TEXT("---"));
    _tprintf(TEXT("\n"));
}

