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

std::vector<int> currently_pressed_keys;

std::mutex mutex;
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
        while (!mutex.try_lock()) {
            Sleep(1);
        }
        _cur_line = i;
        if (limit_ptr_array[i]->bindingComplete == false) {
            MessageBox(NULL, (wchar_t*)L"error hotkey is already being bound...", NULL, MB_OK);
	        return;
        }
        limit_ptr_array[i]->bindingComplete = false;

        _currentHotkeyList.clear();

        //while (limit_ptr_array[i]->bindingComplete == false) {
        done = false;
        while (done == false) {
            Sleep(10);
        }
		limit_ptr_array[_cur_line]->updateUI = true;
        std::cout << "current size: " << _currentHotkeyList.size() << std::endl;
	    limit_ptr_array[_cur_line]->bindingComplete = true;
        _cur_line = -1;
        mutex.unlock();

        return;
	}

	void KeyboardInputHandler(int key, bool isKeyDown)
	{
        if (_cur_line != -1) {
		    if (isKeyDown)
			{
				std::cout << "Key Down: " << key << std::endl;
				auto it = std::find(_currentHotkeyList.begin(), _currentHotkeyList.end(), key);
				//if (it == _currentHotkeyList.end() && limit_ptr_array[_cur_line]->bindingComplete == false) {
				if (it == _currentHotkeyList.end()) {
				    _currentHotkeyList.push_back(key);
				}
			} else
			{
				std::cout << "Key Up: " << key << std::endl;
				auto it = std::find(_currentHotkeyList.begin(), _currentHotkeyList.end(), key);
				limit_ptr_array[_cur_line]->key_list = _currentHotkeyList;
				//limit_ptr_array[_cur_line]->bindingComplete = true;
                done = true;
			}
        }
	}
