// RDMaddonWifi.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RDMaddonWifi.h"

//get WLAN RSSI
typedef UINT (*PFN_GetRSSI)(int*);
PFN_GetRSSI GetRSSI=NULL;

// The loadlibrary HINSTANCE destination
HINSTANCE h802lib=NULL;

//#define MYDEBUG	//for testing

SYSTEM_POWER_STATUS_EX pwrStatus;

//window position
DWORD screenX=0;
DWORD screenY=0;
DWORD dwLeft=100;
DWORD dwTop=100;
DWORD blockWidth=8;
DWORD blockHeight=14;
DWORD blockCount=4;
DWORD blockMargin=1;
// main window size, 
//	width = blockwidth+2*blockMargin
//	height = blockCount*blockHeight+blockCount*blockMargin*2-2*blockMargin
//======================================================================
DWORD FLOATWIDTH	=	16; //100 //200                     // Width of floating wnd
DWORD FLOATHEIGHT	=	80; //100 //100                     // Height of floating wnd
//======================================================================
/*  0							top			bottom
   0+---------+
	| +-----+ |					1*margin	
	| |     | |	75-100	green				1*margin+1*blockHeight
	| +-----+ |								
	| +-----+ |					2*margin+1*blockHeight
	| |     | |	51-75	yellow				2*margin+2*blockHeight
	| +-----+ |								
	| +-----+ |					3*margin+2*blockHeight
	| |     | | 25-50	orange				3*margin+3*blockHeight
	| +-----+ |
	| +-----+ |					4*margin+3*blockHeight
	| |     | | 0-24	red					4*margin+4*blockHeight
	| +-----+ |
	+---------+
*/
// 
// vars for the drawings 
// 
//colors
unsigned long colorLevelHigh		=	RGB(0,		0xFF,	0xff);
unsigned long colorLevelBelowHigh	=	RGB(0,		0x99,	0xFF);
unsigned long colorLevelBelowMid	=	RGB(0xFF,	0xD8,	0x00);
unsigned long colorLevelBad			=	RGB(0xff,	0x6A,	0x00);

unsigned long colorLevelUnknow		=	RGB(0xFF,	0xE4,	0xFF);

unsigned long colorBatt				=	RGB(0x80,	0x80,	0x80);


unsigned long colorWinBackground	=	RGB(0x04,	0x4A,	0x1C);

static HBRUSH hBackground = CreateSolidBrush( colorWinBackground );

DWORD dwTimerID=1000;
UINT hTimer=NULL;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
//HWND				g_hWndCommandBar;	// command bar handle
HWND g_hWnd=NULL;

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int getPercentRSSI(int iRSSI){
	//iRSSI = -30 to -100
	int iRet=iRSSI+100;
	iRet=iRet*1.4;
	return iRet;
}

int _level=-30;

int getValueLevel(){
#ifdef MYDEBUG
	int iRSSI=getPercentRSSI(_level);
	_level-=25/1.4;
	if(_level<-100)
		_level=-30;
	return iRSSI;
#endif
	int iLevel = 0;
	if(h802lib == NULL)
		h802lib = LoadLibrary(_T("80211api.dll"));
	
	if(GetRSSI==NULL)
		GetRSSI = (PFN_GetRSSI)GetProcAddress(h802lib, _T("GetRSSI")); // Range is -100 dBm to -30 dBm
	if(GetRSSI!=NULL)
	{
		int iRes=0;
		if( (iRes=GetRSSI(&iLevel)) == 0 ){
			DEBUGMSG(1, (L"GetRSSI =%i\n", iLevel));
			iLevel+=100;
			DEBUGMSG(1, (L"GetRSSI level=%i\n", iLevel));
		}
		else{
			DEBUGMSG(1, (L"GetRSSI error=%i\n", iRes));
			return 0;
		}
	}
	return iLevel;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		//Do not end before quit message has been received
		//return FALSE;
	}
	else{
		HACCEL hAccelTable;
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RDMADDONBATT2));

		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE | CS_DBLCLKS;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RDMADDONBATT2));
	wc.hCursor       = 0;
	wc.hbrBackground = hBackground;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable


    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_RDMADDONBATT2, szWindowClass, MAX_LOADSTRING);


    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

	HWND hWndTS = FindWindow(L"TSSHELLWND", NULL);
#ifndef MYDEBUG
	if(hWndTS==NULL){
		DEBUGMSG(1, (L"### TSSHELLWND not found. EXIT. ###\n"));
		return FALSE;
	}
#else
	hWndTS=GetForegroundWindow();
