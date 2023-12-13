// TODO comments
#include "main.h"
#include "helperFunctions.h"
#include "krekens_overlay.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <future>
#include <chrono>
#include <condition_variable>

limit lim_3074(    ( wchar_t* )L"3074" ); 
limit lim_3074_ul( ( wchar_t* )L"3074UL" );
limit lim_27k(     ( wchar_t* )L"27k" ); 
limit lim_27k_ul(  ( wchar_t* )L"27kUL" ); 
limit lim_30k(     ( wchar_t* )L"30k" ); 
limit lim_7k(      ( wchar_t* )L"7k" ); 
limit lim_game(    ( wchar_t* )L"game" ); 
limit suspend(     ( wchar_t* )L"suspend" ); 
limit exitapp(     ( wchar_t* )L"exitapp" ); 
limit* limit_ptr_array[] = { &lim_3074, &lim_3074_ul, &lim_27k, &lim_27k_ul, &lim_30k, &lim_7k, &lim_game, &suspend, &exitapp };
int size_of_limit_ptr_array = sizeof(limit_ptr_array) / sizeof(limit_ptr_array[0]);

// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Data
static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;

// Forward declarations of helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef void (*KeyboardEventCallback)(int, bool);


class HotkeyManager {
public:
    void asyncBindHotkey(int i) {
        // TODO fix limit being unreadable due to access violation
            if (_bindingInProgress == true || limit_ptr_array[i]->bindingComplete == false) {
                MessageBox(NULL, (wchar_t*)L"error hotkey is already being bound...", NULL, MB_OK);
                return;
            }
            limit_ptr_array[i]->bindingComplete = false;

            _bindingInProgress = true;
            _currentHotkeyList.clear();

            while (_bindingInProgress == true) {
                Sleep(10);
            }
            std::cout << "current size: " << _currentHotkeyList.size() << std::endl;
            limit_ptr_array[i]->key_list = _currentHotkeyList;
            limit_ptr_array[i]->bindingComplete = true;
            return;
	}

	void KeyboardInputHandler(int key, bool isKeyDown)
	{
		if (isKeyDown)
		{
			std::cout << "Key Down: " << key << std::endl;
            auto it = std::find(_currentHotkeyList.begin(), _currentHotkeyList.end(), key);
            if (it == _currentHotkeyList.end() && _bindingInProgress == true) {
                _currentHotkeyList.push_back(key);
			}
		} else
		{
			std::cout << "Key Up: " << key << std::endl;
            auto it = std::find(_currentHotkeyList.begin(), _currentHotkeyList.end(), key);
            _bindingInProgress = false;
		}
	}
private:
    bool _bindingInProgress = false;

    std::vector<int> _currentHotkeyList;
};

HotkeyManager hotkeyManager;




char combined_windivert_rules[1000];
int OnTriggerHotkey( limit* limit );
wchar_t path_to_config_file[MAX_PATH];
void LoadConfig( bool* useOverlay, int* fontSize, limit* limit_array[], int array_size );
static void SetOverlayLineNumberOfHotkeys( limit* limit_array[], int array_size );
void SetFilterRuleString( limit* limit_array[], int array_size );
void InitializeOverlay( bool useOverlay, int fontSize, limit* limit_array[], int array_size );


// TODO make exitapp work when d2 is not the active window