private:
    int _cur_line = -1;
    int done = false;

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
    std::vector<std::string> String;
    const char* in_progress = "in progress..";
    for (int i = 0; i < (size_of_limit_ptr_array); i++) {
        String.push_back("not set");
    }
    while (!done)
    {
        int line_of_button_clicked = -1;
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
                    ImGui::PushID(i);
                    if (ImGui::Button("Click to bind")) {
                        if (String[i] != in_progress) {
						        button_clicked[i] = true;
                                std::cout << "button " << i << " clicked [callback]" << std::endl;
						}
                    }

                    ImGui::SameLine();
					char name[50];
					size_t size;
					wcstombs_s(&size, name, limit_ptr_array[i]->name, 50);
					ImGui::Text(name);               // Display some text (you can use a format strings too)
					ImGui::SameLine();
					ImGui::Text("bind: %s", String[i].data());               // Display some text (you can use a format strings too)
                    ImGui::PopID();
                }
            }

            ImGui::End();
        }


        // Rendering
        ImGui::Render();

        for (int i = 0; i < button_clicked.size(); i++) {
            if (limit_ptr_array[i]->bindingComplete == true && String[i] == in_progress) {
                std::cout << "updating ui.." << std::endl;
                String[i] = "";
			    for (int j = 0; j < limit_ptr_array[i]->key_list.size(); j++) {
                    if (String[i] != "") {
					    String[i].append("+");
                    }
                    int scan_code = MapVirtualKey(limit_ptr_array[i]->key_list[j], 0);
                    char name_buffer[256];
                    GetKeyNameTextA(scan_code << 16, name_buffer, sizeof(name_buffer) / sizeof(name_buffer[0]));
                    String[i] += name_buffer;
				}
			}

            if (button_clicked[i] == true) {
                std::cout << "button " << i << " clicked [registered]" << std::endl;
                String[i] = in_progress;
				button_clicked[i] = false;
				line_of_button_clicked = i;
				std::thread([&]() {
					hotkeyManager.asyncBindHotkey(line_of_button_clicked);
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

void WriteConfig() {
    Json::Value config;

    wchar_t buffer[250];
    wchar_t buffer2[250];
	char char_buffer[250];
    for (int i = 0; i < size_of_limit_ptr_array; i++) {
        if (limit_ptr_array[i]->key_list[0] == 0x0) {
            continue;
        }
		size_t size;
		wcstombs_s(&size, char_buffer, limit_ptr_array[i]->name, 50);
        strcat_s(char_buffer, sizeof(char_buffer), "_key_list");

        wchar_t keyName[256];
        config[char_buffer] = vectorToJson(limit_ptr_array[i]->key_list);
    }

    // random defaults for now
    config["use_overlay"] = true;
    config["font_size"] = 30;
    config["color_default"] = "0x00FFFFFF";
    config["color_on"] = "0x000000FF";
    config["color_off"] = "0x00FFFFFF";

    StoreConfigToJson(path_to_config_file, config);
}


int __cdecl main( int argc, char** argv ){
    
    strcpy_s( lim_3074.windivert_rule,    sizeof( lim_3074.windivert_rule ),    " or (inbound and udp.SrcPort == 3074) or (inbound and tcp.SrcPort == 3074)" ); 
	strcpy_s( lim_3074_ul.windivert_rule, sizeof( lim_3074_ul.windivert_rule ), " or (outbound and udp.DstPort == 3074) or (outbound and tcp.DstPort == 3074)" ); 
	strcpy_s( lim_27k.windivert_rule,     sizeof( lim_27k.windivert_rule ),     " or (inbound and udp.SrcPort >= 27015 and udp.SrcPort <= 27200) or (inbound and tcp.SrcPort >= 27015 and tcp.SrcPort <= 27200)" ); 
	strcpy_s( lim_27k_ul.windivert_rule,  sizeof( lim_27k_ul.windivert_rule ),  " or (outbound and udp.DstPort >= 27015 and udp.DstPort <= 27200) or (outbound and tcp.DstPort >= 27015 and tcp.DstPort <= 27200)" ); 
	strcpy_s( lim_30k.windivert_rule,     sizeof( lim_30k.windivert_rule ),     " or (inbound and udp.SrcPort >= 30000 and udp.SrcPort <= 30009) or (inbound and tcp.SrcPort >= 30000 and tcp.SrcPort <= 30009)" ); 
	strcpy_s( lim_7k.windivert_rule,      sizeof( lim_7k.windivert_rule ),      " or (inbound and tcp.SrcPort >= 7500 and tcp.SrcPort <= 7509)" ); 

    if ( argv[1] != NULL ){
        if ( ( strcmp( argv[1], "--debug" ) == 0 ) ){
            printf( "debug: true\n" );
            debug = true;
        } else {
            printf( "error: invalid argument...\n" );
            printf( "options:\n" );
            printf( "    --debug     prevents console hiding and enables hotkey trigger outside of destiny 2.\n" );
            return 0;
        }
    } else {
        ShowWindow( GetConsoleWindow(), SW_HIDE );
    }

    if ( !RunningAsAdmin() ){
        MessageBox( NULL, ( LPCWSTR )L"ERROR: not running as admin", ( LPCWSTR )L"ERROR", MB_ICONERROR | MB_DEFBUTTON2 );
        return 0;
    }

    
    SetPathToConfigFile( ( wchar_t* )L"config.txt" );
    if ( !FileExists( path_to_config_file ) ){
        run_gui();
        WriteConfig();
        //WriteDefaultJsonConfig( path_to_config_file );
    }

    bool use_overlay;
	int font_size;
    LoadConfig( &use_overlay, &font_size, limit_ptr_array, size_of_limit_ptr_array );
    SetOverlayLineNumberOfHotkeys( limit_ptr_array, size_of_limit_ptr_array );
    InitializeOverlay( use_overlay, font_size, limit_ptr_array, size_of_limit_ptr_array );


    printf( "starting hotkey thread\n" );

    // TODO remove dwThread
    DWORD dwThread;
    hHotkeyThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )HotkeyThread, ( LPVOID )NULL, NULL, &dwThread );

    if ( hHotkeyThread ){
        return WaitForSingleObject( hHotkeyThread, INFINITE );
    wchar_t buffer[250];
    wchar_t buffer2[250];
	char char_buffer[250];
    }
    else {
        return 1;
    }
    CloseHandle( hHotkeyThread );
    FreeLibrary( hDLL );
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
    for ( int i = 0; i < size_of_limit_ptr_array-1; i++ ){
        if (limit_ptr_array[i]->key_list[0] != undefined_key) {
			limit_ptr_array[i]->overlay_line_number = current_overlay_line;
			current_overlay_line++;
		}
    }
}



void LoadConfig( bool* use_overlay, int* font_size, limit* limit_ptr_array[], int size_of_limit_ptr_array ){
    // Load the config from the JSON file
    Json::Value loaded_config = LoadConfigFileFromJson( path_to_config_file );
    // this could definitely be done programmatically but i think that would be more effort than its worth
    wchar_t buffer[250];
    wchar_t buffer2[250];
	char char_buffer[250];
    for (int i = 0; i < size_of_limit_ptr_array; i++) {
		size_t size;
		wcstombs_s(&size, char_buffer, limit_ptr_array[i]->name, 50);
        strcat_s(char_buffer, sizeof(char_buffer), "_key_list");

        limit_ptr_array[i]->key_list = jsonToVector(loaded_config[char_buffer]);
        if (limit_ptr_array[i]->key_list.size() == 0) {
            limit_ptr_array[i]->key_list.push_back(undefined_key);
        }
    }

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


void UnTriggerHotkeys() {
    for (int i = 0; i < size_of_limit_ptr_array; i++) {
        // Sort both vectors
        std::sort(limit_ptr_array[i]->key_list.begin(), limit_ptr_array[i]->key_list.end());
        std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), limit_ptr_array[i]->key_list.begin(), limit_ptr_array[i]->key_list.end());

        if (!containsAll) {
            limit_ptr_array[i]->triggered = false;
        }
    }
}


void TriggerHotkeys(){
    for ( int i = 0; i < size_of_limit_ptr_array; i++ ){
          // Sort both vectors
        std::sort(limit_ptr_array[i]->key_list.begin(), limit_ptr_array[i]->key_list.end());
		std::sort(currently_pressed_keys.begin(), currently_pressed_keys.end());
        bool containsAll = std::includes(currently_pressed_keys.begin(), currently_pressed_keys.end(), limit_ptr_array[i]->key_list.begin(), limit_ptr_array[i]->key_list.end());

        if (containsAll) {
            OnTriggerHotkey(limit_ptr_array[i]);
        }
    }
}


__declspec( dllexport ) LRESULT CALLBACK KeyboardEvent( int nCode, WPARAM wParam, LPARAM lParam ){
    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYUP ) || ( wParam == WM_KEYUP ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key =  *( ( KBDLLHOOKSTRUCT* )lParam );

        //int key = hooked_key.vkCode;
        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
        std::cout << "key recv: " << key << std::endl;


		// Check if the vector contains the target element
		auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it != currently_pressed_keys.end()) {
            currently_pressed_keys.erase(it);
		}
        UnTriggerHotkeys();
    }


    if  ( ( nCode == HC_ACTION ) && ( ( wParam == WM_SYSKEYDOWN ) || ( wParam == WM_KEYDOWN ) ) )      
    {
        KBDLLHOOKSTRUCT hooked_key = *( ( KBDLLHOOKSTRUCT* ) lParam );

        int key = MapVirtualKey(hooked_key.scanCode, MAPVK_VSC_TO_VK);
        //int key = hooked_key.vkCode;

        //HotkeySetKeyDownState( key );
		auto it = std::find(currently_pressed_keys.begin(), currently_pressed_keys.end(), key);
        if (it == currently_pressed_keys.end()) {
            currently_pressed_keys.push_back(key);
		}
        TriggerHotkeys( );
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
    if ( !limit->triggered ) {
		limit->triggered = true;
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
