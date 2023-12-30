#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "Limit.h"
#include "UserInterface.h"
#include "HotkeyManager.h"
#include "HelperFunctions.h"
#include "ConfigFile.h"
#include <windows.h>
#include <GL/GL.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>


UserInterface::UserInterface(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, wchar_t* path_to_config_file, Settings* settings)
    : _limit_ptr_vector(limit_ptr_vector), _path_to_config_file(path_to_config_file), _settings(settings) {}

void UserInterface::FormatHotkeyStatusWcString(char* c_string, const std::atomic<Limit>* limit_ptr)
{
    constexpr int wc_string_size = 200;

    wchar_t wc_string[wc_string_size];
    if (limit_ptr->load().key_list[0] == 0) 
    {
        return;
    }

    wcscpy_s(wc_string, wc_string_size, L"");
    for (int i = 0; i < limit_ptr->load().max_key_list_size; i++) 
    {
        wchar_t name_buffer[256];
        if (limit_ptr->load().key_list[i] == 0) 
        {
            break;
        }

        if (wcscmp(wc_string, L"") != 0) 
        {
            wcscat_s(wc_string, wc_string_size, L"+");
        }

        const int scan_code = MapVirtualKey(limit_ptr->load().key_list[i], 0);
        GetKeyNameText(scan_code << 16, name_buffer, std::size(name_buffer));
        wcscat_s(wc_string, wc_string_size, name_buffer);
    }

    wcscat_s(wc_string, wc_string_size, L" ");
    WideCharToMultiByte(CP_UTF8, 0, wc_string, -1, c_string, 200, nullptr, nullptr);
    strcat_s(c_string, 200, limit_ptr->load().name);
    strcat_s(c_string, 200, limit_ptr->load().state ? "(on)" : "(off)");
}

