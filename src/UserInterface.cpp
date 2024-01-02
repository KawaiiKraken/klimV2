#include "UserInterface.h"
#include "ConfigFile.h"
#include "HelperFunctions.h"
#include "HotkeyManager.h"
#include "Limit.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include <GL/GL.h>
#include <dwmapi.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>
#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Klim
{
    UserInterface::UserInterface(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, wchar_t* path_to_config_file, Settings* settings)
        : _limit_ptr_vector(limit_ptr_vector)
        , _path_to_config_file(path_to_config_file)
        , _settings(settings)
    {
    }

    void UserInterface::FormatHotkeyStatusWcString(char* c_string, const std::atomic<Limit>* limit_ptr)
    {
        // No keybinds set
        if (limit_ptr->load().key_list[0] == 0)
        {
            return;
        }

        constexpr int wc_string_size = 200;
        wchar_t hotkey_state_string[wc_string_size];

        wcscpy_s(hotkey_state_string, wc_string_size, L"");

        for (int i = 0; i < limit_ptr->load().max_key_list_size; i++)
        {
            wchar_t name_buffer[256];
            if (limit_ptr->load().key_list[i] == 0)
            {
                break;
            }

            if (wcscmp(hotkey_state_string, L"") != 0)
            {
                wcscat_s(hotkey_state_string, wc_string_size, L"+");
            }

            const int scan_code = MapVirtualKey(limit_ptr->load().key_list[i], 0);
            GetKeyNameText(scan_code << 16, name_buffer, std::size(name_buffer));
            wcscat_s(hotkey_state_string, wc_string_size, name_buffer);
        }

        wcscat_s(hotkey_state_string, wc_string_size, L" ");

        WideCharToMultiByte(CP_UTF8, 0, hotkey_state_string, -1, c_string, 200, nullptr, nullptr);
        strcat_s(c_string, 200, limit_ptr->load().name);


        strcat_s(c_string, 200, " ");
        strcat_s(c_string, 200, limit_ptr->load().state ? "(on)" : "(off)");
    }

    void UserInterface::Overlay(bool* p_open, const HWND window_handle) const
    {
        const ImGuiIO& io = ImGui::GetIO();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize;

        // make click-through (entire window)
        // TODO make it remove click-through when not needed
        SetWindowLongPtr(window_handle, GWL_EXSTYLE, GetWindowLongPtr(window_handle, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

        ImGuiStyle& style = ImGui::GetStyle();

        // Set background alpha (transparency)
        style.Colors[ImGuiCol_WindowBg].w = 0.0f;
        style.WindowBorderSize = 0.0f;
        ImGui::SetNextWindowPos(ImVec2(0, 0));

        ImVec2 overlay_window_size(0.0f, 0.0f);
        if (ImGui::Begin("Overlay", p_open, window_flags))
        {
            std::vector<char[200]> char_ptr_vector(_limit_ptr_vector.size());
            for (size_t i = 0; i < _limit_ptr_vector.size(); i++)
            {
                FormatHotkeyStatusWcString(char_ptr_vector[i], _limit_ptr_vector[i]);
                ImGui::PushID(static_cast<int>(i));
                if (strcmp(char_ptr_vector[i], "") != 0)
                {
                    ImGui::Text(char_ptr_vector[i]);
                    if (overlay_window_size.x < ImGui::CalcTextSize(char_ptr_vector[i]).x + 30)
                    {
                        overlay_window_size.x = ImGui::CalcTextSize(char_ptr_vector[i]).x;
                        overlay_window_size.x += 30; // TODO replace with window style padding
                    }
                    overlay_window_size.y += ImGui::CalcTextSize(char_ptr_vector[i]).y;
                    overlay_window_size.y += 10;
                }
                ImGui::PopID();
            }
            ImGui::SetWindowSize(overlay_window_size);
            SetWindowPos(window_handle, 0, 0, 0, overlay_window_size.x, overlay_window_size.y, SWP_NOZORDER | SWP_NOMOVE);
        }

        ImGui::End();
    }

    void UserInterface::Config(const HWND window_handle)
    {
        // make click-through (entire window)
        // TODO make it remove click-through when not needed
        SetWindowLongPtr(window_handle, GWL_EXSTYLE, GetWindowLongPtr(window_handle, GWL_EXSTYLE) | ~WS_EX_TRANSPARENT);


        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(250, 360));

        ImGui::Begin("Config", nullptr, flags);
        if (ImGui::BeginTabBar("MyTabBar"))
        {
            if (ImGui::BeginTabItem("Hotkeys"))
            {
                UserInterface::HotkeyTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visuals"))
            {
                UserInterface::UiConfigTab(window_handle);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        _window_size = ImGui::GetWindowSize();
        SetWindowPos(window_handle, 0, 0, 0, _window_size.x, _window_size.y, SWP_NOZORDER | SWP_NOMOVE);

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        // Calculate the bottom boundary of the window
        float windowBottom = windowPos.y + windowSize.y;
        ImGui::SetCursorPosY(windowBottom - 30);
        if (ImGui::Button("Close"))
        {
            // check if exit app is bound
            for (std::atomic<Limit>*& limit_ptr : _limit_ptr_vector)
            {
                if (strcmp(limit_ptr->load().name, Limit::TypeToString(exit_app)) == 0)
                {
                    if (limit_ptr->load().key_list[0] == 0)
                    {
                        MessageBoxA(nullptr, "bind Exit App before closing", nullptr, MB_OK);
                    }
                    else
                    {
                        ConfigFile::WriteConfig(_limit_ptr_vector, _path_to_config_file, _settings);
                        ConfigFile::LoadConfig(_limit_ptr_vector, _path_to_config_file, _settings);
                        show_config = false;
                        show_overlay = true;
                    }
                }
            }
        }

        ImGui::End();
    }

    void UserInterface::HotkeyTab()
    {
        std::vector<bool> button_clicked(_limit_ptr_vector.size());
        std::vector<std::string> string_vector(_limit_ptr_vector.size());

        int line_of_button_clicked = -1;

        for (size_t i = 0; i < _limit_ptr_vector.size(); i++)
        {
            string_vector.emplace_back("");
        }
        ImGui::SeparatorText("");

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
            const char* in_progress = "in progress...";
            ImGui::PushID(static_cast<int>(i));

            ImGui::Text("%s ", _limit_ptr_vector[i]->load().name); // Display some text (you can use a format strings too)
            ImGui::SameLine();
            ImGui::SetCursorPosX(115);

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
                bool can_trigger_hotkey = true;
                for (int j = 0; j < _limit_ptr_vector.size(); j++)
                {
                    if (_limit_ptr_vector[j]->load().binding_complete == false)
                    {
                        can_trigger_hotkey = false;
                    }
                }
                if (can_trigger_hotkey)
                {
                    std::thread([&]() { hk_instance->AsyncBindHotkey(line_of_button_clicked); }).detach();
                }
            }

            ImGui::PopID();
        }
    }

    void UserInterface::HelpMarker(const char* description)
    {
        ImGui::Text("(?)");
        if (ImGui::IsItemHovered())
            if (ImGui::BeginItemTooltip())
            {
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(description);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
    }

    void UserInterface::UiConfigTab(const HWND hwnd)
    {
        ImGui::SeparatorText("Overlay");
        {
            static bool show_overlay = true;
            ImGui::Checkbox("Show Overlay", &show_overlay);
            ImGui::SameLine();
            UserInterface::HelpMarker("Show/Hide overlay.");

            static bool show_hotkey = true;
            ImGui::Checkbox("Show hotkey", &show_hotkey);
            ImGui::SameLine();
            UserInterface::HelpMarker("Shows the hotkey to trigger a limit.");

            static bool show_timer = true;
            ImGui::Checkbox("Show timer", &show_timer);
            ImGui::SameLine();
            UserInterface::HelpMarker("Shows how long a limit has been on.");

            static bool frosted_glass = false;
            ImGui::Checkbox("Frosted glass", &frosted_glass);
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                UserInterface::BlurBehindHwnd(hwnd, frosted_glass);
            }
            ImGui::SameLine();
            UserInterface::HelpMarker("Frosted glass effect as background.");

            static bool show_limit_state = true;
            ImGui::Checkbox("Show limit state", &show_limit_state);
            ImGui::SameLine();
            UserInterface::HelpMarker("(on)/(off) text.");

            static bool change_text_color = false;
            ImGui::Checkbox("Change color", &change_text_color);
            ImGui::SameLine();
            UserInterface::HelpMarker("Change text color based on limit state.");
        }

        ImGui::SeparatorText("Misc");
        {
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 maxWidgetSize = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);


            ImGui::Text("Window position");
            ImGui::PushID(0xdeadbeef); // doesn't work in this case unless given an id
            const char* possible_window_pos[] = { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };
            ImGui::Combo("", &_window_location, possible_window_pos, IM_ARRAYSIZE(possible_window_pos));
            ImGui::PopID();


            ImGui::Text("Text size");
            ImGui::SameLine();
            UserInterface::HelpMarker("Currently broken");
            ImGui::BeginDisabled();
            ImGui::InputInt("", &font_size);
            ImGui::EndDisabled();


            ImGui::Text("Theme");
            ImGui::SameLine();
            UserInterface::HelpMarker("coming soon..?");
        }
    }


    void UserInterface::SetHwndPos(HWND hwnd)
    {
        RECT desktop_rect;
        const HWND desktop_window_handle = GetDesktopWindow();
        GetWindowRect(desktop_window_handle, &desktop_rect);
        const float PAD = 0.0f;
        ImVec2 work_pos(0, 0); // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size(desktop_rect.right, desktop_rect.bottom);
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (_window_location & 1) ? (work_pos.x + work_size.x - PAD - _window_size.x) : (work_pos.x + PAD);
        window_pos.y = (_window_location & 2) ? (work_pos.y + work_size.y - PAD - _window_size.y) : (work_pos.y + PAD);
        window_pos_pivot.x = (_window_location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (_window_location & 2) ? 1.0f : 0.0f;

        SetWindowPos(hwnd, 0, window_pos.x, window_pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }


    int UserInterface::RunGui()
    {
        WNDCLASSEXW wc = { sizeof(wc), CS_OWNDC | CS_HREDRAW | CS_VREDRAW, &UserInterface::WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };

        RegisterClassExW(&wc);
        RECT desktop_rect;
        const HWND desktop_window_handle = GetDesktopWindow();
        GetWindowRect(desktop_window_handle, &desktop_rect);

        constexpr DWORD dw_style = WS_VISIBLE | WS_OVERLAPPED | WS_POPUP;
        const HWND window_handle = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED, wc.lpszClassName, L"klim config", dw_style, 0, 0, 1000, 1000, nullptr, nullptr, wc.hInstance, nullptr);

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

        ImFont* custom_font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(Hack_Regular, 18.0f, &config);
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


            ImGui::PushFont(default_font);
            if (show_config)
            {
                Config(window_handle);
            }
            ImGui::PopFont();

            custom_font->FontSize = static_cast<float>(font_size);
            ImGui::PushFont(custom_font);
            custom_font->FontSize = static_cast<float>(font_size);
            if (show_overlay)
            {
                Overlay(&show_overlay, window_handle);
            }
            ImGui::PopFont();

            UserInterface::SetHwndPos(window_handle);

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

    void UserInterface::BlurBehindHwnd(HWND hwnd, bool state)
    {
        enum AccentState
        {
            ACCENT_DISABLED = 0,
            ACCENT_ENABLE_GRADIENT = 1,
            ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
            ACCENT_ENABLE_BLURBEHIND = 3,
            ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
            ACCENT_ENABLE_HOSTBACKDROP = 5, // RS5 1809
            ACCENT_INVALID_STATE = 6
        };
        enum WindowCompositionAttribute
        {
            WCA_ACCENT_POLICY = 19
        };
        struct AccentPolicy
        {
                AccentState accentState;
                int accentFlags;
                int gradientColor;
                int animationId;
        };
        struct WindowCompositionAttributeData
        {
                WindowCompositionAttribute attribute;
                PVOID pData;
                ULONG dataSize;
        };
        typedef BOOL(WINAPI * pSetWindowCompositionAttribute)(HWND, WindowCompositionAttributeData*);
        const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
        // int isRS4OrGreater = 1;
        // int redValue = 0;
        // int greenValue = 0;
        // int blueValue = 0;
        // int alphaValue = 20;
        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
        // int gradientColor = (alphaValue<<24) + (blueValue<<16) + (greenValue<<8) + (redValue);
        // AccentState blurType = isRS4OrGreater == 1 ? ACCENT_ENABLE_ACRYLICBLURBEHIND : ACCENT_ENABLE_BLURBEHIND;
        // AccentPolicy policy = {blurType, 2, gradientColor, 0};
        AccentPolicy policy;
        if (state)
        {
            policy = { ACCENT_ENABLE_ACRYLICBLURBEHIND, 2, 1, 0 };
        }
        else
        {
            policy = { ACCENT_DISABLED, 0, 0, 0 };
        }
        WindowCompositionAttributeData data = { WCA_ACCENT_POLICY, &policy, sizeof(AccentPolicy) };
        SetWindowCompositionAttribute(hwnd, &data);
    }


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
                if (wParam != SIZE_MINIMIZED)
                {
                    // ui_instance->_g_width = LOWORD(lParam);
                    // ui_instance->_g_height = HIWORD(lParam);
                }
                return 0;
            case WM_SYSCOMMAND:
                if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                {
                    return 0;
                }
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
}
