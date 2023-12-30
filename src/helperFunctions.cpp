#include <algorithm>
#include <psapi.h>
#include <Windows.h>
#include "HelperFunctions.h"
#include "ConfigFile.h"
#include "Limit.h"

void Helper::ExitApp(bool debug)
{
    std::cout << "shutting down\n";
    ShellExecute(nullptr, nullptr, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", nullptr, SW_HIDE);
    if (!debug) 
    {
        ShowWindow(GetConsoleWindow(), SW_RESTORE);
    }
    PostQuitMessage(0);
}


bool Helper::D2Active()
{
    TCHAR buffer[MAX_PATH] = { 0 };
    DWORD dw_proc_id = 0;

    GetWindowThreadProcessId(GetForegroundWindow(), &dw_proc_id);

    const HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dw_proc_id);
    GetModuleFileNameEx(process_handle, nullptr, buffer, MAX_PATH);
    CloseHandle(process_handle);

    const wchar_t* filename = GetFileName(buffer);
    std::cout << "active window filename: " << filename << "\n";

    if (wcscmp(filename, L"destiny2.exe") == 0) 
    {
        return true;
    }

    return false;
}


bool Helper::RunningAsAdmin()
{
    bool is_admin = false;
    HANDLE token_handle = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token_handle)) 
    {
        TOKEN_ELEVATION elevation;
        DWORD cb_size = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(token_handle, TokenElevation, &elevation, sizeof(elevation), &cb_size)) 
        {
            is_admin = elevation.TokenIsElevated;
        }
    }
    if (token_handle) 
    {
        CloseHandle(token_handle);
    }
    return is_admin;
}


const wchar_t* Helper::GetFileName(const wchar_t* path)
{
    const wchar_t* filename = wcsrchr(path, '\\');
    if (filename == nullptr) 
    {
        filename = path;
    }
    else 
    {
        filename++;
    }
    return filename;
}
