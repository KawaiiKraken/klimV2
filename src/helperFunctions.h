#pragma once
#include "spdlog/async.h"
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace Klim
{

    class Helper
    {
        public:
            static std::vector<std::string> StringSplit(const std::string& string, const std::string& delimiter);
            static bool IsDestinyTheActiveWindow(std::shared_ptr<spdlog::logger> logger);
            static bool RunningAsAdmin();
            static void ExitApp(const bool debug, std::shared_ptr<spdlog::logger> logger);
            static const wchar_t* GetFileName(const wchar_t* path);
            static std::shared_ptr<spdlog::logger> LoggerInit(const std::string& logFileName);
            static std::string GetMouseButtonNameByVkCode(int vk_code);
            static HWND GetHwndByExeName(const std::wstring& exeName);

        private:
            static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    };
}
