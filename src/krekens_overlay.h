#pragma once
#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
__declspec( dllexport ) DWORD WINAPI UpdateOverlayLine( LPTSTR text, int linenum, COLORREF color );
__declspec( dllexport ) DWORD WINAPI startOverlay( bool isOverlayArg, int fontSizeArg );
#ifdef __cplusplus
}
#endif