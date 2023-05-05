#include "utils.h"


//return 0 -> unknown command
//return 1 -> known command
//Commands: stop <time>
//			put <x> <y>
//			turn <road number>
int verifyCommand(TCHAR* command) {
	TCHAR** commandArray = NULL;
	const TCHAR delim[2] = TEXT(" ");
	unsigned int nrArguments = 0;
	int cordX, cordY;

	commandArray = splitString(command, delim, &nrArguments);

	// Se a funcao splitString retornar NULL é porque o comando estava vazio
	if (commandArray == NULL) {
		_tprintf(TEXT("[ERRO] Argumentos inválidos!\n"));
		return 0;
	}
	//stop
	if (_tcscmp(commandArray[0], TEXT("stop")) == 0) {
		if (nrArguments != 2) {
			_tprintf( TEXT("[ERRO] Argumentos inválidos! Comando: stop <time>\n"));
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
			_tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: put <x> <y>\n"));
			return 0;
		}
		//garante que é um numero
		if (!isStringANumber(commandArray[1]) || !isStringANumber(commandArray[2])) {
			_tprintf(TEXT("[ERRO] Argumentos inválidos! Comando: put <x> <y>\n"));
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

/*
int verifyCommand(TCHAR *command) {
	TCHAR* token = NULL;
	TCHAR commandAux[TAM_STR];
	int n;
	int time;
	unsigned int count_found=0;

// Dividimos o comando
		commandArray = splitString(command, delim, &nrArguments);

		// Se a funcao splitString retornar NULL é porque o comando estava vazio
		if (commandArray == NULL) {
			EnterCriticalSection(mainInterface->criticalSectionBool);
			continue;
		}

	/*
	count_found = _stscanf_s(command, "%[^ ] %d", commandAux, sizeof(commandAux), &time);
	_tprintf(TEXT("2\n"));

	if (count_found == 2) {
		_tprintf(TEXT("Commando conhecido: \n"));
		return 1;
	}
	_tprintf(TEXT("Commando desconhecido: \n"));
	
	return 0;
}

*/