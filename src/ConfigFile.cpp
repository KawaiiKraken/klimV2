#include "ConfigFile.h"

void ConfigFile::WriteConfig(std::vector<limit*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH]) {
    Json::Value config;

	char char_buffer[250];
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        if (limit_ptr_vector[i]->key_list[0] == 0x0) {
            continue;
        }
		size_t size;
		wcstombs_s(&size, char_buffer, limit_ptr_vector[i]->name, 50);
        strcat_s(char_buffer, sizeof(char_buffer), "_key_list");

        config[char_buffer] = ConfigFile::vectorToJson(limit_ptr_vector[i]->key_list);
    }

    // random defaults for now
    config["use_overlay"] = true;
    config["font_size"] = 30;
    config["color_default"] = "0x00FFFFFF";
    config["color_on"] = "0x000000FF";
    config["color_off"] = "0x00FFFFFF";

    ConfigFile::StoreConfigToJson(path_to_config_file, config);
}



void ConfigFile::LoadConfig( bool* use_overlay, int* font_size, COLORREF* color_default, COLORREF* color_on, COLORREF* color_off , std::vector<limit*> limit_ptr_vector, wchar_t path_to_config_file[MAX_PATH]){
    // Load the config from the JSON file
    Json::Value loaded_config = ConfigFile::LoadConfigFileFromJson( path_to_config_file );
	char char_buffer[250];
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
		size_t size;
		wcstombs_s(&size, char_buffer, limit_ptr_vector[i]->name, 50);
        strcat_s(char_buffer, sizeof(char_buffer), "_key_list");

        limit_ptr_vector[i]->key_list = ConfigFile::jsonToVector(loaded_config[char_buffer]);
        if (limit_ptr_vector[i]->key_list.size() == 0) {
            limit_ptr_vector[i]->key_list.push_back(undefined_key);
        }
    }

    *color_default = stol( loaded_config["color_default"].asString(), NULL, 16 );
    *color_on      = stol( loaded_config["color_on"].asString(),      NULL, 16 );
    *color_off     = stol( loaded_config["color_off"].asString(),     NULL, 16 );

    *use_overlay  = loaded_config["use_overlay"].asBool();
    *font_size    = loaded_config["font_size"].asInt();
}




bool ConfigFile::FileExists( LPCTSTR szPath ){
  DWORD dwAttrib = GetFileAttributes( szPath );
  return ( dwAttrib != INVALID_FILE_ATTRIBUTES && !( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) );
}

Json::Value ConfigFile::LoadConfigFileFromJson( wchar_t* filepath ){
    Json::Value json_data;
    std::ifstream config_file( filepath, std::ifstream::binary );

    if ( config_file.is_open() ){
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(config_file, json_data);

        if ( !parsingSuccessful ){
            std::cerr << "Error parsing JSON: " << reader.getFormattedErrorMessages() << std::endl;
        }
        config_file.close();
    } else {
        std::cerr << "Error opening file for reading" << std::endl;
    }

    return json_data;
}

// TODO make it overwrite the file
void ConfigFile::StoreConfigToJson(wchar_t* file_path, const Json::Value& config_data) {
    printf( "creating new config file\n" );
    HANDLE hFile = CreateFileW( ( LPCTSTR )file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE ){
        printf( "Error opening file for writing" );
        return;
    }

    std::string json_data = config_data.toStyledString();

    printf("writing config\n");
    DWORD bytes_written;
    WriteFile( hFile, json_data.c_str(), static_cast<DWORD>( json_data.length() ), &bytes_written, NULL );

    CloseHandle( hFile );
}

Json::Value ConfigFile::vectorToJson(const std::vector<int>& vec) {
    Json::Value jsonVec;

    for (const auto& element : vec) {
        jsonVec.append(element);
    }

    return jsonVec;
}

std::vector<int> ConfigFile::jsonToVector(const Json::Value& jsonVec) {
    std::vector<int> vec;

    for (const auto& element : jsonVec) {
        vec.push_back(element.asInt());
    }

    return vec;
}


void ConfigFile::SetPathToConfigFile( wchar_t* config_filename, wchar_t* path_to_config_file) {
    wchar_t file_path_self[MAX_PATH], folder_path_self[MAX_PATH];
    GetModuleFileName( NULL, file_path_self, MAX_PATH );
    wcsncpy_s( folder_path_self, MAX_PATH, file_path_self, ( wcslen( file_path_self ) - wcslen( Helper::GetFilename( file_path_self ) ) ) );
    wchar_t filename[MAX_PATH], file_path[MAX_PATH];
    wcscpy_s( filename, MAX_PATH, config_filename );
    wcscpy_s( file_path, MAX_PATH, folder_path_self );
    wcscat_s( file_path, MAX_PATH, filename );
    wcsncpy_s( path_to_config_file, MAX_PATH, file_path, MAX_PATH );
}