int run_gui(){
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui Win32+OpenGL3 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize OpenGL
    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    std::vector<bool> button_clicked(size_of_limit_ptr_array);
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            bool use_work_area = true; // fullscreen
            static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

            ImGui::Begin("config", NULL, flags);

            {
                for (int i = 0; i < size_of_limit_ptr_array; i++) {
                    std::lock_guard<std::mutex> lock(limit_ptr_array[i]->mutex);
                    ImGui::PushID(i);
                    if (ImGui::Button("Click to bind")) {
                        button_clicked[i] = true;
                        limit_ptr_array[i]->String = "in progress..";
                        limit_ptr_array[i]->bindingComplete = false;
                    }
                    else if (limit_ptr_array[i]->bindingComplete == true && limit_ptr_array[i]->String == "in progress..") {
                        limit_ptr_array[i]->String = "";
                        for (int i = 0; i < limit_ptr_array[i]->key_list.size(); i++) {
                            limit_ptr_array[i]->String += std::to_string(limit_ptr_array[i]->key_list[i]);
                            limit_ptr_array[i]->String.append(", ");
                        }
                    }


                    ImGui::SameLine();
					char name[50];
					size_t size;
					wcstombs_s(&size, name, limit_ptr_array[i]->name, 50);
					ImGui::Text(name);               // Display some text (you can use a format strings too)
					ImGui::SameLine();
					ImGui::Text(limit_ptr_array[i]->String.data());               // Display some text (you can use a format strings too)
                    ImGui::PopID();
                }
            }

            ImGui::End();
        }


        // Rendering
        ImGui::Render();

        for (int i = 0; i < (button_clicked.size()-1); i++) {
            if (button_clicked[i] == true) {
                std::cout << "button_clicked.size = " << button_clicked.size() << std::endl;
                button_clicked[i] = false;
                //std::string s = std::to_string(button_clicked.size());
                //char const* const_char = s.c_str();
                //MessageBoxA(NULL, const_char, (char*)"debug", MB_OK);
                std::thread([&]() {
                    hotkeyManager.asyncBindHotkey(i);
                    }).detach();
            }
        }



        glViewport(0, 0, g_Width, g_Height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Present
        ::SwapBuffers(g_MainWindow.hDC);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL(hwnd, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}



int __cdecl main( int argc, char** argv ){
    run_gui();
    
//    strcpy_s( lim_3074.windivert_rule,    sizeof( lim_3074.windivert_rule ),    " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)" ); 
//	strcpy_s( lim_3074_ul.windivert_rule, sizeof( lim_3074_ul.windivert_rule ), " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)" ); 
//	strcpy_s( lim_27k.windivert_rule,     sizeof( lim_27k.windivert_rule ),     " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)" ); 
//	strcpy_s( lim_27k_ul.windivert_rule,  sizeof( lim_27k_ul.windivert_rule ),  " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)" ); 
//	strcpy_s( lim_30k.windivert_rule,     sizeof( lim_30k.windivert_rule ),     " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)" ); 
//	strcpy_s( lim_7k.windivert_rule,      sizeof( lim_7k.windivert_rule ),      " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)" ); 
//
//    if ( argv[1] != NULL ){
//        if ( ( strcmp( argv[1], "--debug" ) == 0 ) ){
//            printf( "debug: true\n" );
//            debug = true;
//        } else {
//            printf( "error: invalid argument...\n" );
//            printf( "options:\n" );
//            printf( "    --debug     prevents console hiding and enables hotkey trigger outside of destiny 2.\n" );
//            return 0;
//        }
//    } else {
//        ShowWindow( GetConsoleWindow(), SW_HIDE );
//    }
//
//    if ( !RunningAsAdmin() ){
//        MessageBox( NULL, ( LPCWSTR )L"ERROR: not running as admin", ( LPCWSTR )L"ERROR", MB_ICONERROR | MB_DEFBUTTON2 );
//        return 0;
//    }
//
//    
//    SetPathToConfigFile( ( wchar_t* )L"config.txt" );
//    if ( !FileExists( path_to_config_file ) ){
//        WriteDefaultJsonConfig( path_to_config_file );
//    }
//
//    bool use_overlay;
//	int font_size;
//    LoadConfig( &use_overlay, &font_size, limit_ptr_array, size_of_limit_ptr_array );
//    SetOverlayLineNumberOfHotkeys( limit_ptr_array, size_of_limit_ptr_array );
//    InitializeOverlay( use_overlay, font_size, limit_ptr_array, size_of_limit_ptr_array );
//
//
//    printf( "starting hotkey thread\n" );
//
//    // TODO remove dwThread
//    DWORD dwThread;
//    hHotkeyThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )HotkeyThread, ( LPVOID )NULL, NULL, &dwThread );
//
//    if ( hHotkeyThread ){
//        return WaitForSingleObject( hHotkeyThread, INFINITE );
//    }
//    else {
//        return 1;
//    }
//    CloseHandle( hHotkeyThread );
//    FreeLibrary( hDLL );
    return 0;
}


// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
    return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(NULL, NULL);
    ::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            g_Width = LOWORD(lParam);
            g_Height = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        hotkeyManager.KeyboardInputHandler(static_cast<int>(wParam), true);
        break;
    case WM_KEYUP:
        hotkeyManager.KeyboardInputHandler(static_cast<int>(wParam), false);
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}



