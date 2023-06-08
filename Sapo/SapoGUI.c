#include <windows.h>
#include <Windowsx.h>
#include <tchar.h>
#include "resource.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK TrataEventosLogin(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("Sapo");

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;

	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;

	wcApp.style = CS_HREDRAW | CS_VREDRAW;

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);

	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);

	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_PRINCIPAL);

	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,
		TEXT("Sapo Cliente"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(HWND)HWND_DESKTOP,
		(HMENU)NULL,
		(HINSTANCE)hInst,
		0);

	ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);

		DispatchMessage(&lpMsg);
	}

	return((int)lpMsg.wParam);
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	MINMAXINFO* mni;

	static HDC bmpDC = NULL;
	HBITMAP hBmp = NULL;
	static BITMAP bmp;
	static int xBitmap;
	static int yBitmap;

	int xPos, yPos;

	char c;
	static HDC memDC = NULL;

	switch (messg) {
	//Evento do Menu dos Resources
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CLARO:
			//Mostra um set de bitmaps
			MessageBox(hWnd, TEXT("OPÇÃO CLARO!"), TEXT("Bitmaps claros"), MB_OK | MB_ICONINFORMATION);
			break;
		case ID_ESCURO:
			//Mostra outro set de bitmaps
			MessageBox(hWnd, TEXT("OPÇÃO ESCURA!"), TEXT("Bitmaps escuros"), MB_OK | MB_ICONINFORMATION);
			break;
		}
	break;

	//Evento do click do botao direito do rato
	case WM_LBUTTONDOWN:
		//Se for ao lado do sapo, o sapo vai para o lugar do rato
		MessageBox(hWnd, TEXT("Botao Esquerdo"), TEXT("Sapo deslocou se para lá"), MB_OK | MB_ICONINFORMATION);
		break;

	//Evento do click do botao direito do rato
	case WM_RBUTTONDOWN:
		//Se for clicado em cima do sapo, o sapo volta ao ponto de partida
		MessageBox(hWnd, TEXT("Botao Direito"), TEXT("Sapo deslocou se para a base"), MB_OK | MB_ICONINFORMATION);
		break;

	//Evento ao rato passar por cima do sapo
	case WM_MOUSEHOVER:
		//Se for o sapo, irá mostrar as estatisticas do sapo
		break;

	case WM_CREATE:
		//Sitio certo para fazer inicializacoes;
		DialogBox(NULL, MAKEINTRESOURCE(ID_LOGIN), hWnd, TrataEventosLogin);
		
		/*hBmp = (HBITMAP)LoadImage(NULL, TEXT("cav.bmp"),
			IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		GetObject(hBmp, sizeof(bmp), &bmp);

		hdc = GetDc(hWnd);
		bmpDC = CreateCompatibleDC(hdc);
		SelectObject(bmpDC, hBmp);
		ReleaseDC(hWnd, hdc);

		GetClientRect(hWnd, &rect);
		xBitmap = (rect.right / 2) - (bmp.bmWidth / 2);
		yBitmap = (rect.bottom / 2) - (bmp.bmHeight / 2);*/
		break;

	case WM_PAINT:
		//Double Buffering
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);

		if (memDC == NULL) {
			memDC = CreateCompatibleDC(hdc);
			hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			SelectObject(memDC, hBmp);
			DeleteObject(hBmp);

			FillRect(memDC, &rect, CreateSolidBrush(RGB(0, 255, 0)));
			BitBlt(memDC, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
		}
		FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 255, 0)));
		EndPaint(hWnd, &ps);

		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_GETMINMAXINFO:
		mni = (MINMAXINFO*)lParam;
		mni->ptMinTrackSize.x = bmp.bmWidth + 2;
		mni->ptMinTrackSize.y = bmp.bmHeight + 2;
		break;

	case WM_SIZE:
		xBitmap = (LOWORD(lParam) / 2) - (bmp.bmWidth / 2);
		yBitmap = (HIWORD(lParam) / 2) - (bmp.bmHeight / 2);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_LEFT) 
		{
			//Sapo desloca-se para a esquerda
			MessageBox(hWnd, TEXT("Arrow Esquerda"), TEXT("Sapo deslocou-se para a esquerda"), MB_OK | MB_ICONINFORMATION);
		}
		if (wParam == VK_UP)
		{
			//Sapo desloca-se para cima
			MessageBox(hWnd, TEXT("Arrow Cima"), TEXT("Sapo deslocou-se para cima"), MB_OK | MB_ICONINFORMATION);
		}
		if (wParam == VK_DOWN)
		{
			//Sapo desloca-se para baixo
			MessageBox(hWnd, TEXT("Arrow Baixo"), TEXT("Sapo deslocou-se para baixo"), MB_OK | MB_ICONINFORMATION);
		}
		if (wParam == VK_RIGHT)
		{
			//Sapo desloca-se para a direita
			MessageBox(hWnd, TEXT("Arrow Direita"), TEXT("Sapo deslocou-se para a direita"), MB_OK | MB_ICONINFORMATION);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("Desejas sair?"), TEXT("Sair"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;

	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
	}
	return(0);
}

LRESULT CALLBACK TrataEventosLogin(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	TCHAR username[16], password[16];

	switch (messg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hWnd, IDC_EDIT_USERNAME, username, 16);
			GetDlgItemText(hWnd, IDC_EDIT_PASSWORD, password, 16);
			MessageBox(hWnd, username, TEXT("Username"), MB_OK | MB_ICONINFORMATION);
			MessageBox(hWnd, password, TEXT("Password"), MB_OK | MB_ICONINFORMATION);
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	}

	return FALSE;
}

