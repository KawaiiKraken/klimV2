#pragma once

#include "jsoncpp_header/json.h"
#include <fstream>
#include <tchar.h>
#include <windows.h>

namespace Klim
{
    class Limit;

    class Settings
    {
        public:
            bool use_overlay;
            int font_size;
            COLORREF color_default;
            COLORREF color_on;
            COLORREF color_off;
    };


    class ConfigFile
    {
        public:
            static bool FileExists(LPCTSTR file_path);
            static Json::Value LoadConfigFileFromJson(wchar_t* filepath);
            static void StoreConfigToJson(const wchar_t* file_path, const Json::Value& config_data);
            static Json::Value VectorToJson(const std::vector<int>& vec);
            static std::vector<int> JsonToVector(const Json::Value& json_vec);
            static void WriteConfig(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], const Settings* settings);
            static void LoadConfig(std::vector<std::atomic<Limit>*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], Settings* settings);
            static void SetPathToConfigFile(const wchar_t* config_filename, wchar_t* path_to_config_file);
    };
}
