#include "imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include <windows.h>
#include <GL/GL.h>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include "HotkeyManager.h"
#include "UserInterface.h"
#include "helperFunctions.h"
#include "ConfigFile.h"


UserInterface::UserInterface(std::vector<std::atomic<limit>*> limit_ptr_vector, wchar_t* path_to_config_file, Settings* settings) 
    : limit_ptr_vector(limit_ptr_vector), path_to_config_file(path_to_config_file), settings(settings) {
}

void UserInterface::FormatHotkeyStatusWcString( char* c_string, std::atomic<limit>* limit ){ 
    int szWcString = 200;
    
    wchar_t wcString[200];
    if (limit->load().key_list[0] == 0) { 
        return;
    }
    wchar_t nameBuffer[256];
    wcscpy_s(wcString, szWcString, L"");
    for (int i = 0; i < limit->load().max_key_list_size; i++) {
        if (limit->load().key_list[i] == 0) break;
        if (wcscmp(wcString, L"") != 0) {
            wcscat_s(wcString, szWcString, L"+");
        }
        int scan_code = MapVirtualKey(limit->load().key_list[i], 0);
        GetKeyNameText(scan_code << 16, nameBuffer, sizeof(nameBuffer) / sizeof(nameBuffer[0]));
        wcscat_s(wcString, szWcString, nameBuffer);
    }
    //wcscat_s(wcString, szWcString, L" to ");
    wcscat_s(wcString, szWcString, L" ");
    WideCharToMultiByte(CP_UTF8, 0, wcString, -1, c_string, 200, nullptr, nullptr);
    strcat_s(c_string, 200, limit->load().name);
    strcat_s(c_string, 200, limit->load().state ? "(on)" : "(off)");
}


void UserInterface::Overlay(bool* p_open, HWND hwnd)
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT); // make clickthrough (entire window) // TODO make it remove clickthrough when not needed
    if (corner != -1)
        window_flags |= ImGuiWindowFlags_NoMove;

    //ImFont* currentFont = ImGui::GetFont();
    //ImFont* newFont = io.Fonts->Fonts[0];  // Assuming you want to use the first font (default font)
    //float fontSize = 18.0f; // Adjust the font size as needed
    //newFont->Scale = fontSize / currentFont->FontSize;
      // Push the new font
    //ImGui::PushFont(newFont);
        // Set up ImGui style
    ImGuiStyle& style = ImGui::GetStyle();
    // Set background alpha (transparency)
    style.Colors[ImGuiCol_WindowBg].w = 0.0f; 
    style.WindowBorderSize = 0.0f;  // Set window border size to zero



    if (ImGui::Begin("Example: Simple overlay", p_open, window_flags))
    {

        std::vector<char[200]> char_ptr_vector(limit_ptr_vector.size());
        for (int i = 0; i < limit_ptr_vector.size(); i++) {
            UserInterface::FormatHotkeyStatusWcString(char_ptr_vector[i], limit_ptr_vector[i]);
            ImGui::PushID(i);
            if (strcmp(char_ptr_vector[i], "") != 0) {
                ImGui::Text(char_ptr_vector[i]);
            }
            ImGui::PopID();
        }
        //ImGui::Text("Simple overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
        //ImGui::Separator();
        //if (ImGui::BeginPopupContextWindow())
        //{
            //if (ImGui::MenuItem("Custom",       NULL, corner == -1)) corner = -1;
            //if (ImGui::MenuItem("Top-left",     NULL, corner == 0)) corner = 0;
            //if (ImGui::MenuItem("Top-right",    NULL, corner == 1)) corner = 1;
            //if (ImGui::MenuItem("Bottom-left",  NULL, corner == 2)) corner = 2;
            //if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            //if (p_open && ImGui::MenuItem("Close")) *p_open = false;
            //ImGui::EndPopup();
        //}
    }
    //ImGui::PopFont();
    ImGui::End();
}

