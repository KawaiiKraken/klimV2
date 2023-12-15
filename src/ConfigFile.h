#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "helperFunctions.h"
#include <windows.h>
#include <tchar.h>
#include "jsoncpp_header\json.h"
#include <fstream>
#include <iostream>
#include <vector>


class ConfigFile {
public:
	static bool FileExists(LPCTSTR szPath);
	static Json::Value LoadConfigFileFromJson(wchar_t* filepath);
	static void StoreConfigToJson(wchar_t* file_path, const Json::Value& config_data);
	static Json::Value vectorToJson(const std::vector<int>& vec);
	static std::vector<int> jsonToVector(const Json::Value& jsonVec);
	static void WriteConfig(std::vector<limit*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH]);
	static void LoadConfig(bool* use_overlay, int* font_size, COLORREF* color_default, COLORREF* color_on, COLORREF* color_off, std::vector<limit*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH]);
};

#endif CONFIGFILE_H