static void SetOverlayLineNumberOfHotkeys( limit* limit_ptr_array[], int size_of_limit_ptr_array ){
    int current_overlay_line = 1;
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        bool valid_hotkey = ( limit_ptr_array[i]->hotkey != undefined_key );
        bool valid_modkey = ( limit_ptr_array[i]->modkey != undefined_key );
        if ( valid_hotkey || valid_modkey ){
            limit_ptr_array[i]->overlay_line_number = current_overlay_line;
            current_overlay_line++;
        }
    }
}



void LoadConfig( bool* use_overlay, int* font_size, limit* limit_ptr_array[], int size_of_limit_ptr_array ){
    // Load the config from the JSON file
    Json::Value loaded_config = LoadConfigFileFromJson( path_to_config_file );
    // this could definitely be done programmatically but i think that would be more effort than its worth
    SetVarFromJson( limit_ptr_array[0], loaded_config["hotkey_3074"].asString(),    loaded_config["modkey_3074"].asString() );
    SetVarFromJson( limit_ptr_array[1], loaded_config["hotkey_3074_ul"].asString(), loaded_config["modkey_3074_ul"].asString() );
    SetVarFromJson( limit_ptr_array[2], loaded_config["hotkey_27k"].asString(),     loaded_config["modkey_27k"].asString() );
    SetVarFromJson( limit_ptr_array[3], loaded_config["hotkey_27k_ul"].asString(),  loaded_config["modkey_27k_ul"].asString() );
    SetVarFromJson( limit_ptr_array[4], loaded_config["hotkey_30k"].asString(),     loaded_config["modkey_30k"].asString() );
    SetVarFromJson( limit_ptr_array[5], loaded_config["hotkey_7k"].asString(),      loaded_config["modkey_7k"].asString() );
    SetVarFromJson( limit_ptr_array[6], loaded_config["hotkey_game"].asString(),    loaded_config["modkey_game"].asString() );
    SetVarFromJson( limit_ptr_array[7], loaded_config["hotkey_suspend"].asString(), loaded_config["modkey_suspend"].asString() );
    SetVarFromJson( limit_ptr_array[8], loaded_config["hotkey_exitapp"].asString(), loaded_config["modkey_exitapp"].asString() );

    color_default = stol( loaded_config["color_default"].asString(), NULL, 16 );
    color_on      = stol( loaded_config["color_on"].asString(),      NULL, 16 );
    color_off     = stol( loaded_config["color_off"].asString(),     NULL, 16 );

    *use_overlay  = loaded_config["use_overlay"].asBool();
    *font_size    = loaded_config["font_size"].asInt();
}



void InitializeOverlay( bool use_overlay, int font_size, limit* limit_ptr_array[], int size_of_limit_ptr_array ){
    startOverlay( use_overlay, font_size );

    // set overlay to default state
    wchar_t* wc_string = new wchar_t[200];
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        FormatHotkeyStatusWcString( wc_string, 200, limit_ptr_array[i] );
        UpdateOverlayLine( wc_string, limit_ptr_array[i]->overlay_line_number, color_default );
    }
    delete []wc_string;
}



void HotkeyResetKeyDownState( int key ){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        if ( key == (int)limit_ptr_array[i]->hotkey ){
            limit_ptr_array[i]->hotkey_down= false;
        }
    }
}



void ModkeyResetKeyDownState(int key){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        if ( key == limit_ptr_array[i]->modkey ){
            limit_ptr_array[i]->modkey_down = false;
        }
    }
}



void ModkeySetDownState( int key ){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
        if ( key == limit_ptr_array[i]->modkey || limit_ptr_array[i]->modkey == undefined_key ){
            limit_ptr_array[i]->modkey_down = true;
        }
    }
}



