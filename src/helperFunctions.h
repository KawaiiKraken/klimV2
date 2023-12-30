#pragma once

#include <iostream>
#include <mutex>
#include <vector>
#include <windows.h>

class Helper
{
    public:
        static bool D2Active();
        static bool RunningAsAdmin();
        static void ExitApp(bool debug);
        static const wchar_t* GetFileName(const wchar_t* path);
};
