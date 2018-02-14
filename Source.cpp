#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <comdef.h>  
#include <msinkaut.h>  
#include <msinkaut_i.c>

TCHAR szClassName[] = TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton1, hButton2;
	static IInkCollector * pIInkCollector = NULL;
	static IInkDisp* pIInkDisp = NULL;
	static IInkRecognizerContext* pIInkRecoContext = NULL;
	switch (msg)
	{
	case WM_CREATE:
		CoInitialize(NULL);
		if (FAILED(CoCreateInstance(CLSID_InkRecognizerContext,
			NULL, CLSCTX_INPROC_SERVER,
			IID_IInkRecognizerContext,
			(void **)&pIInkRecoContext)))
			return -1;
		if (FAILED(CoCreateInstance(CLSID_InkCollector,
			NULL, CLSCTX_INPROC_SERVER,
			IID_IInkCollector,
			(void **)&pIInkCollector)))
			return -1;
		if (FAILED(pIInkCollector->get_Ink(&pIInkDisp)))
			return -1;
		if (FAILED(pIInkCollector->put_hWnd((LONG_PTR)hWnd)))
			return -1;
		if (FAILED(pIInkCollector->put_Enabled(VARIANT_TRUE)))
			return -1;
		hButton1 = CreateWindow(TEXT("BUTTON"), TEXT("判定"), WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton2 = CreateWindow(TEXT("BUTTON"), TEXT("クリア"), WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)IDCANCEL, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hButton1, 10, 10, 256, 32, TRUE);
		MoveWindow(hButton2, 10, 50, 256, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			IInkStrokes* pIInkStrokes = NULL;
			if (SUCCEEDED(pIInkDisp->get_Strokes(&pIInkStrokes)))
			{
				if (SUCCEEDED(pIInkRecoContext->putref_Strokes(pIInkStrokes)))
				{
					IInkRecognitionResult* pIInkRecoResult = NULL;
					InkRecognitionStatus RecognitionStatus;
					if (SUCCEEDED(pIInkRecoContext->Recognize(&RecognitionStatus, &pIInkRecoResult)) && (pIInkRecoResult != NULL))
					{
						BSTR bstrBestResult = NULL;
						if (SUCCEEDED(pIInkRecoResult->get_TopString(&bstrBestResult)))
						{
							MessageBoxW(hWnd, bstrBestResult, L"判定結果", MB_OK);
							SysFreeString(bstrBestResult);
						}
						pIInkRecoResult->Release();
						pIInkRecoResult = NULL;
					}
					pIInkRecoContext->putref_Strokes(NULL);
				}
				pIInkStrokes->Release();
				pIInkStrokes = NULL;
			}
			SetFocus(hWnd);
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			pIInkDisp->DeleteStrokes(0);
			InvalidateRect(hWnd, NULL, TRUE);
			SetFocus(hWnd);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		if (pIInkRecoContext != NULL)
		{
			pIInkRecoContext->Release();
			pIInkRecoContext = NULL;
		}
		if (pIInkDisp != NULL)
		{
			pIInkDisp->Release();
			pIInkDisp = NULL;
		}
		if (pIInkCollector != NULL)
		{
			pIInkCollector->Release();
			pIInkCollector = NULL;
		}
		CoUninitialize();
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("手書き文字入力"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}