void TriggerHotkeys( int key ){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){ 
        if ( limit_ptr_array[i]->modkey_down == true && key == limit_ptr_array[i]->hotkey ){
            OnTriggerHotkey( limit_ptr_array[i] );
        }
    }
}



__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam ){
    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYUP ) || ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =  *( ( KBDLLHOOKSTRUCT* )lParam );

        int key = hooked_key.vkCode;

        ModkeyResetKeyDownState( key );
        HotkeyResetKeyDownState( key );
    }


    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = hooked_key.vkCode;

        ModkeySetDownState( key );
        TriggerHotkeys( key );
    }
    return CallNextHookEx( hKeyboardHook, nCode, wParam, lParam );
}



void Exitapp(){
    std::wcout << "shutting down\n";
    if ( !debug ){
		ShellExecute( NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE );
		ShowWindow( GetConsoleWindow(), SW_RESTORE );
		}
	PostQuitMessage( 0 );
}



int OnTriggerHotkey( limit* limit ){
    if ( !( D2Active() || debug ) )
    {
        printf("hotkey ignored: d2 is not the active window and debug mode is not on");
        return 1;
    }
    if ( !limit->hotkey_down ) {
		limit->hotkey_down = true;
		if ( wcscmp( limit->name, L"exitapp" ) == 0 ){
            Exitapp();
		} 
        else if ( wcscmp( limit->name, L"game" ) == 0){
			ToggleWholeGameLimit( limit, color_on, color_off );
		} 
        else if ( wcscmp( limit->name, L"suspend") == 0 ){
			ToggleSuspend( limit, color_on, color_off );
		} 
        else {
			ToggleBlockingLimit( limit, color_on, color_off );
        }
        printf( "state of %ws: %s\n", limit->name, limit->state ? "true" : "false" );
        SetFilterRuleString( limit_ptr_array, size_of_limit_ptr_array );
        UpdateFilter( combined_windivert_rules );
    }
    return 0;
}



void MessageLoop(){
    MSG message;
    while ( GetMessage( &message, NULL, 0, 0 ) ){
        TranslateMessage( &message );
        DispatchMessage( &message );
    }
}



DWORD WINAPI HotkeyThread( LPVOID lpParam ){
    HINSTANCE hInstance = GetModuleHandle( NULL );
    if ( !hInstance ){
        hInstance = LoadLibrary( ( LPCWSTR )lpParam );
    }
    if ( !hInstance ){ 
        return 1; 
    }
    hKeyboardHook = SetWindowsHookEx ( WH_KEYBOARD_LL, ( HOOKPROC ) KeyboardEvent, hInstance, NULL );
    MessageLoop();
    UnhookWindowsHookEx( hKeyboardHook );
    return 0;
}



void SetPathToConfigFile( wchar_t* config_filename ){ 
    wchar_t file_path_self[MAX_PATH], folder_path_self[MAX_PATH];
    GetModuleFileName( NULL, file_path_self, MAX_PATH );
    wcsncpy_s( folder_path_self, MAX_PATH, file_path_self, ( wcslen( file_path_self ) - wcslen( GetFilename( file_path_self ) ) ) );
    wchar_t filename[MAX_PATH], file_path[MAX_PATH];
    wcscpy_s( filename, MAX_PATH, config_filename );
    wcscpy_s( file_path, MAX_PATH, folder_path_self );
    wcscat_s( file_path, MAX_PATH, filename );
    wcsncpy_s( path_to_config_file, MAX_PATH, file_path, MAX_PATH );
}



void SetFilterRuleString( limit* limit_array[], int array_size ){
    strcpy_s( combined_windivert_rules, sizeof( combined_windivert_rules ), "(udp.DstPort < 1 and udp.DstPort > 1)" ); // set to rule that wont match anything

    for ( int i = 0; i < array_size; i++ ){
        if ( strcmp( limit_array[i]->windivert_rule, "" ) != 0 ){
            if ( limit_array[i]->state ){
                strcat_s( combined_windivert_rules, sizeof( combined_windivert_rules ), limit_array[i]->windivert_rule );
            }
        }
    }
    printf( "filter: %s\n", combined_windivert_rules );
}
