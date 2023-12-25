#ifndef CONFIGFILE_H
#define CONFIGFILE_H
#pragma once

#include "helperFunctions.h"
#include <windows.h>
#include <tchar.h>
#include "jsoncpp_header\json.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "Limit.h"
#include "jsoncpp_header/json.h"

struct Settings {
	bool use_overlay;
	int font_size;
	COLORREF color_default;
	COLORREF color_on;
	COLORREF color_off;
};


class ConfigFile {
public:
	static bool FileExists(LPCTSTR szPath);
	static Json::Value LoadConfigFileFromJson(wchar_t* filepath);
	static void StoreConfigToJson(wchar_t* file_path, const Json::Value& config_data);
	static Json::Value vectorToJson(const std::vector<int>& vec);
	static std::vector<int> jsonToVector(const Json::Value& jsonVec);
	static void WriteConfig(std::vector<limit*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], Settings* settings);
	static void LoadConfig(std::vector<limit*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], Settings* settings);
	static void SetPathToConfigFile(wchar_t* config_filename, wchar_t* path_to_config_file);
private:
	static void ColorRefToHex(COLORREF colorRef, char hexString[8]);
};

#endif CONFIGFILE_H
