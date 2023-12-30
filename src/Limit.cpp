#include <Windows.h>

#include "helperFunctions.h"
#include "Limit.h"
#include "ConfigFile.h"

#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10


void Limit::ToggleWholeGameLimit(std::atomic<limit>* lim_game)
{
    limit temp_limit = lim_game->load();
    temp_limit.state = !temp_limit.state;
    lim_game->store(temp_limit);

    if (temp_limit.state) {
        ShellExecute(NULL, NULL, L"powershell.exe",
            L"-ExecutionPolicy bypass -noe -c New-NetQosPolicy -Name 'Destiny2-Limit' -AppPathNameMatchCondition 'destiny2.exe' -ThrottleRateActionBitsPerSecond 0.801KB", NULL,
            SW_HIDE);
    } else {
        ShellExecute(NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE);
    }
}


void Limit::ToggleSuspend(std::atomic<limit>* suspend)
{
    if (!Helper::D2Active()) { // prevents from pausing random stuff if running with debug
        MessageBox(NULL, L"failed to pause...\nd2 is not the active window", NULL, MB_OK | MB_ICONWARNING);
        return;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    suspend->load().state ? SuspendProcess(pid, false) : SuspendProcess(pid, true);

    limit temp_limit = suspend->load();
    temp_limit.state = !temp_limit.state;
    suspend->store(temp_limit);
}


void Limit::SuspendProcess(DWORD pid, bool suspend)
{
    if (pid == 0) {
        return;
    }

    HANDLE hProc = OpenProcess(PROCESS_SUSPEND_RESUME, 0, pid);

    if (hProc == NULL) {
        return;
    }

    typedef LONG (NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle );
    typedef LONG (NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle );

    if (suspend) {
        std::cout << "suspending process" << std::endl;
        NtSuspendProcess pfn_NtSuspendProcess = (NtSuspendProcess)GetProcAddress(
					GetModuleHandleA( "ntdll" ), "NtSuspendProcess" );
        pfn_NtSuspendProcess(hProc);
    } else {
        std::cout << "resuming process" << std::endl;
        	NtResumeProcess pfn_NtResumeProcess = (NtResumeProcess)GetProcAddress(
		GetModuleHandleA( "ntdll" ), "NtResumeProcess" );
        pfn_NtResumeProcess(hProc);
    }

    CloseHandle(hProc);
}