void UserInterface::Overlay(bool* p_open, const HWND window_handle) const
{
    static int corner = 0;
    const ImGuiIO& io = ImGui::GetIO();

    if (corner != -1) 
    {
        constexpr float distance = 10.0f;
        const ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - distance : distance, (corner & 2) ? io.DisplaySize.y - distance : distance);
        const ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | 
                                    ImGuiWindowFlags_AlwaysAutoResize | 
                                    ImGuiWindowFlags_NoSavedSettings | 
                                    ImGuiWindowFlags_NoFocusOnAppearing | 
                                    ImGuiWindowFlags_NoNav | 
                                    ImGuiWindowFlags_NoResize;

    // make click-through (entire window)
    // TODO make it remove click-through when not needed
    SetWindowLongPtr(window_handle, GWL_EXSTYLE, GetWindowLongPtr(window_handle, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
    if (corner != -1) 
    {
        window_flags |= ImGuiWindowFlags_NoMove;
    }

    ImGuiStyle& style = ImGui::GetStyle();

    // Set background alpha (transparency)
    style.Colors[ImGuiCol_WindowBg].w = 0.0f;
    style.WindowBorderSize = 0.0f;

    if (ImGui::Begin("Overlay", p_open, window_flags)) 
    {
        std::vector<char[200]> char_ptr_vector(_limit_ptr_vector.size());
        for (size_t i = 0; i < _limit_ptr_vector.size(); i++) 
        {
            FormatHotkeyStatusWcString(char_ptr_vector[i], _limit_ptr_vector[i]);
            ImGui::PushID(i);
            if (strcmp(char_ptr_vector[i], "") != 0) 
            {
                ImGui::Text("%s", char_ptr_vector[i]);
            }

            ImGui::PopID();
        }
    }

    ImGui::End();
}

void UserInterface::Config(const HWND window_handle)
{
    SetWindowLongPtr(window_handle, GWL_EXSTYLE, GetWindowLongPtr(window_handle, GWL_EXSTYLE) | ~WS_EX_TRANSPARENT); // make clickthrough (entire window) // TODO make it remove clickthrough when not needed

    std::vector<bool> button_clicked(_limit_ptr_vector.size());
    std::vector<std::string> string_vector(_limit_ptr_vector.size());
    int line_of_button_clicked = -1;

    for (size_t i = 0; i < _limit_ptr_vector.size(); i++) 
    {
        string_vector.emplace_back("");
    }

    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("config", nullptr, flags);

    ImGui::SeparatorText("Hotkeys");

    float button_size = 0.0f;
    for (size_t i = 0; i < _limit_ptr_vector.size(); i++) 
    {
        if (_limit_ptr_vector[i]->load().key_list[0] == 0) 
        {
            string_vector[i] = "Bind";
        }
        else 
        {
            string_vector[i] = "";
            for (int j = 0; j < _limit_ptr_vector[i]->load().max_key_list_size; j++) 
            {
                if (_limit_ptr_vector[i]->load().key_list[j] == 0) 
                {
                    continue;
                }
                if (!string_vector[i].empty()) 
                {
                    string_vector[i].append("+");
                }

                const int scan_code = MapVirtualKey(_limit_ptr_vector[i]->load().key_list[j], 0);
                char name_buffer[256];
                GetKeyNameTextA(scan_code << 16, name_buffer, std::size(name_buffer));
                string_vector[i] += name_buffer;
            }
        }

        const char* bind = string_vector[i].c_str();
        const ImVec2 out_size = ImGui::CalcTextSize(bind);
        if (button_size < out_size.x)
        {
            button_size = out_size.x;
        }
    }

    for (size_t i = 0; i < _limit_ptr_vector.size(); i++) 
    {
        const char* in_progress = "in progress..";
        ImGui::PushID(i);

        ImGui::Text("%s ", _limit_ptr_vector[i]->load().name); // Display some text (you can use a format strings too)
        ImGui::SameLine();
        ImGui::SetCursorPosX(70);

        const char* bind = string_vector[i].c_str();
        if (ImGui::Button(bind, ImVec2(button_size + 20.0f, 0.0f))) 
        {
            if (string_vector[i] != in_progress) 
            {
                button_clicked[i] = true;
                std::cout << "button " << i << " clicked [callback]\n";
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset")) 
        {
            string_vector[i] = "";
            hk_instance->done = true;
            Limit temp_limit = _limit_ptr_vector[i]->load();
            for (int j = 0; j < temp_limit.max_key_list_size; j++) 
            {
                temp_limit.key_list[j] = 0;
            }
            temp_limit.binding_complete = true;
            temp_limit.update_ui = true;
            _limit_ptr_vector[i]->store(temp_limit);
        }

        if (_limit_ptr_vector[i]->load().binding_complete == true && string_vector[i] == in_progress) 
        {
            std::cout << "updating ui...\n";
            string_vector[i] = "";
            for (int j = 0; j < _limit_ptr_vector[i]->load().max_key_list_size; j++) 
            {
                if (_limit_ptr_vector[i]->load().key_list[0] == 0) 
                {
                    continue;
                }
                if (!string_vector[i].empty()) 
                {
                    string_vector[i].append("+");
                }

                const int scan_code = MapVirtualKey(_limit_ptr_vector[i]->load().key_list[j], 0);
                char name_buffer[256];
                GetKeyNameTextA(scan_code << 16, name_buffer, std::size(name_buffer));
                string_vector[i] += name_buffer;
            }
        }

        if (button_clicked[i] == true) 
        {
            std::cout << "button " << i << " clicked [registered]\n";
            string_vector[i] = in_progress;
            button_clicked[i] = false;
            line_of_button_clicked = static_cast<int>(i); // this isn't required, but a static cast shows its intentional
            std::thread([&]() { hk_instance->AsyncBindHotkey(line_of_button_clicked); }).detach();
        }

        ImGui::PopID();
    }

    if (ImGui::Button("Close")) 
    {
        // check if exit app is bound
        for (std::atomic<Limit>*& limit_ptr : _limit_ptr_vector) 
        {
            if (strcmp(limit_ptr->load().name, "exitapp") == 0) 
            {
                if (limit_ptr->load().key_list[0] == 0) 
                {
                    MessageBoxA(nullptr, "bind exitapp before closing", nullptr, MB_OK);
                }
                else 
                {
                    // ImGui::DestroyContext();
                    ConfigFile::WriteConfig(_limit_ptr_vector, _path_to_config_file, _settings);
                    ConfigFile::LoadConfig(_limit_ptr_vector, _path_to_config_file, _settings);
                    show_config  = false;
                    show_overlay = true;
                }
            }
        }
    }

    ImGui::End();
}


int UserInterface::RunGui()
{
    WNDCLASSEXW wc = 
    {
        sizeof(wc),
        CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        &UserInterface::WndProc,
        0L, 0L,
        GetModuleHandle(nullptr),
        nullptr, nullptr,
        nullptr, nullptr,
        L"ImGui Example", nullptr
    };

    RegisterClassExW(&wc);
    RECT desktop_rect;
    const HWND desktop_window_handle = GetDesktopWindow();
    GetWindowRect(desktop_window_handle, &desktop_rect);

    constexpr DWORD dw_style = WS_VISIBLE | WS_OVERLAPPED | WS_POPUP;
    const HWND window_handle = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED, wc.lpszClassName, L"klim config", dw_style, 0, 0, 250, 350, nullptr, nullptr, wc.hInstance, nullptr);


    // Initialize OpenGL
    WGL_WindowData g_main_window;
    if (!CreateDeviceWGL(window_handle, &g_main_window)) 
    {
        CleanupDeviceWGL(window_handle, &g_main_window);
        DestroyWindow(window_handle);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_main_window.device_context_handle, _g_render_context_handle);

    UpdateWindow(window_handle);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    
    ImFontConfig config;
    config.RasterizerMultiply = 1.0f; // Adjust the value for better antialiasing

    ImFont* custom_font  = io.Fonts->AddFontFromMemoryCompressedBase85TTF(Hack_Regular, 15.0f, &config);
    ImFont* default_font = io.Fonts->AddFontDefault();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable GamePad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(window_handle);
    ImGui_ImplOpenGL3_Init();

    // Our state
    constexpr ImVec4 clear_color = ImVec4(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);

    // Main loop
    bool done = false;
    while (!done) 
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (msg.message == WM_QUIT) 
            {
                done = true;
            }
        }
        if (done) 
        {
            break;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        ImGuiStyle& style = ImGui::GetStyle();
        style.FrameRounding = 0.0f;
        style.WindowPadding = ImVec2(15.0f, 5.0f);


        default_font->FontSize = 13.0f;
        ImGui::PushFont(default_font);
        if (show_config) 
        {
            Config(window_handle);
        }
        ImGui::PopFont();

        custom_font->FontSize = 13.0f;
        ImGui::PushFont(custom_font);
        if (show_overlay) 
        {
            Overlay(&show_overlay, window_handle);
        }
        ImGui::PopFont();


        // Rendering
        ImGui::Render();

        glViewport(0, 0, _g_width, _g_height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Present
        SwapBuffers(g_main_window.device_context_handle);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL(window_handle, &g_main_window);
    wglDeleteContext(_g_render_context_handle);
    DestroyWindow(window_handle);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool UserInterface::CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ChoosePixelFormat(hDc, &pfd);
    if (pf == 0) 
    {
        return false;
    }
    if (SetPixelFormat(hDc, pf, &pfd) == FALSE) 
    {
        return false;
    }

    ReleaseDC(hWnd, hDc);

    data->device_context_handle = GetDC(hWnd);
    if (!_g_render_context_handle) 
    {
        _g_render_context_handle = wglCreateContext(data->device_context_handle);
    }

    return true;
}

void UserInterface::CleanupDeviceWGL(HWND hWnd, const WGL_WindowData* data)
{
    wglMakeCurrent(nullptr, nullptr);
    ReleaseDC(hWnd, data->device_context_handle);
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
    {
        return true;
    }

    switch (msg)
    {
        case WM_CREATE:
            SetLayeredWindowAttributes(hWnd, RGB(128, 128, 128), 0, LWA_COLORKEY);
            break;
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                ui_instance->_g_width  = LOWORD(lParam);
                ui_instance->_g_height = HIWORD(lParam);
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
            hk_instance->KeyboardInputHandler(static_cast<int>(wParam), true);
            break;
        case WM_KEYUP:
            hk_instance->KeyboardInputHandler(static_cast<int>(wParam), false);
            break;
        default:
            break;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
