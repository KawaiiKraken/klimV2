// The export mechanism used here is the __declspec(export)
// method supported by Microsoft Visual Studio, but any
// other export method supported by your development
// environment may be substituted.
 
 
#include <windows.h>
#include <iostream>

#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "gdi32.lib" )
//#pragma clang diagnostic ignored "-Wwritable-strings"

// TODO make it not take away focus when spawned

//LPCTSTR mytext;
wchar_t mytext[10][1000];
HWND hwnd, emptyhwnd;
HINSTANCE dllHinst;
HANDLE threadHandle = nullptr;
int nCmdShow = NULL;
bool isOverlay;
RECT screenSize;
COLORREF colors[10];

//using namespace std;

RECT myDrawText(wchar_t* text, int index, RECT rect, HDC hdc){
            SetTextColor(hdc, colors[index]);
            DrawText(hdc, ::mytext[index], -1, &rect, NULL);
            DrawText(hdc, ::mytext[index], -1, &rect, DT_CALCRECT);
            rect.top = rect.bottom;
            rect.bottom = screenSize.bottom;
            rect.right = screenSize.right;
            return rect;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

DWORD WINAPI myfunc(LPVOID lpParam)
{
    //std::cout << "dll: myfunc" << std::endl; 
    HINSTANCE hInstance = dllHinst;
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.
    int winstyle = (WS_VISIBLE | WS_SYSMENU | WS_DISABLED);
    int ex_winstyle = ( WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE );
    if ( isOverlay == false ){
        winstyle = ( WS_VISIBLE | WS_OVERLAPPEDWINDOW );
        ex_winstyle = ( 0 );
    }
    hwnd = CreateWindowEx(
        ex_winstyle,// Optional window styles. // remove WS_EX_APPWINDOW to hide from taskbar WS_EX_TOOLWINDOW??
        CLASS_NAME,                     // Window class
        L"klimV2",    // Window text
        winstyle,          // Window style
        //WS_VISIBLE | WS_POPUP,          // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );
    SetLayeredWindowAttributes(hwnd, RGB(10,10,10), 0, LWA_COLORKEY);
    if ( isOverlay == true ){
        SetWindowLong(hwnd, GWL_STYLE, winstyle & ~WS_BORDER);
    }
    // set window pos
    SystemParametersInfoA(SPI_GETWORKAREA, NULL, &screenSize, NULL);
    int ret;
    //ret = AdjustWindowRect(&screenSize, NULL, 0);
    if ( isOverlay == true ){
        ret = SetWindowPos(hwnd, HWND_TOPMOST, screenSize.left, screenSize.top, screenSize.right, screenSize.bottom, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        if (ret == 0){
            std::cout << "dll: adjustWindowRect FAILED" << std::endl;
        }
        std::cout << "dll: xpos - " << screenSize.left << "; ypos - " << screenSize.top << "; cx - " << screenSize.right << "; cy - " << screenSize.bottom << std::endl;
    }

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOWNA);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.
            FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(10,10,10)));
            SetBkColor(hdc, RGB(10, 10, 10));
            // !!!FONT!!!
            RECT rect;
            HFONT hFontOriginal, hFont1, hFont2;
            hFont1 = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, 
                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
            hFontOriginal = (HFONT)SelectObject(hdc, hFont1);
            SetRect(&rect, 8, 8, screenSize.right, screenSize.bottom); // set to screen width so text is never cut
            SetTextColor(hdc, RGB(255,255,255));
            DrawText(hdc, L"made by _kreken", -1, &rect, NULL);
            DrawText(hdc, L"made by _kreken", -1, &rect, DT_CALCRECT);
            SelectObject(hdc,hFontOriginal);
            DeleteObject(hFont1);

            hFont2 = CreateFont(30, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, 
                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
            hFontOriginal = (HFONT)SelectObject(hdc, hFont2);
            
            rect.top = rect.bottom;
            rect.bottom = screenSize.bottom;
            rect.right = screenSize.right;
            rect = myDrawText(::mytext[0], 0, rect, hdc);
            rect = myDrawText(::mytext[0], 1, rect, hdc);
            rect = myDrawText(::mytext[0], 2, rect, hdc);
            rect = myDrawText(::mytext[0], 3, rect, hdc);
            rect = myDrawText(::mytext[0], 4, rect, hdc);
            rect = myDrawText(::mytext[0], 5, rect, hdc);
            rect = myDrawText(::mytext[0], 6, rect, hdc);
            rect = myDrawText(::mytext[0], 7, rect, hdc);
            rect = myDrawText(::mytext[0], 8, rect, hdc);
            rect = myDrawText(::mytext[0], 9, rect, hdc);
            SelectObject(hdc,hFontOriginal);
            DeleteObject(hFont2);
        
            // !!!ENDFONT!!!

            EndPaint(hwnd, &ps);
            return 0;
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 
#define EOF (-1)
 
#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
 
int threadId = 0;
__declspec( dllexport ) DWORD WINAPI startOverlay( bool isOverlayArg ) // TODO split into multiple functions
{
    std::cout << "overlay startup\n";
    //::mytext = L"a";
    wcscpy_s(::mytext[0], (wchar_t*)L"asdf");
    std::cout << "overlay startup 2\n";
    isOverlay = isOverlayArg;
    threadId = GetThreadId(threadHandle);
    if (threadId == 0){ // if window created update instead
        threadHandle = CreateThread(NULL, NULL, myfunc, NULL, 0, NULL);
    }
    return 0;
}

__declspec(dllexport) DWORD WINAPI updateOverlayLine( LPTSTR text, int linenum, COLORREF color) // TODO split into multiple functions
{
    colors[linenum-1] = color;
    wcscpy_s(::mytext[linenum-1], text);
    printf("1: mytext[%d] = \"%ls\"\n", linenum-1, ::mytext[linenum-1]);
    RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
    return 0;
}


BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD ul_reason_for_call,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{
    dllHinst = hinstDLL;
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
  }
    return TRUE;
}
 
#ifdef __cplusplus
}
#endif
