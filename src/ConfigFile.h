#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <windows.h>
#include <tchar.h>
#include "jsoncpp_header\json.h"
#include <fstream>
#include <iostream>


class ConfigFile {
public:
	static bool FileExists(LPCTSTR szPath);
	static Json::Value LoadConfigFileFromJson(wchar_t* filepath);
	static void StoreConfigToJson(wchar_t* file_path, const Json::Value& config_data);
	static Json::Value vectorToJson(const std::vector<int>& vec);
	static std::vector<int> jsonToVector(const Json::Value& jsonVec);
	static const wchar_t* GetFilename(const wchar_t* path);
};

#endif CONFIGFILE_H
