#pragma once

#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <vector>
#include <mutex>
#include <iostream>

class Helper {
public:
	static bool D2Active();
	static bool RunningAsAdmin();
	static void Exitapp(bool debug);
	static const wchar_t* GetFilename(const wchar_t* path);
};
