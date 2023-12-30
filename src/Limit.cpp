#include "Limit.h"
#include "ConfigFile.h"
#include "HelperFunctions.h"
#include <Windows.h>

#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10

namespace Klim
{
    void Limit::ToggleWholeGameLimit(std::atomic<Limit>* lim_game)
    {
        Limit temp_limit = lim_game->load();
        temp_limit.state = !temp_limit.state;
        lim_game->store(temp_limit);

        if (temp_limit.state)
        {
            ShellExecute(nullptr, nullptr, L"powershell.exe", L"-ExecutionPolicy bypass -noe -c New-NetQosPolicy -Name 'Destiny2-Limit' -AppPathNameMatchCondition 'destiny2.exe' -ThrottleRateActionBitsPerSecond 0.801KB", nullptr, SW_HIDE);
        }
        else
        {
            ShellExecute(nullptr, nullptr, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", nullptr, SW_HIDE);
        }
    }


    void Limit::ToggleSuspend(std::atomic<Limit>* suspend)
    {
        // prevents from pausing random stuff if running with debug
        if (!Helper::D2Active())
        {
            MessageBox(nullptr, L"failed to pause...\nd2 is not the active window", nullptr, MB_OK | MB_ICONWARNING);
            return;
        }

        DWORD pid = 0;
        GetWindowThreadProcessId(GetForegroundWindow(), &pid);
        suspend->load().state ? SuspendProcess(pid, false) : SuspendProcess(pid, true);

        Limit temp_limit = suspend->load();
        temp_limit.state = !temp_limit.state;
        suspend->store(temp_limit);
    }


    void Limit::SuspendProcess(const DWORD pid, const bool suspend)
    {
        if (pid == 0)
        {
            return;
        }

        const HANDLE process_handle = OpenProcess(PROCESS_SUSPEND_RESUME, 0, pid);

        if (process_handle == nullptr)
        {
            return;
        }

        typedef LONG(NTAPI * NtSuspendProcess)(IN HANDLE ProcessHandle);
        typedef LONG(NTAPI * NtResumeProcess)(IN HANDLE ProcessHandle);

        if (suspend)
        {
            std::cout << "suspending process\n";
            const NtSuspendProcess pfn_NtSuspendProcess = reinterpret_cast<NtSuspendProcess>(GetProcAddress(GetModuleHandleA("ntdll"), "NtSuspendProcess"));
            pfn_NtSuspendProcess(process_handle);
        }
        else
        {
            std::cout << "resuming process\n";
            const NtResumeProcess pfn_NtResumeProcess = reinterpret_cast<NtResumeProcess>(GetProcAddress(GetModuleHandleA("ntdll"), "NtResumeProcess"));
            pfn_NtResumeProcess(process_handle);
        }

        CloseHandle(process_handle);
    }
}
