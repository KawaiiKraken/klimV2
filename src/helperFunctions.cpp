#include "helperFunctions.h"
#include <algorithm>
#include "Limit.h"
#include "ConfigFile.h"

void Helper::Exitapp(bool debug){
    std::wcout << "shutting down\n";
    ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
    if ( !debug ){
		ShowWindow( GetConsoleWindow(), SW_RESTORE );
		}
	PostQuitMessage( 0 );
}

void Helper::TriggerHotkeys( std::vector<std::atomic<limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug, Settings settings, char combined_windivert_rules[1000]) {
    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
        if (limit_ptr_vector[i]->load().key_list[0] == 0) {
            continue;
        }
          // Sort both vectors
        limit temp_limit = limit_ptr_vector[i]->load();
        std::vector<int> key_list;
        for (int i = 0; i < temp_limit.max_key_list_size; i++) {
            if (temp_limit.key_list[i] != 0) {
                key_list.push_back(temp_limit.key_list[i]);
            }
        }
        std::sort(key_list.begin(), key_list.end());
		std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), key_list.begin(), key_list.end());

        if (containsAll) {
	        Helper::OnTriggerHotkey(limit_ptr_vector[i], debug, settings, limit_ptr_vector, combined_windivert_rules);
        }
    }
}


void Helper::UnTriggerHotkeys( std::vector<std::atomic<limit>*> limit_ptr_vector, std::vector<int> currently_pressed_keys) {
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        // Sort both vectors
        limit temp_limit = limit_ptr_vector[i]->load();
        std::vector<int> key_list;
        for (int i = 0; i < temp_limit.max_key_list_size; i++) {
            if (temp_limit.key_list[i] != 0) {
                key_list.push_back(temp_limit.key_list[i]);
            }
        }
        std::sort(key_list.begin(), key_list.end());
        std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), key_list.begin(), key_list.end());

        if (!containsAll) {
            temp_limit.triggered = false;
            limit_ptr_vector[i]->store(temp_limit);
        }
    }
}

void Helper::SetOverlayLineNumberOfLimits( std::vector<std::atomic<limit>*> limit_ptr_vector){
    int current_overlay_line = 1;
    for ( int i = 0; i < limit_ptr_vector.size(); i++) {
        if (limit_ptr_vector[i]->load().key_list[0] != 0) {
            limit temp_limit = limit_ptr_vector[i]->load();
			temp_limit.overlay_line_number = current_overlay_line;
            limit_ptr_vector[i]->store(temp_limit);
			current_overlay_line++;
		}
    }
}



bool Helper::D2Active(){
    TCHAR buffer[MAX_PATH] = {0};
    DWORD dw_proc_id = 0; 

    GetWindowThreadProcessId( GetForegroundWindow(), &dw_proc_id );
    
    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, dw_proc_id );
    GetModuleFileNameEx( hProc, NULL, buffer, MAX_PATH );
    CloseHandle( hProc );

    const wchar_t* filename = GetFilename( buffer );
    printf( "active window filename: %ls\n", filename );

    if ( wcscmp( filename, L"destiny2.exe" ) == 0 ){
        return true;
    } else {
        return false;
    }
}




bool Helper::RunningAsAdmin(){
    bool fRet = false;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) ){
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &elevation, sizeof( elevation ), &cbSize ) ){
            fRet = elevation.TokenIsElevated;
        }
    }
    if( hToken ){
        CloseHandle( hToken );
    }
    return fRet;
}



const wchar_t* Helper::GetFilename( const wchar_t *path ){
    const wchar_t *filename = wcsrchr( path, '\\' );
    if ( filename == NULL )
        filename = path;
    else
        filename++;
    return filename;
}

void Helper::InitializeOverlay( Settings settings, std::vector<limit*> limit_ptr_vector){
    //startOverlay( settings.use_overlay, settings.font_size );

    // set overlay to default state
    wchar_t* wc_string = new wchar_t[200];
    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
        if (limit_ptr_vector[i]->overlay_line_number != -1) {
            //Limit::FormatHotkeyStatusWcString(wc_string, 200, limit_ptr_vector[i]);
            //UpdateOverlayLine(wc_string, limit_ptr_vector[i]->overlay_line_number, settings.color_default);
        }
    }
    delete []wc_string;
}

int Helper::OnTriggerHotkey( std::atomic<limit>* limitarg, bool debug, Settings settings, std::vector<std::atomic<struct limit>*> limit_ptr_vector, char* combined_windivert_rules) {
    if ( strcmp(limitarg->load().name, "exitapp") == 0 ){
        Helper::Exitapp(debug);
	} 
    if ( !( Helper::D2Active() || debug ) )
    {
        printf("hotkey ignored: d2 is not the active window and debug mode is not on");
        return 1;
    }
    if ( !limitarg->load().triggered ) {
        limit limit = limitarg->load();
		limit.triggered = true;
        limitarg->store(limit);
        if ( strcmp(limitarg->load().name, "game") == 0){
			Limit::ToggleWholeGameLimit( limitarg, settings);
		} 
        else if ( strcmp(limitarg->load().name, "suspend") == 0) {
			Limit::ToggleSuspend( limitarg, settings);
		} 
        else {
            limit = limitarg->load();
            limit.state = !limit.state;
            limitarg->store(limit);
        }
        printf( "state of %ws: %s\n", limitarg->load().name, limitarg->load().state ? "true" : "false" );
        SetFilterRuleString( limit_ptr_vector, combined_windivert_rules );
        UpdateFilter( combined_windivert_rules );
    }
    return 0;
}


