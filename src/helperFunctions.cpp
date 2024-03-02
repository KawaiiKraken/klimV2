#include "HelperFunctions.h"
#include "ConfigFile.h"
#include <filesystem>
#include <iostream>
#include <psapi.h>
#include <shellapi.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
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

    void Helper::ExitApp(const bool console, std::shared_ptr<spdlog::logger> logger)
    {
        if (logger != nullptr)
        {
            logger->info("shutting down");
        }
        ShellExecute(nullptr, nullptr, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", nullptr, SW_HIDE);
        if (!console)
        {
            ShowWindow(GetConsoleWindow(), SW_RESTORE);
        }
        PostQuitMessage(0);
    }

    // logger can be nullptr
    bool Helper::IsDestinyTheActiveWindow(std::shared_ptr<spdlog::logger> logger)
    {
        TCHAR buffer[MAX_PATH] = { 0 };
        DWORD dw_proc_id = 0;

        GetWindowThreadProcessId(GetForegroundWindow(), &dw_proc_id);

        const HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dw_proc_id);
        GetModuleFileNameEx(process_handle, nullptr, buffer, MAX_PATH);
        CloseHandle(process_handle);

        const wchar_t* filename = GetFileName(buffer);
        if (logger != nullptr)
        {
            char filename_str[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, filename, MAX_PATH, filename_str, MAX_PATH, 0, 0);
            logger->info("active window: {}", filename_str);
        }

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
            TOKEN_ELEVATION elevation = { 0 };
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

    std::shared_ptr<spdlog::logger> Helper::LoggerInit(const std::string& log_file_name)
    {
        // Remove old log files on startup
        for (const auto& entry : std::filesystem::directory_iterator("."))
        {
            if (entry.is_regular_file() && entry.path().filename().string().find(log_file_name) != std::string::npos)
            {
                std::filesystem::remove(entry);
            }
        }
        spdlog::init_thread_pool(8192, 1);

        wchar_t filename[MAX_PATH];
        mbstowcs_s(0, filename, log_file_name.c_str(), MAX_PATH);

        wchar_t path[MAX_PATH];
        ConfigFile::SetPathToFileInExeDir(filename, path);

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name);
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        file_sink->set_level(spdlog::level::trace);
        console_sink->set_level(spdlog::level::trace);

        std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list { file_sink, console_sink });
        logger->set_pattern("[%M:%S.%e] [%^%l%$] %v");
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
        return logger;
    }


    std::string Helper::GetMouseButtonNameByVkCode(int vk_code)
    {
        switch (vk_code)
        {
            case VK_LBUTTON:
                return "LB";
                break;
            case VK_RBUTTON:
                return "RB";
                break;
            case VK_MBUTTON:
                return "MB";
                break;
            case VK_XBUTTON1:
                return "XB1";
                break;
            case VK_XBUTTON2:
                return "XB2";
                break;
            default:
                return "Unknown mouse button";
        }
    }
}
