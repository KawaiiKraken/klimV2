#include "ConfigFile.h"
#include "HelperFunctions.h"
#include "Limit.h"


#include <iostream>

namespace Klim
{
    void ConfigFile::WriteConfig(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], const Settings* settings)
    {
        Json::Value config;

        for (const std::atomic<Limit>* limit_ptr : limit_ptr_vector)
        {
            const Limit temp_limit = limit_ptr->load();
            if (temp_limit.key_list[0] == 0)
            {
                continue;
            }

            std::string name = temp_limit.name;
            name.append("_key_list");

            std::vector<int> key_list;
            for (int j = 0; j < temp_limit.max_key_list_size; j++)
            {
                if (temp_limit.key_list[j] == 0)
                {
                    continue;
                }
                key_list.push_back(temp_limit.key_list[j]);
            }
            config[name] = VectorToJson(key_list);
        }

        config["color_default"] = "0x00FFFFFF";
        config["color_on"] = "0x000000FF";
        config["color_off"] = "0x00FFFFFF";

        config["window_location"] = settings->window_location;
        config["font_size"] = settings->font_size;

        config["change_text_color"] = settings->change_text_color;
        config["show_limit_state"] = settings->show_limit_state;
        config["show_hotkey"] = settings->show_hotkey;
        config["show_overlay"] = settings->show_overlay;
        config["show_timer"] = settings->show_timer;
        config["frosted_glass"] = settings->frosted_glass;
        config["debug"] = settings->debug;

        StoreConfigToJson(path_to_config_file, config);
    }

    void ConfigFile::LoadConfig(std::vector<std::atomic<Limit>*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], Settings* settings)
    {
        // Load the config from the JSON file
        Json::Value loaded_config = LoadConfigFileFromJson(path_to_config_file);
        for (std::atomic<Limit>*& limit_ptr : limit_ptr_vector)
        {
            std::string name = limit_ptr->load().name;
            name.append("_key_list");

            Limit temp_limit = limit_ptr->load();
            std::vector<int> key_vector = JsonToVector(loaded_config[name.c_str()]);

            for (size_t j = 0; j < key_vector.size(); j++)
            {
                temp_limit.key_list[j] = key_vector[j];
            }
            limit_ptr->store(temp_limit);
        }

        settings->color_default = stol(loaded_config["color_default"].asString(), nullptr, 16);
        settings->color_on = stol(loaded_config["color_on"].asString(), nullptr, 16);
        settings->color_off = stol(loaded_config["color_off"].asString(), nullptr, 16);

        settings->window_location = loaded_config["window_location"].asInt();
        settings->font_size = loaded_config["font_size"].asInt();

        settings->change_text_color = loaded_config["change_text_color"].asBool();
        settings->show_limit_state = loaded_config["show_limit_state"].asBool();
        settings->show_hotkey = loaded_config["show_hotkey"].asBool();
        settings->show_overlay = loaded_config["show_overlay"].asBool();
        settings->show_timer = loaded_config["show_timer"].asBool();
        settings->frosted_glass = loaded_config["frosted_glass"].asBool();
        settings->debug = loaded_config["debug"].asBool();
    }

    bool ConfigFile::FileExists(const LPCTSTR file_path)
    {
        const DWORD dw_attrib = GetFileAttributes(file_path);
        return dw_attrib != INVALID_FILE_ATTRIBUTES && !(dw_attrib & FILE_ATTRIBUTE_DIRECTORY);
    }

    Json::Value ConfigFile::LoadConfigFileFromJson(wchar_t* filepath)
    {
        Json::Value json_data;
        std::ifstream config_file(filepath, std::ifstream::binary);

        if (config_file.is_open())
        {
            Json::Reader reader;
            bool parsing_successful = reader.parse(config_file, json_data);

            if (!parsing_successful)
            {
                std::cerr << "Error parsing JSON: " << reader.getFormattedErrorMessages() << "\n";
            }
            config_file.close();
        }
        else
        {
            std::cerr << "Error opening file for reading\n";
            MessageBoxA(nullptr, "Save a hotkey first", nullptr, MB_OK);
            PostQuitMessage(0);
        }

        return json_data;
    }

    // TODO make it overwrite the file
    void ConfigFile::StoreConfigToJson(const wchar_t* file_path, const Json::Value& config_data)
    {
        std::cout << "creating new config file\n";
        const HANDLE file_handle = CreateFileW(file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (file_handle == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Error opening file for writing\n";
            return;
        }

        const std::string json_data = config_data.toStyledString();

        std::cout << "writing config\n";
        DWORD bytes_written;
        WriteFile(file_handle, json_data.c_str(), static_cast<DWORD>(json_data.length()), &bytes_written, nullptr);

        CloseHandle(file_handle);
    }

    Json::Value ConfigFile::VectorToJson(const std::vector<int>& vec)
    {
        Json::Value json_vec;

        for (const int& element : vec)
        {
            json_vec.append(element);
        }

        return json_vec;
    }

    std::vector<int> ConfigFile::JsonToVector(const Json::Value& json_vec)
    {
        std::vector<int> vec;

        for (const Json::Value& element : json_vec)
        {
            vec.push_back(element.asInt());
        }

        return vec;
    }

    void ConfigFile::SetPathToConfigFile(const wchar_t* config_filename, wchar_t* path_to_config_file)
    {
        wchar_t file_path_self[MAX_PATH];
        wchar_t folder_path_self[MAX_PATH];

        GetModuleFileName(nullptr, file_path_self, MAX_PATH);
        wcsncpy_s(folder_path_self, MAX_PATH, file_path_self, wcslen(file_path_self) - wcslen(Helper::GetFileName(file_path_self)));
        wchar_t filename[MAX_PATH], file_path[MAX_PATH];
        wcscpy_s(filename, MAX_PATH, config_filename);
        wcscpy_s(file_path, MAX_PATH, folder_path_self);
        wcscat_s(file_path, MAX_PATH, filename);
        wcsncpy_s(path_to_config_file, MAX_PATH, file_path, MAX_PATH);
    }
}
