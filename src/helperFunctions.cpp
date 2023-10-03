#include "helperFunctions.h"

const wchar_t* GetFileName( const wchar_t *path )
{
    const wchar_t *filename = wcsrchr( path, '\\' );
    if ( filename == NULL )
        filename = path;
    else
        filename++;
    return filename;
}

bool isD2Active()
{
    TCHAR buffer[MAX_PATH] = {0};
    DWORD dwProcId = 0; 
    GetWindowThreadProcessId( GetForegroundWindow(), &dwProcId );
    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dwProcId );
    GetModuleFileNameEx( hProc, NULL, buffer, MAX_PATH );
    printf( "buffer: %ls\n", buffer );
    const wchar_t* bufferFilename = GetFileName( buffer );
    printf( "filename: %ls\n", bufferFilename );
    CloseHandle( hProc );
    if ( wcscmp( bufferFilename, L"destiny2.exe" ) == 0 ){
        return TRUE;
    } else {
        return FALSE;
    }
}