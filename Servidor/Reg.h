#ifndef REG_H
#define REG_H

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#define TAM_REG 200

typedef struct {
    int starterVelocity;    //Velocidade inicial dos carros
    int lanesNumber;        //Numero de faixas de rodagem
}GameData;

void registryCheck(LPTSTR argv[], GameData* gameData);

#endif