void UserInterface::Config(HWND hwnd){
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | ~WS_EX_TRANSPARENT); // make clickthrough (entire window) // TODO make it remove clickthrough when not needed
    std::vector<bool> button_clicked(limit_ptr_vector.size());
    std::vector<std::string> String;
    int line_of_button_clicked = -1;
    const char* in_progress = "in progress..";
    for (int i = 0; i < (limit_ptr_vector.size()); i++) {
        String.push_back("");
    }
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("config", NULL, flags);

	ImGui::SeparatorText("Hotkeys");

    float buttonSize = 0.0f;
    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        if (limit_ptr_vector[i]->load().key_list[0] == 0) {
            String[i] = "Bind";
        }
        else {
            String[i] = "";
            for (int j = 0; j < limit_ptr_vector[i]->load().max_key_list_size; j++) {
                if (limit_ptr_vector[i]->load().key_list[j] == 0) continue;
                if (String[i] != "") {
                    String[i].append("+");
                }
                int scan_code = MapVirtualKey(limit_ptr_vector[i]->load().key_list[j], 0);
                char name_buffer[256];
                GetKeyNameTextA(scan_code << 16, name_buffer, sizeof(name_buffer) / sizeof(name_buffer[0]));
                String[i] += name_buffer;
            }
        }

        const char* bind = String[i].c_str();
        ImVec2 outSize = ImGui::CalcTextSize(bind);
        if (buttonSize < outSize.x) buttonSize = outSize.x;
    }

    for (int i = 0; i < limit_ptr_vector.size(); i++) {
        ImGui::PushID(i);

        ImGui::Text("%s ", limit_ptr_vector[i]->load().name);               // Display some text (you can use a format strings too)
        ImGui::SameLine();
        ImGui::SetCursorPosX(70);

        const char* bind = String[i].c_str();
        if (ImGui::Button(bind, ImVec2(buttonSize + 20.0f, 0.0f))) {
            if (String[i] != in_progress) {
                button_clicked[i] = true;
                std::cout << "button " << i << " clicked [callback]" << std::endl;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset")) {
            String[i] = "";
            hotkeyInstance->done = true;
            limit temp_limit = limit_ptr_vector[i]->load();
            for (int j = 0; j < temp_limit.max_key_list_size; j++) {
                temp_limit.key_list[j] = 0;
            }
            temp_limit.bindingComplete = true;
            temp_limit.updateUI = true;
            limit_ptr_vector[i]->store(temp_limit);
        }

        if (limit_ptr_vector[i]->load().bindingComplete == true && String[i] == in_progress) {
            std::cout << "updating ui.." << std::endl;
            String[i] = "";
            for (int j = 0; j < limit_ptr_vector[i]->load().max_key_list_size; j++) {
                if (limit_ptr_vector[i]->load().key_list[0] == 0) continue;
                if (String[i] != "") {
                    String[i].append("+");
                }
                int scan_code = MapVirtualKey(limit_ptr_vector[i]->load().key_list[j], 0);
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
                hotkeyInstance->asyncBindHotkey(line_of_button_clicked);
                }).detach();
        }

        ImGui::PopID();
    }


	//ImGui::SetCursorPos(ImVec2(100, 260));
	if (ImGui::Button("Close")) {
        // check if exitapp is bound
        for (int i = 0; i < limit_ptr_vector.size(); i++) {
            if (strcmp(limit_ptr_vector[i]->load().name, "exitapp") == 0) {
                if (limit_ptr_vector[i]->load().key_list[0] == 0) {
                    MessageBoxA(NULL, "bind exitapp before closing", NULL, MB_OK);
                }
                else {
		            //ImGui::DestroyContext();
		            ConfigFile::WriteConfig(limit_ptr_vector, path_to_config_file, settings);
					ConfigFile::LoadConfig( limit_ptr_vector, path_to_config_file, settings);
					show_config = false;
					show_overlay = true;
                }
            }
        }
	}

    ImGui::End();	
}



int UserInterface::run_gui(){
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    //void (UserInterface::*ptrWndProc)(LRESULT WINAPI)
    WNDCLASSEXW wc = { sizeof(wc), CS_OWNDC | CS_HREDRAW | CS_VREDRAW, &UserInterface::WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    RECT desktopRect;
    HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktopRect);
    // Calculate the pixel size
    //int screenWidth = desktopRect.right - desktopRect.left;
	//int screenHeight = desktopRect.bottom - desktopRect.top;
    DWORD dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_POPUP;
    //HWND hwnd = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED, wc.lpszClassName, L"klim config", dwStyle, 0, 0, screenWidth, screenHeight, NULL, NULL, wc.hInstance, NULL);
    HWND hwnd = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED, wc.lpszClassName, L"klim config", dwStyle, 0, 0, 250, 350, NULL, NULL, wc.hInstance, NULL);



	WGL_WindowData   g_MainWindow;
    // Initialize OpenGL
    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFontConfig config;
    config.RasterizerMultiply = 1.0f; // Adjust the value for better antialiasing
    //ImFont* customFont = io.Fonts->AddFontFromFileTTF("Hack-Regular.ttf", 18.0f, &config); 
    ImFont* customFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(Hack_Regular, 15.0f, &config); 
    ImFont* defaultFont = io.Fonts->AddFontDefault();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

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
    ImVec4 clear_color = ImVec4(128.0f/255.0f, 128.0f/255.0f, 128.0f/255.0f, 1.0f);
    //ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    // Main loop
    bool done = false;
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
        static float f = 0.0f;
		static int counter = 0;
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 0.0f;
		style.WindowPadding = ImVec2(15.0f, 5.0f);


        defaultFont->FontSize = 13.0f;
        ImGui::PushFont(defaultFont);
        if (show_config)  UserInterface::Config(hwnd);
        ImGui::PopFont();



        customFont->FontSize = 13.0f;
        ImGui::PushFont(customFont);
		if (show_overlay) UserInterface::Overlay(&show_overlay, hwnd);
        ImGui::PopFont();

        

        // Rendering
        ImGui::Render();

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

// Helper functions
bool UserInterface::CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
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

void UserInterface::CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
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

LRESULT WINAPI UserInterface::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_CREATE:
		SetLayeredWindowAttributes(hWnd, RGB(128, 128, 128), 0, LWA_COLORKEY);
        break;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            instance->g_Width = LOWORD(lParam);
            instance->g_Height = HIWORD(lParam);
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
        hotkeyInstance->KeyboardInputHandler(static_cast<int>(wParam), true);
        break;
    case WM_KEYUP:
        hotkeyInstance->KeyboardInputHandler(static_cast<int>(wParam), false);
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
