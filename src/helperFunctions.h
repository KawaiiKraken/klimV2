#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include "../phnt/phnt_windows.h"
#include "../phnt/phnt.h"
#include "windivertFunctions.h"
#include <vector>
#include <mutex>
#include "krekens_overlay.h"
#include <iostream>




class Helper {
public:
	static bool D2Active();
	static bool RunningAsAdmin();
	static void Exitapp(bool debug);
	static const wchar_t* GetFilename(const wchar_t* path);
};


#endif HELPERFUNCTIONS_H
