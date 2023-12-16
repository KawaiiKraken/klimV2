#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H
#pragma once

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



struct limit;

class Helper {
public:
	static bool D2Active();
	static bool RunningAsAdmin();
	static void Exitapp(bool debug);
	static const wchar_t* GetFilename(const wchar_t* path);
	static void TriggerHotkeys(std::vector<limit*> limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug, COLORREF color_on, COLORREF color_off, char combined_windivert_rules[1000]);
	static void UnTriggerHotkeys(std::vector<limit*> limit_ptr_vector, std::vector<int> currently_pressed_keys);
	static void SetOverlayLineNumberOfLimits(std::vector<limit*> limit_ptr_vector);
	static void InitializeOverlay(bool use_overlay, int font_size, COLORREF color_default, std::vector<limit*> limit_ptr_vector);
private:
	static int OnTriggerHotkey(limit* limit, bool debug, COLORREF color_on, COLORREF color_off, std::vector<struct limit*> limit_ptr_vector, char* combined_windivert_rules);
};


#endif HELPERFUNCTIONS_H
