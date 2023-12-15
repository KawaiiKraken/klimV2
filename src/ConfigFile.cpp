#include "ConfigFile.h"

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
    //if ( FileExists( file_path ) ){
        //return;
    //}
    printf( "creating new config file\n" );
    HANDLE hFile = CreateFileW( ( LPCTSTR )file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE ){
        printf( "Error opening file for writing" );
        return;
    }

    std::string json_data = config_data.toStyledString();

    printf("setting config to default\n");
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

const wchar_t* ConfigFile::GetFilename( const wchar_t *path ){
    const wchar_t *filename = wcsrchr( path, '\\' );
    if ( filename == NULL )
        filename = path;
    else
        filename++;
    return filename;
}


