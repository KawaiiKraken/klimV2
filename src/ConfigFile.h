#pragma once

#include <fstream>
#include <json/json.h>
#include <spdlog/spdlog.h>
#include <tchar.h>
#include <windows.h>

namespace Klim
{
    class Limit;

    struct Settings
    {
            COLORREF color_default = RGB(255, 255, 255);
            COLORREF color_on = RGB(255, 0, 0);
            COLORREF color_off = RGB(255, 255, 255);
            int window_location = 0;
            int font_size = 18;
            bool change_text_color = true;
            bool show_limit_state = false;
            bool show_hotkey = true;
            bool show_overlay = true;
            bool show_timer = true;
            bool frosted_glass = false;
            bool debug = false;
            bool force_passthrough = false;
            bool show_console = false;
            bool always_on_top = false;
            bool use_custom_theme = true;
            int fps = 200;
    };


    class ConfigFile
    {
        public:
            static bool FileExists(LPCTSTR file_path);
            static Json::Value LoadConfigFileFromJson(wchar_t* filepath);
            static void StoreConfigToJson(const wchar_t* file_path, const Json::Value& config_data, std::shared_ptr<spdlog::logger> logger);
            static Json::Value VectorToJson(const std::vector<int>& vec);
            static std::vector<int> JsonToVector(const Json::Value& json_vec);
            static void WriteConfig(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], const Settings* settings, std::shared_ptr<spdlog::logger> logger);
            static void LoadConfig(std::vector<std::atomic<Limit>*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], Settings* settings);
            static void SetPathToFileInExeDir(const wchar_t* config_filename, wchar_t* path_to_config_file);
    };
}
