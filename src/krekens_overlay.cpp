#include <windows.h>
#include <iostream>

#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "gdi32.lib" )
//#pragma clang diagnostic ignored "-Wwritable-strings"

// TODO make it not take away focus when spawned
// TODO comments

wchar_t overlay_text[20][1000];
COLORREF line_color_arr[10];
HWND hwnd;
HINSTANCE dllHInst;
HANDLE threadHandle = nullptr;
int fontSize;
bool useOverlay;
RECT screenSize;
COLORREF transparent = RGB(1, 1, 1);


RECT DrawTextLine( wchar_t* text, int index, RECT rect, HDC hdc ){
            SetTextColor( hdc, line_color_arr[index] );
            DrawText( hdc, ::overlay_text[index], -1, &rect, NULL );
            DrawText( hdc, ::overlay_text[index], -1, &rect, DT_CALCRECT );
            rect.top = rect.bottom;
            rect.bottom = screenSize.bottom;
            rect.right = screenSize.right;
            return rect;
}

LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

DWORD WINAPI funcCreateWindow( LPVOID lpParam )
{
    HINSTANCE hInstance = dllHInst;
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass( &wc );

    // Create the window.
    int window_styles = ( WS_VISIBLE | WS_SYSMENU | WS_DISABLED );
    int ex_window_styles = ( WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE );
    if ( useOverlay == false ){
        window_styles = ( WS_VISIBLE | WS_OVERLAPPEDWINDOW );
        ex_window_styles = ( 0 );
    }
    hwnd = CreateWindowEx( ex_window_styles, CLASS_NAME, L"klim", window_styles, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL );
    SetLayeredWindowAttributes( hwnd, transparent, 0, LWA_COLORKEY ); // set transparent color
    if ( useOverlay == true ){
        SetWindowLong( hwnd, GWL_STYLE, window_styles & ~WS_BORDER );
    }
    // set window pos
    SystemParametersInfoA( SPI_GETWORKAREA, NULL, &screenSize, NULL );
    int ret;
    if ( useOverlay == true ){
        ret = SetWindowPos( hwnd, HWND_TOPMOST, screenSize.left, screenSize.top, screenSize.right, screenSize.bottom, SWP_NOACTIVATE | SWP_SHOWWINDOW );
        printf( "dll: window position: x=%ld; y=%ld; cx=%ld; cy=%ld\n", screenSize.left, screenSize.top, screenSize.right, screenSize.bottom );
    }

    ShowWindow( hwnd, SW_SHOWNA );

    // Run the message loop.
    MSG msg = { };
    while ( GetMessage( &msg, NULL, 0, 0 ) > 0 )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return 0;
}

LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg )
    {
    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint( hwnd, &ps );
            FillRect( hdc, &ps.rcPaint, CreateSolidBrush( transparent ) );
            SetBkColor( hdc, transparent );
            HFONT hFont1 = CreateFont( fontSize-3, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, 
                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT( "Impact" ) );
            SelectObject( hdc, hFont1 );
            RECT rect;
            SetRect( &rect, 5, 5, screenSize.right, screenSize.bottom ); // set to screen width so text is never cut except its not full screen size for some reason??
            line_color_arr[0] = RGB( 255, 255, 255 );
            wcscpy_s( ::overlay_text[0], L"by kreky :3");
            for (int i = 0; i < 10; i++) {
                rect = DrawTextLine( ::overlay_text[0], i, rect, hdc );
            }
            DeleteObject( hFont1 );
            EndPaint( hwnd, &ps );
            return 0;
        }
        return 0;

    }
    return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

 
#define EOF (-1)
 
#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
 
int threadId = 0;
__declspec( dllexport ) DWORD WINAPI startOverlay( bool isOverlayArg, int fontSizeArg ) 
{
    printf( "dll: overlay start\n" );
    fontSize = fontSizeArg;
    useOverlay = isOverlayArg;
    threadId = GetThreadId( threadHandle );
    if ( threadId == 0 ){ // if window created update instead
        threadHandle = CreateThread( NULL, NULL, funcCreateWindow, NULL, 0, NULL );
    }
    return 0;
}

__declspec( dllexport ) DWORD WINAPI UpdateOverlayLine( LPTSTR text, int linenum, COLORREF color ) // TODO split into multiple functions
{
    if (linenum == -1) {
        return 0;
    }
    line_color_arr[linenum] = color;
    wcscpy_s( ::overlay_text[linenum], text );
    printf( "dll: overlay_text[%d] = \"%ls\"\n", linenum, ::overlay_text[linenum] );
    RedrawWindow( hwnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN );
    return 0;
}


BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD ul_reason_for_call,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{
    dllHInst = hinstDLL;
    switch ( ul_reason_for_call ) {
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
