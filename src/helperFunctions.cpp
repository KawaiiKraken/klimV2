#include "helperFunctions.h"
#include <algorithm>
#include "Limit.h"

void Helper::Exitapp(bool debug){
    std::wcout << "shutting down\n";
    if ( !debug ){
		ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
		ShowWindow( GetConsoleWindow(), SW_RESTORE );
		}
	PostQuitMessage( 0 );
}

void Helper::TriggerHotkeys( std::vector<limit*> limit_ptr_vector, std::vector<int> currently_pressed_keys, bool debug, COLORREF color_on, COLORREF color_off, char combined_windivert_rules[1000]) {
    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
          // Sort both vectors
        std::sort(limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());
		std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());

        if (containsAll) {
	        Helper::OnTriggerHotkey(limit_ptr_vector[i], debug, color_on, color_off, limit_ptr_vector, combined_windivert_rules);
        }
    }
}


void Helper::UnTriggerHotkeys( std::vector<limit*> limit_ptr_vector, std::vector<int> currently_pressed_keys) {
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        // Sort both vectors
        std::sort(limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());
        std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), limit_ptr_vector[i]->key_list.begin(), limit_ptr_vector[i]->key_list.end());

        if (!containsAll) {
            limit_ptr_vector[i]->triggered = false;
        }
    }
}

void Helper::SetOverlayLineNumberOfLimits( std::vector<limit*> limit_ptr_vector){
    int current_overlay_line = 1;
    for ( int i = 0; i < limit_ptr_vector.size(); i++) {
        if (limit_ptr_vector[i]->key_list[0] != undefined_key) {
			limit_ptr_vector[i]->overlay_line_number = current_overlay_line;
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

void Helper::InitializeOverlay( bool use_overlay, int font_size, COLORREF color_default, std::vector<limit*> limit_ptr_vector){
    startOverlay( use_overlay, font_size );

    // set overlay to default state
    wchar_t* wc_string = new wchar_t[200];
    for ( int i = 0; i < limit_ptr_vector.size(); i++ ){
        if (limit_ptr_vector[i]->overlay_line_number != -1) {
            Limit::FormatHotkeyStatusWcString(wc_string, 200, limit_ptr_vector[i]);
            UpdateOverlayLine(wc_string, limit_ptr_vector[i]->overlay_line_number, color_default);
        }
    }
    delete []wc_string;
}

int Helper::OnTriggerHotkey( limit* limit, bool debug, COLORREF color_on, COLORREF color_off, std::vector<struct limit*> limit_ptr_vector, char* combined_windivert_rules) {
    if ( wcscmp( limit->name, L"exitapp" ) == 0 ){
        Helper::Exitapp(debug);
	} 
    if ( !( Helper::D2Active() || debug ) )
    {
        printf("hotkey ignored: d2 is not the active window and debug mode is not on");
        return 1;
    }
    if ( !limit->triggered ) {
		limit->triggered = true;
        if ( wcscmp( limit->name, L"game" ) == 0){
			Limit::ToggleWholeGameLimit( limit, color_on, color_off );
		} 
        else if ( wcscmp( limit->name, L"suspend") == 0 ){
			Limit::ToggleSuspend( limit, color_on, color_off );
		} 
        else {
			Limit::ToggleBlockingLimit( limit, color_on, color_off );
        }
        printf( "state of %ws: %s\n", limit->name, limit->state ? "true" : "false" );
        SetFilterRuleString( limit_ptr_vector, combined_windivert_rules );
        UpdateFilter( combined_windivert_rules );
    }
    return 0;
}


