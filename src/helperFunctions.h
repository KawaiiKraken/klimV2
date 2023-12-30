#pragma once
#include <string>
#include <vector>

namespace Klim
{

    class Helper
    {
        public:
            static std::vector<std::string> StringSplit(const std::string& string, const std::string& delimiter);

            static bool D2Active();
            static bool RunningAsAdmin();
            static void ExitApp(bool debug);
            static const wchar_t* GetFileName(const wchar_t* path);
    };
}
