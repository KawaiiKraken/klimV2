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

// TODO make it not take away focus when spawned

LPCTSTR mytext;
HWND hwnd, emptyhwnd;
HINSTANCE dllHinst;
HANDLE threadHandle = nullptr;
int nCmdShow = NULL;
RECT screenSize;

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
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.
    int winstyle = (WS_VISIBLE | WS_SYSMENU);
    hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,// Optional window styles. // remove WS_EX_APPWINDOW to hide from taskbar WS_EX_TOOLWINDOW??
        //WS_EX_APPWINDOW | WS_EX_TOPMOST ,// Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
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
    SetWindowLong(hwnd, GWL_STYLE, winstyle & ~WS_BORDER);
    // set window pos
    SystemParametersInfoA(SPI_GETWORKAREA, NULL, &screenSize, NULL);
    int ret;
    //ret = AdjustWindowRect(&screenSize, NULL, 0);
    ret = SetWindowPos(hwnd, HWND_TOPMOST, screenSize.left, screenSize.top, screenSize.right, screenSize.bottom, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    if (ret == 0){
        std::cout << "dll: adjustWindowRect FAILED" << std::endl;
    }
    std::cout << "dll: xpos - " << screenSize.left << "; ypos - " << screenSize.top << "; cx - " << screenSize.right << "; cy - " << screenSize.bottom << std::endl;

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
            HFONT hFontOriginal, hFont1;
            hFont1 = CreateFont(30, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, 
                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
            hFontOriginal = (HFONT)SelectObject(hdc, hFont1);
            
            //Sets the coordinates for the rectangle in which the text is to be formatted.
            SetRect(&rect, 25, 25, screenSize.right, 225); // set to screen width so text is never cut
            SetTextColor(hdc, RGB(255,255,255));
            DrawText(hdc, mytext, -1, &rect, NULL);
            //std::wcout << L"dll: DrawText " << mytext << std::endl;
            SelectObject(hdc,hFontOriginal);
            DeleteObject(hFont1);
        
            // !!!ENDFONT!!!

            EndPaint(hwnd, &ps);
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
__declspec(dllexport) DWORD WINAPI Overlay(LPTSTR inputText)
{
    mytext = inputText;
    //std::cout << "dll: attach" << std::endl;
    //std::wcout << L"dll: mytext = " << mytext << std::endl;
    //std::cout << "dll: get thread id" << std::endl;
    threadId = GetThreadId(threadHandle);
    //std::cout << "dll: " << threadId << std::endl;
    if (threadId == 0){ // if window created update instead
        //std::cout << "dll: create thread" << std::endl;
        threadHandle = CreateThread(NULL, NULL, myfunc, NULL, 0, NULL);
    } else {
        //std::cout << "dll: update window " << hwnd << std::endl;
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
    }
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
