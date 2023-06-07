#include "Utils.h"

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

int VerifyCommandConsole(TCHAR* comando) {
    TCHAR** commandArray = NULL;
    const TCHAR delim[2] = TEXT(" ");
    unsigned int nrArguments = 0;
    int cordX, cordY;
    TCHAR commandAux[60];
    _tcscpy_s(commandAux, 60, comando);
    commandArray = splitString(commandAux, delim, &nrArguments);

    // Se a funcao splitString retornar NULL é porque o comando estava vazio
    if (commandArray == NULL) {
        _tprintf(TEXT("[ERRO] Argumentos inválidos!\n"));
        return 0;
    }
    //stop
    if (_tcscmp(commandArray[0], TEXT("suspender")) == 0) {
        if (nrArguments != 1) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: suspender\n"));
            return 0;
        }
        _tprintf(TEXT("Commando conhecido!\n"));
        return 1;
    }
    //retomar
    if (_tcscmp(commandArray[0], TEXT("retomar\n")) == 0) {
        if (nrArguments != 1) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: retomar\n"));
            return 0;
        }
        _tprintf(TEXT("Commando conhecido!\n"));
        return 2;
    }
    //reiniciar
    if (_tcscmp(commandArray[0], TEXT("reiniciar")) == 0) {
        if (nrArguments != 1) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: reiniciar\n"));
            return 0;
        }
        _tprintf(TEXT("Commando conhecido!\n"));
        return 3;
    }
    //sair
    if (_tcscmp(commandArray[0], TEXT("sair\n")) == 0) {
        if (nrArguments != 1) {
            _tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: sair\n"));
            return 0;
        }
        _tprintf(TEXT("Commando conhecido!\n"));
        return 4;
    }
    return 0;
}
