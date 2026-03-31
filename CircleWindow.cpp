#include <Windows.h>
#include "WM_USER_messages.h"
#include "CircleWindow.h"
#include "MHRepErr.h"
extern HINSTANCE	MHInst;
HWND CircleWindow::CircleHwnd=0;
extern HPEN green_pen;
extern bool G_eytracker_is_working;
//=================================================================================
// Îêîííàÿ ïðîöåäóðà
//=================================================================================
LRESULT CALLBACK CircleTranspWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
		case WM_CREATE:
			// Ñîäðàíî èç èíòåðíåòà - òàê ìû äåëàåì îêíî ïðîçðà÷íûì â áåëûõ åãî ÷àñòÿõ
			SetLayeredWindowAttributes(hwnd,RGB(255,255,255),NULL,LWA_COLORKEY);
			break;
		case WM_PAINT: // Ðèñóåì êðóæêè äëÿ ãëàç
			PAINTSTRUCT ps;
			HDC hdc;
			hdc=BeginPaint(hwnd,&ps);
			SelectObject(hdc,green_pen);
			Ellipse(hdc,2,2,97,97);
			EndPaint(hwnd,&ps);
			break;
		case WM_USER_MOVEWINDOW:
			 MoveWindow(hwnd, static_cast<int>(wparam), static_cast<int>(lparam), 100, 100, FALSE);
			 break;
		case WM_CLOSE: // Çàêðûòèå îêíà èãíîðèðóåòñÿ
			break;
		default:
			return DefWindowProc(hwnd,message,wparam,lparam);
	}
	return 0; // Îáðàáîòàëè, ñâàëèëèñü ñþäà ïî break
}
// Ñîçäàíèå îêíà
int CircleWindow::Init()
{
	ATOM aresult; // Äëÿ âñÿêèõ êîäîâ âîçâðàòà
	TCHAR *CircleWindowCName=L"MhookCircleWindow1.0";
	// 1. Ðåãèñòðàöèÿ êëàññà îêíà
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, CircleTranspWndProc, 0,
		0,
		MHInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL,
		CircleWindowCName
	};
	aresult=::RegisterClass(&wcl);
	if (aresult==0)
	{
#ifdef _DEBUG
		MHReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
#endif
		return 1;
	}
	// Ñîçäà¸ì îêíî
	CircleHwnd=CreateWindowEx(
		//WS_EX_LAYERED|WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
		WS_EX_LAYERED|WS_EX_TOPMOST,
		CircleWindowCName,
		NULL,
		WS_POPUP,
		100,100,
		100,100,
		0,0,
		MHInst,
		0 );
	if(NULL==CircleHwnd)
	{
#ifdef _DEBUG
		MHReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
#endif
		return 1;
	}
	// Ïîêàçûâàåì îêíî (ïîòîì îòëîæèì ïîêàç äî âðåìåíè ïåðåõâàòà ìûøè)
	//ShowWindow( CircleHwnd, SW_SHOWNORMAL);
	//UpdateWindow( CircleHwnd );
	return 0;
}
 void CircleWindow::Show()
 {
	 if(CircleHwnd&&G_eytracker_is_working) ShowWindow( CircleHwnd, SW_SHOWNORMAL);
 }
 void CircleWindow::Hide()
 {
	 if(CircleHwnd) ShowWindow( CircleHwnd, SW_HIDE);
 }