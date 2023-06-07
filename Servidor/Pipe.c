#include "Pipe.h"

DWORD WINAPI ThreadMensagens(LPVOID param)
{
	TCHAR msg_env[256], msg_rec[256];
	DWORD n;
	int i;
	BOOL ret;
	ThreadDadosPipe* dados = (ThreadDadosPipe*)param;
	Game game[10];
	do
	{
		_tcscpy_s(msg_env, 256, TEXT("TestePipeEnvio"));
		for (i = 0; i < MAX_CLI; i++)
		{
			WaitForSingleObject(dados->hMutex, INFINITE);

			if (dados->hPipes[i].activo)
			{
				if (!WriteFile(dados->hPipes[i].hInstancia, dados->game, sizeof(dados->game) * 10 * 2 * sizeof(TCHAR), &n, NULL))
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				else
				{
					ret = ReadFile(dados->hPipes[i].hInstancia, &game, sizeof(game), &n, NULL);
					
					WaitForSingleObject(dados->hMutexJogo, INFINITE);
					CopyMemory(dados->game, game, sizeof(game));//O jogo fica um pouco mais lento
					/*
					for (int i = 0; i < dados->game->total_lanes; i++)
					{
						for (int j = 0; j < TAM_LANE; j++)
						{
							dados->game[i].estrada[j] = game[i].estrada[j];

						}
					}
					*/
					system("cls");

					show(dados->game);
					ReleaseMutex(dados->hMutexJogo);
					
				}
			}

			ReleaseMutex(dados->hMutex);

		}
		//	Sleep(2000);

	} while (!dados->terminar);

	dados->terminar = 1;

	for (i = 0; i < MAX_CLI; i++)
		SetEvent(dados->hEvents[i]);

	return 0;
}