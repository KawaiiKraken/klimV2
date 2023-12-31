#include "HelperFunctions.h"
#include "ConfigFile.h"
#include <iostream>
#include <psapi.h>
#include <windows.h>

namespace Klim
{
    std::vector<std::string> Helper::StringSplit(const std::string& string, const std::string& delimiter)
    {
        size_t pos_start = 0, pos_end, delimiter_length = delimiter.length();
        std::vector<std::string> res;

        while ((pos_end = string.find(delimiter, pos_start)) != std::string::npos)
        {
            std::string token = string.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delimiter_length;
            res.push_back(token);
        }

        res.push_back(string.substr(pos_start));
        return res;
    }

    void Helper::ExitApp(const bool debug)
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
        std::wcout << L"active window filename: " << filename << L"\n";

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
}
