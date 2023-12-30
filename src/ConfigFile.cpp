#include "ConfigFile.h"
#include "Limit.h"
#include "HelperFunctions.h"

void ConfigFile::WriteConfig(std::vector<std::atomic<Limit>*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], Settings* settings)
{
    Json::Value config;

    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        Limit temp_limit = limit_ptr_vector[i]->load();
        if (temp_limit.key_list[0] == 0) {
            continue;
        }
        std::string name = temp_limit.name;
        name.append("_key_list");

        std::vector<int> key_list;
        for (int j = 0; j < temp_limit.max_key_list_size; j++) {
            if (temp_limit.key_list[j] == 0)
                continue;
            key_list.push_back(temp_limit.key_list[j]);
        }
        config[name] = ConfigFile::vectorToJson(key_list);
    }

    // random defaults for now
    config["use_overlay"]   = settings->use_overlay;
    config["font_size"]     = settings->font_size;
    config["color_default"] = "0x00FFFFFF";
    config["color_on"]      = "0x000000FF";
    config["color_off"]     = "0x00FFFFFF";

    ConfigFile::StoreConfigToJson(path_to_config_file, config);
}


void ConfigFile::LoadConfig(std::vector<std::atomic<Limit>*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH], Settings* settings)
{
    // Load the config from the JSON file
    Json::Value loaded_config = ConfigFile::LoadConfigFileFromJson(path_to_config_file);
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        std::string name = limit_ptr_vector[i]->load().name;
        name.append("_key_list");

        Limit temp_limit = limit_ptr_vector[i]->load();
        std::vector<int> key_vector = ConfigFile::jsonToVector(loaded_config[name.c_str()]);
        // temp_limit.key_list;
        for (int j = 0; j < key_vector.size(); j++) {
            temp_limit.key_list[j] = key_vector[j];
        }
        limit_ptr_vector[i]->store(temp_limit);
    }

    settings->color_default = stol(loaded_config["color_default"].asString(), NULL, 16);
    settings->color_on      = stol(loaded_config["color_on"].asString(), NULL, 16);
    settings->color_off     = stol(loaded_config["color_off"].asString(), NULL, 16);

    settings->use_overlay = loaded_config["use_overlay"].asBool();
    settings->font_size   = loaded_config["font_size"].asInt();
}


bool ConfigFile::FileExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

Json::Value ConfigFile::LoadConfigFileFromJson(wchar_t* filepath)
{
    Json::Value json_data;
    std::ifstream config_file(filepath, std::ifstream::binary);

    if (config_file.is_open()) {
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(config_file, json_data);

        if (!parsingSuccessful) {
            std::cerr << "Error parsing JSON: " << reader.getFormattedErrorMessages() << std::endl;
        }
        config_file.close();
    } else {
        std::cerr << "Error opening file for reading" << std::endl;
        MessageBoxA(NULL, "Save a hotkey first", NULL, MB_OK);
        PostQuitMessage(0);
    }

    return json_data;
}

// TODO make it overwrite the file
void ConfigFile::StoreConfigToJson(wchar_t* file_path, const Json::Value& config_data)
{
    std::cout << "creating new config file" << std::endl;
    HANDLE hFile
        = CreateFileW(( LPCTSTR )file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening file for writing" << std::endl;
        return;
    }

    std::string json_data = config_data.toStyledString();

    std::cout << "writing config" << std::endl;
    DWORD bytes_written;
    WriteFile(hFile, json_data.c_str(), static_cast<DWORD>(json_data.length()), &bytes_written, NULL);

    CloseHandle(hFile);
}

Json::Value ConfigFile::vectorToJson(const std::vector<int>& vec)
{
    Json::Value jsonVec;

    for (const auto& element : vec) {
        jsonVec.append(element);
    }

    return jsonVec;
}

std::vector<int> ConfigFile::jsonToVector(const Json::Value& jsonVec)
{
    std::vector<int> vec;

    for (const auto& element : jsonVec) {
        vec.push_back(element.asInt());
    }

    return vec;
}


void ConfigFile::SetPathToConfigFile(wchar_t* config_filename, wchar_t* path_to_config_file)
{
    wchar_t file_path_self[MAX_PATH], folder_path_self[MAX_PATH];
    GetModuleFileName(NULL, file_path_self, MAX_PATH);
    wcsncpy_s(folder_path_self, MAX_PATH, file_path_self, (wcslen(file_path_self) - wcslen(Helper::GetFilename(file_path_self))));
    wchar_t filename[MAX_PATH], file_path[MAX_PATH];
    wcscpy_s(filename, MAX_PATH, config_filename);
    wcscpy_s(file_path, MAX_PATH, folder_path_self);
    wcscat_s(file_path, MAX_PATH, filename);
    wcsncpy_s(path_to_config_file, MAX_PATH, file_path, MAX_PATH);
}
