#include "utils.h"

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
/*

void executeCommand(TCHAR* command, ControlData* dados) {
	TCHAR** commandArray = NULL;
	const TCHAR delim[2] = TEXT(" ");
	unsigned int nrArguments = 0;
	//int cordX, cordY;

	commandArray = splitString(command, delim, &nrArguments);

	if (_tcscmp(commandArray[0], TEXT("stop")) == 0) {
		return;
	}
	if (_tcscmp(commandArray[0], TEXT("turn")) == 0) {

		if (_tcscmp(dados->dadosLanes[atoi(commandArray[1])].currDirection, TEXT("right")) == 0) {
			_tcscpy_s(dados->dadosLanes[atoi(commandArray[1])].currDirection, 64, TEXT("left"));
		}
		else {
			_tcscpy_s(dados->dadosLanes[atoi(commandArray[1])].currDirection, 64, TEXT("right"));
		}

		return;
	}
}*/