#endif
    //hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
    //    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	hWnd = CreateWindowEx(
		0, //WS_EX_TOPMOST | WS_EX_ABOVESTARTUP, 
		szWindowClass, 
		NULL /* szTitle */, // NO CAPTION
		WS_VISIBLE, //WS_VISIBLE | WS_EX_ABOVESTARTUP, // | WS_EX_TOOLWINDOW | WS_CHILD | WS_POPUP | WS_NONAVDONEBUTTON, 
		0, 
		0, 
		FLOATWIDTH, 
		FLOATHEIGHT, 
		hWndTS, // NULL, 
		NULL, 
		hInstance, 
		NULL);

    if (!hWnd)
    {
        return FALSE;
    }

	g_hWnd=hWnd;

	LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
	DEBUGMSG(1, (L"GetWindowLong GWL_SYTLE=%08x\n", lStyle));

	screenX = GetSystemMetrics(SM_CXSCREEN);
	screenY = GetSystemMetrics(SM_CYSCREEN);
	DWORD dwX, dwY, dwW, dwH;

	//size
	dwW=blockWidth+2*blockMargin;// + GetSystemMetrics(SM_CXEDGE) * 2;// FLOATWIDTH;
	dwH=blockCount*blockHeight + blockCount*blockMargin + blockMargin;// + GetSystemMetrics(SM_CYEDGE) * 2;//FLOATHEIGHT;
	//position
	dwX=screenX-blockWidth;// FLOATWIDTH;
	dwY=screenY/2-dwH-blockHeight;//move lower end to mid //-FLOATHEIGHT; 

	//MoveWindow(hWnd, dwX, dwY, dwW, dwH, TRUE);

	SetWindowPos(hWnd, HWND_TOPMOST, dwX, dwY, dwW, dwH, SWP_SHOWWINDOW);

	//move the window 
	//SetWindowPos(hWnd, HWND_TOPMOST, dwLeft, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
	RECT rect;
	static HBRUSH hBr, hBrBatt, hbrLevelLow, hbrLevelBelowMid, hbrLevelHigh, hbrLevelUnknown, hbrLevelBelowHigh;
	static HPEN hPen;
	unsigned long fillColor = colorLevelBelowMid;
	static int iValueLevel=100;
	int iRes=0;

    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:

			hBr = CreateSolidBrush( fillColor );
			hBrBatt = CreateSolidBrush ( colorBatt );
			hbrLevelLow = CreateSolidBrush(colorLevelBad);
			hbrLevelBelowMid = CreateSolidBrush(colorLevelBelowMid);
			hbrLevelHigh = CreateSolidBrush(colorLevelHigh);
			hbrLevelUnknown = CreateSolidBrush(colorLevelUnknow);
			hbrLevelBelowHigh=CreateSolidBrush(colorLevelBelowHigh);

			hTimer=SetTimer(hWnd, dwTimerID, 30 *1000, NULL);	//30 seconds

			iValueLevel = getValueLevel();
            break;
		case WM_TIMER:
			if(wParam==dwTimerID){
#ifndef MYDEBUG
				if(FindWindow(L"TSSHELLWND", NULL)==NULL)
					PostQuitMessage(-2);
#endif
				iValueLevel = getValueLevel();

				DEBUGMSG(1, (L"getValueLevel=%i\n", iValueLevel));
				GetClientRect(hWnd, &rect);
				InvalidateRect(hWnd, &rect, TRUE);
			}
			break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            GetClientRect(hWnd, &rect);
			iRes=FillRect(hdc, &rect, hBackground);			

			DEBUGMSG(1,(L"Painting for batt=%i\n",iValueLevel));
			//upper bar
			rect.left=blockMargin; rect.top=blockMargin; 
			rect.right=blockWidth+blockMargin; rect.bottom=blockHeight+blockMargin;

			if(iValueLevel>75)
				iRes=FillRect(hdc, &rect, hbrLevelHigh);
			else
				iRes=FillRect(hdc, &rect, hBackground);

			//second bar
			rect.left=1; rect.top=2*blockMargin+1*blockHeight; rect.right=blockWidth; rect.bottom=2*blockMargin+2*blockHeight;
			if(iValueLevel>50){
				iRes=FillRect(hdc, &rect, hbrLevelBelowHigh);
			}

			//third bar
			rect.left=1; rect.top=3*blockMargin+2*blockHeight; rect.right=blockWidth; rect.bottom=3*blockMargin+3*blockHeight;
			if(iValueLevel>25){
				iRes=FillRect(hdc, &rect, hbrLevelBelowMid);
			}

			//forth and last bar
			rect.left=1; rect.top=4*blockMargin+3*blockHeight; rect.right=blockWidth; rect.bottom=4*blockMargin+4*blockHeight;
			if(iValueLevel>15){
				iRes=FillRect(hdc, &rect, hbrLevelLow);
			}
			if(iValueLevel<15){
				iRes=FillRect(hdc, &rect, hbrLevelUnknown);
			}
			
            EndPaint(hWnd, &ps);
            break;
		case WM_LBUTTONDBLCLK:
			if(MessageBox(hWnd, L"Exit?", L"Battery Monitor", MB_YESNO|MB_ICONQUESTION)==IDYES)
				PostQuitMessage(-1);
			break;
        case WM_DESTROY:
            PostQuitMessage(0);
			DeleteObject(hBrBatt);
			DeleteObject(hbrLevelLow);
			DeleteObject(hbrLevelHigh);
			DeleteObject(hbrLevelBelowMid);
			DeleteObject(hbrLevelUnknown);
			DeleteObject(hbrLevelBelowHigh);
			DeleteObject(hBr);
			DeleteObject(hPen);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            RECT rectChild, rectParent;
            int DlgWidth, DlgHeight;	// dialog width and height in pixel units
            int NewPosX, NewPosY;

            // trying to center the About dialog
            if (GetWindowRect(hDlg, &rectChild)) 
            {
                GetClientRect(GetParent(hDlg), &rectParent);
                DlgWidth	= rectChild.right - rectChild.left;
                DlgHeight	= rectChild.bottom - rectChild.top ;
                NewPosX		= (rectParent.right - rectParent.left - DlgWidth) / 2;
                NewPosY		= (rectParent.bottom - rectParent.top - DlgHeight) / 2;
				
                // if the About box is larger than the physical screen 
                if (NewPosX < 0) NewPosX = 0;
                if (NewPosY < 0) NewPosY = 0;
                SetWindowPos(hDlg, 0, NewPosX, NewPosY,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;

    }
    return (INT_PTR)FALSE;
}
