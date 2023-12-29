#include "helperFunctions.h"
#include "ConfigFile.h"
#include "Limit.h"
#include <algorithm>

void Helper::Exitapp(bool debug)
{
    std::wcout << "shutting down\n";
    ShellExecute(NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE);
    if (!debug) {
        ShowWindow(GetConsoleWindow(), SW_RESTORE);
    }
    PostQuitMessage(0);
}


bool Helper::D2Active()
{
    TCHAR buffer[MAX_PATH] = { 0 };
    DWORD dw_proc_id       = 0;

    GetWindowThreadProcessId(GetForegroundWindow(), &dw_proc_id);

    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dw_proc_id);
    GetModuleFileNameEx(hProc, NULL, buffer, MAX_PATH);
    CloseHandle(hProc);

    const wchar_t* filename = GetFilename(buffer);
    printf("active window filename: %ls\n", filename);

    if (wcscmp(filename, L"destiny2.exe") == 0) {
        return true;
    } else {
        return false;
    }
}


bool Helper::RunningAsAdmin()
{
    bool fRet     = false;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
            fRet = elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
}


const wchar_t* Helper::GetFilename(const wchar_t* path)
{
    const wchar_t* filename = wcsrchr(path, '\\');
    if (filename == NULL)
        filename = path;
    else
        filename++;
    return filename;
}
