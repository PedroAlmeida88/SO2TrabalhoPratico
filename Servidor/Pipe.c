#include "Pipe.h"

DWORD WINAPI ThreadMensagens(LPVOID param)
{
	TCHAR msg_env[256], msg_rec[256];
	DWORD n;
	int i;
	BOOL ret;
	ThreadDadosPipe* dados = (ThreadDadosPipe*)param;

	do
	{
		/*
		_tprintf(TEXT("[ESCRITOR] Frase: "));
		_fgetts(msg_env, 256, stdin);
		msg_env[_tcslen(msg_env) - 1] = '\0';
		*/
		_tcscpy_s(msg_env, 256, TEXT("TestePipeEnvio"));
		for (i = 0; i < MAX_CLI; i++)
		{
			WaitForSingleObject(dados->hMutex, INFINITE);

			if (dados->hPipes[i].activo)
			{
				////ATÉ AQUI O GAME ESTÁ CORRETO!! (Chega certo até aqui)
				//if (!WriteFile(dados->hPipes[i].hInstancia, msg_env, _tcslen(msg_env) * sizeof(TCHAR), &n, NULL))
				if (!WriteFile(dados->hPipes[i].hInstancia, dados->game, sizeof(dados->game) * 10 * sizeof(TCHAR), &n, NULL))
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				else
				{
					//_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao leitor [%d]... (WriteFile)\n"), n, i);

					ret = ReadFile(dados->hPipes[i].hInstancia, msg_rec, sizeof(msg_rec) - sizeof(TCHAR), &n, NULL);
					msg_rec[n / sizeof(TCHAR)] = '\0';
					//_tprintf(TEXT("[ESCRITOR] Recebi %d bytes: '%s'... (ReadFile)\n"), n, msg_rec);
				}
			}

			ReleaseMutex(dados->hMutex);

		}
	//	Sleep(2000);

	} while (_tcscmp(msg_env, TEXT("fim")));

	dados->terminar = 1;

	for (i = 0; i < MAX_CLI; i++)
		SetEvent(dados->hEvents[i]);

	return 0;
}