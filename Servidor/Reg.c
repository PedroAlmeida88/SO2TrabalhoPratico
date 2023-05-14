#include "Reg.h"


void registryCheck(LPTSTR argv[], GameData* gameData) {
    TCHAR key_velocity[TAM_REG] = TEXT("StarterVelocity");
    TCHAR key_lanes[TAM_REG] = TEXT("LanesNumber");
    TCHAR MaximumValues_key_path[100] = TEXT("SOFTWARE\\SO2-TP\\GameData");
    HKEY reg_key; // handle para a chave principal
    DWORD key_res; // resultado do RegCreateKeyEx

    //Criar/abrir chave
    if (RegCreateKeyEx(
        HKEY_CURRENT_USER,
        MaximumValues_key_path,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL,
        &reg_key,
        &key_res
    ) != ERROR_SUCCESS) {
        _ftprintf(stderr, TEXT("[ERRO] Não foi possível criar a chave principal!\n"));
        return -1;
    }

    if (argv[1] == NULL && argv[2] == NULL && argv[3] == NULL) {
        gameData->starterVelocity = 5; //_ttoi(argv[1]);
        gameData->lanesNumber = 5; //_ttoi(argv[2]);
        if (gameData->starterVelocity > 100) {
            gameData->starterVelocity = 100;
        }
        else if (gameData->starterVelocity < 0) {
            gameData->starterVelocity = 0;
        }

        if (gameData->lanesNumber > 10) {
            gameData->lanesNumber = 10;
        }
        else if (gameData->lanesNumber < 5) {
            gameData->lanesNumber = 5;
        }
        //---------Armazenar info no registry----------
        if (key_res == REG_CREATED_NEW_KEY || key_res == REG_OPENED_EXISTING_KEY) {
            // Criar um par "nome-valor" para a velocidade inicial
            if (RegSetValueEx(reg_key, key_velocity, 0, REG_DWORD, (LPBYTE)&gameData->starterVelocity, sizeof(gameData->starterVelocity)) != ERROR_SUCCESS) {
                _ftprintf(stderr, TEXT("[ERRO] Não foi possível adicionar o atributo %s!\n"), key_velocity);
            }
            // Criar um par "nome-valor" para as faizas de rodagem
            if (RegSetValueEx(reg_key, key_lanes, 0, REG_DWORD, (LPBYTE)&gameData->lanesNumber, sizeof(gameData->lanesNumber)) != ERROR_SUCCESS) {
                _ftprintf(stderr, TEXT("[ERRO] Não foi possível adicionar o atributo %s!\n"), key_lanes);
            }
            if (key_res == REG_CREATED_NEW_KEY)
                _tprintf(TEXT("Chave criada com sucesso!\n"));
            if (key_res == REG_OPENED_EXISTING_KEY)
                _tprintf(TEXT("Chave atualizada com sucesso!\n"));
        }
    }
    else {//ler do registry 
        _tprintf(TEXT("Formato invalido!A ler informação do registry...\n"));
        DWORD value_size = sizeof(int);
        int a;
        if (RegQueryValueEx(reg_key, key_velocity, 0, NULL, (LPBYTE)&a, &value_size) != ERROR_SUCCESS) {
            _tprintf(TEXT("Atributo [StarterVelocity] não foi encontrado! ERRO!\n"));
        }
        else {
            _tprintf(TEXT("Atributo encontrado com nome [%s] tem valor %d\n"), key_velocity, a);
            gameData->starterVelocity = a;
        }

        if (RegQueryValueEx(reg_key, key_lanes, 0, NULL, (LPBYTE)&a, &value_size) != ERROR_SUCCESS) {
            _tprintf(TEXT("Atributo [LanesNumber] não foi encontrado! ERRO!\n"));
        }
        else {
            _tprintf(TEXT("Atributo encontrado com nome [%s] tem valor %d\n"), key_lanes, a);
            gameData->lanesNumber = a;
        }
    }

    RegCloseKey(reg_key);//fechar o handle da chave no fim do programa
}