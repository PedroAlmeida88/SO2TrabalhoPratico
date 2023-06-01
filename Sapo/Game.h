#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define TAM_LANE 20

typedef struct {
    int total_lanes;
    TCHAR estrada[TAM_LANE];
}Game;

void show(Game* game);

#endif