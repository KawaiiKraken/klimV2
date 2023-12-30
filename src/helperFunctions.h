#pragma once

#include <iostream>
#include <mutex>
#include <vector>
#include <windows.h>

class Helper {
public:
    static bool D2Active();
    static bool RunningAsAdmin();
    static void Exitapp(bool debug);
    static const wchar_t* GetFilename(const wchar_t* path);
};
