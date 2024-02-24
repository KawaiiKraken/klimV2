#include "Limit.h"
#include "ConfigFile.h"
#include "HelperFunctions.h"
#include <Windows.h>
#include <iostream>

// This shuts up the ReSharper linter
// ReSharper disable CppInconsistentNaming
#define _WIN32_WINNT_WIN10 0x0A00 // Windows 10
// ReSharper restore CppInconsistentNaming

namespace Klim
{
    const char* Limit::TypeToString(const LimitType type)
    {
        switch (type)
        {
            case limit_3074_dl:
                return "3074(DL)";
            case limit_3074_ul:
                return "3074(UL)";
            case limit_27k_dl:
                return "27k(DL)";
            case limit_27k_ul:
                return "27k(UL)";
            case limit_30k_dl:
                return "30k(DL)";
            case limit_7500_dl:
                return "7500(DL)";
            case limit_full_game:
                return "Full_Game";
            case suspend_game:
                return "Suspend_Game";
            case exit_app:
                return "Exit_App";
            case invalid:
                return nullptr;
        }

        return "Invalid LimitType";
    }

    const char* Limit::TypeToRule(LimitType type)
    {
        switch (type)
        {
            case limit_3074_dl:
                return " or (inbound and udp.SrcPort == 3074)";
            case limit_3074_ul:
                return " or (outbound and udp.DstPort == 3074)";
            case limit_27k_dl:
                return " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200)";
            case limit_27k_ul:
                return " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200)";
            case limit_30k_dl:
                return " or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)";
            case limit_7500_dl:
                return " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)";
            default:
                return nullptr;
        }
    }

    LimitType Limit::StringToType(const char* str)
    {
        if (strcmp(str, "3074(DL)") == 0)
        {
            return limit_3074_dl;
        }
        if (strcmp(str, "3074(UL)") == 0)
        {
            return limit_3074_ul;
        }
        if (strcmp(str, "27k(DL)") == 0)
        {
            return limit_27k_dl;
        }
        if (strcmp(str, "27k(UL)") == 0)
        {
            return limit_27k_ul;
        }
        if (strcmp(str, "30k(DL)") == 0)
        {
            return limit_30k_dl;
        }
        if (strcmp(str, "7500(DL)") == 0)
        {
            return limit_7500_dl;
        }
        if (strcmp(str, "Full_Game") == 0)
        {
            return limit_full_game;
        }
        if (strcmp(str, "Suspend_Game") == 0)
        {
            return suspend_game;
        }
        if (strcmp(str, "Exit_App") == 0)
        {
            return exit_app;
        }

        return invalid;
    }

    std::atomic<Limit>* Limit::GetLimitPtrByType(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, LimitType limit)
    {
        for (int i = 0; i < limit_ptr_vector.size(); i++)
        {
            if (limit_ptr_vector[i]->load().type == limit)
            {
                return limit_ptr_vector[i];
            }
        }
        std::cout << "error: no matching limit ptr found\n";
        return nullptr; // is this safe?
    }

    void Limit::ToggleWholeGameLimit(std::atomic<Limit>* limit_ptr)
    {
        Limit temp_limit = limit_ptr->load();
        temp_limit.state = !temp_limit.state;
        limit_ptr->store(temp_limit);

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
            MessageBox(nullptr, L"failed to pause...\ndestiny2.exe is not the active window", nullptr, MB_OK | MB_ICONWARNING);
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

        const HANDLE suspend_resume_handle = OpenProcess(PROCESS_SUSPEND_RESUME, 0, pid);

        if (suspend_resume_handle == nullptr)
        {
            return;
        }

        // ReSharper disable CppInconsistentNaming
        typedef LONG(NTAPI * NtSuspendProcess)(IN HANDLE ProcessHandle);
        typedef LONG(NTAPI * NtResumeProcess)(IN HANDLE ProcessHandle);
        // ReSharper restore CppInconsistentNaming

        if (suspend)
        {
            std::cout << "suspending process\n";
            const NtSuspendProcess nt_suspend_process = reinterpret_cast<NtSuspendProcess>(GetProcAddress(GetModuleHandleA("ntdll"), "NtSuspendProcess"));
            nt_suspend_process(suspend_resume_handle);
        }
        else
        {
            std::cout << "resuming process\n";
            const NtResumeProcess nt_resume_process = reinterpret_cast<NtResumeProcess>(GetProcAddress(GetModuleHandleA("ntdll"), "NtResumeProcess"));
            nt_resume_process(suspend_resume_handle);
        }

        CloseHandle(suspend_resume_handle);
    }
}
