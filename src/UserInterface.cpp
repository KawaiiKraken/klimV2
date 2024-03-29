#include "UserInterface.h"
#include "ConfigFile.h"
#include "HelperFunctions.h"
#include "HotkeyManager.h"
#include "Limit.h"
#include <GL/GL.h>
#include <dwmapi.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>
#include <iostream>
#include <shellapi.h>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>
#pragma comment(lib, "dwmapi.lib")

// this should prob be moved somewhere else
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Klim
{
    UserInterface::UserInterface(const std::vector<std::atomic<Limit>*>& limit_ptr_vector, wchar_t* path_to_config_file, Settings* settings, std::shared_ptr<spdlog::logger> _logger)
        : _limit_ptr_vector(limit_ptr_vector)
        , _path_to_config_file(path_to_config_file)
        , _settings(settings)
        , _custom_font(nullptr)
        , _line_of_button_clicked(-1) // this could have been a local variable but it goes out of scope and crashes the release build
        , logger(_logger)
    {
    }

    UserInterface::FrameRateLimiter::FrameRateLimiter(int targetFPS)
    {
        targetFrameDuration = std::chrono::milliseconds(1000) / targetFPS;
        lastFrameTime = std::chrono::steady_clock::now();
    }

    void UserInterface::FrameRateLimiter::StartFrame()
    {
        // Calculate the time taken for the previous frame
        auto currentTime = std::chrono::steady_clock::now();
        auto frameDuration = currentTime - lastFrameTime;

        // If the frame took less time than the target duration, sleep for the remaining time
        if (frameDuration < targetFrameDuration)
        {
            auto sleepTime = targetFrameDuration - frameDuration;
            std::this_thread::sleep_for(sleepTime);
        }

        // Update the last frame time for the next frame
        lastFrameTime = std::chrono::steady_clock::now();
    }

    ImVec4 UserInterface::ColorRefToImVec4(COLORREF colorRef)
    {
        ImVec4 color;
        color.x = (float)GetRValue(colorRef) / 255.0f;
        color.y = (float)GetGValue(colorRef) / 255.0f;
        color.z = (float)GetBValue(colorRef) / 255.0f;
        color.w = 1.0f; // Alpha set to 1
        return color;
    }


    // format text for the overlay
    std::string UserInterface::FormatHotkeyStatus(const std::atomic<Limit>* limit_ptr)
    {
        // No keybinds set
        if (limit_ptr->load().key_list[0] == 0)
        {
            return "";
        }

        std::string hotkey_status = "";

        if (_settings->show_hotkey)
        {
            for (int i = 0; i < limit_ptr->load().max_key_list_size; i++)
            {
                if (limit_ptr->load().key_list[i] == 0)
                {
                    break;
                }

                if (hotkey_status != "")
                {
                    hotkey_status += "+";
                }

                const int scan_code = MapVirtualKey(limit_ptr->load().key_list[i], 0);

                char name_buffer[256];

                if (!GetKeyNameTextA(scan_code << 16, name_buffer, static_cast<int>(std::size(name_buffer))))
                {
                    // this should prob be a ret check for keyname or mapvk
                    std::string mouse_button = Helper::GetMouseButtonNameByVkCode(limit_ptr->load().key_list[i]);
                    strcpy_s(name_buffer, sizeof(name_buffer), mouse_button.c_str());
                }

                hotkey_status += name_buffer;
            }
            hotkey_status += " ";
        }

        hotkey_status += limit_ptr->load().name;

        if (_settings->show_limit_state)
        {
            hotkey_status += limit_ptr->load().state ? " (on)" : " (off)";
        }

        return hotkey_status;
    }

    void UserInterface::Overlay(bool* p_open, const HWND window_handle) const
    {
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
            if (_settings->debug)
            {
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                overlay_window_size.y += ImGui::CalcTextSize("FPS: %.1f").y;
                overlay_window_size.y += 7;
            }
            std::vector<std::string> char_ptr_vector(_limit_ptr_vector.size());
            for (size_t i = 0; i < _limit_ptr_vector.size(); i++)
            {
                char_ptr_vector[i] = ui_instance->FormatHotkeyStatus(_limit_ptr_vector[i]);
                ImGui::PushID(static_cast<int>(i));
                if (char_ptr_vector[i] == "")
                {
                    continue;
                }
                ImVec4 color(1.0f, 1.0f, 1.0f, 1.0f); // default color
                // ImGui::Text(char_ptr_vector[i]);
                if (_limit_ptr_vector[i]->load().state)
                {
                    color = UserInterface::ColorRefToImVec4(_settings->color_on);
                }
                else
                {
                    color = UserInterface::ColorRefToImVec4(_settings->color_off);
                }
                if (!_settings->change_text_color)
                {
                    color = UserInterface::ColorRefToImVec4(_settings->color_default);
                }
                ImGui::TextColored(color, char_ptr_vector[i].c_str());
                char formattedString[50];
                strcpy_s(formattedString, sizeof(formattedString), "");
                if (_settings->show_timer == true && timer_vector[i].running == true)
                {
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::Text("%.1f", timer_vector[i].getElapsedTime());
                    ImGui::PopStyleColor();
                    snprintf(formattedString, sizeof(formattedString), " %.1f", timer_vector[i].getElapsedTime());
                }
                if (overlay_window_size.x < ImGui::CalcTextSize(char_ptr_vector[i].c_str()).x + ImGui::CalcTextSize(formattedString).x + 30)
                {
                    overlay_window_size.x = ImGui::CalcTextSize(char_ptr_vector[i].c_str()).x + ImGui::CalcTextSize(formattedString).x + 30; // TODO replace with window style padding
                }
                overlay_window_size.y += ImGui::CalcTextSize(char_ptr_vector[i].c_str()).y;
                overlay_window_size.y += 7;
                ImGui::PopID();
            }
            ui_instance->_window_size = overlay_window_size;
            ImGui::SetWindowSize(overlay_window_size);
            SetWindowPos(window_handle, 0, 0, 0, static_cast<int>(overlay_window_size.x), static_cast<int>(overlay_window_size.y), SWP_NOZORDER | SWP_NOMOVE);
            ui_instance->BlurBehindHwnd(window_handle, _settings->frosted_glass);
        }

        ImGui::End();
    }

    void UserInterface::Config(const HWND window_handle)
    {
        // make click-through (entire window)
        // TODO make it remove click-through when not needed
        SetWindowLongPtr(window_handle, GWL_EXSTYLE, GetWindowLongPtr(window_handle, GWL_EXSTYLE) | ~WS_EX_TRANSPARENT);


        // static ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        // ImGui::SetNextWindowSize(ImVec2(250, 360));

        // does this have to happen every frame?
        if (_settings->use_custom_theme)
        {
            ImGuiApplyTheme_EnemyMouse();
        }
        else
        {
            ImGui::StyleColorsDark();
        }

        ImGui::Begin("klim", nullptr, flags);
        if (_settings->debug)
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
        if (ImGui::BeginTabBar("MyTabBar"))
        {
            if (ImGui::BeginTabItem("Hotkeys"))
            {
                UserInterface::HotkeyTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Visuals"))
            {
                UserInterface::VisualsTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Advanced"))
            {
                UserInterface::AdvancedTab();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        _window_size = ImGui::GetWindowSize();
        // set size
        // SetWindowPos(window_handle, 0, 0, 0, static_cast<int>(_window_size.x), static_cast<int>(_window_size.y), SWP_NOZORDER | SWP_NOMOVE);
        SetWindowPos(window_handle, 0, 0, 0, 400, 400, SWP_NOZORDER | SWP_NOMOVE);

        if (ImGui::Button("Start"))
        {
            // check if exit app is bound
            if (Limit::GetLimitPtrByType(_limit_ptr_vector, exit_app)->load().key_list[0] == 0)
            {
                MessageBoxA(nullptr, "bind Exit App before closing", nullptr, MB_OK);
            }
            else
            {
                ConfigFile::WriteConfig(_limit_ptr_vector, _path_to_config_file, _settings, logger);

                if (_restart_required)
                {
                    // TODO fix restart
                    // RestartApp();
                    ExitProcess(0);
                }


                ConfigFile::LoadConfig(_limit_ptr_vector, _path_to_config_file, _settings);
                show_config = false;
                if (_settings->show_overlay)
                {
                    show_overlay = true;
                }
            }
        }
        ImGui::End();
    }


    // self explanatory, currently broken
    void UserInterface::RestartApp()
    {
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(NULL, szPath, MAX_PATH);

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Create a new process
        if (CreateProcess(szPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            // Close the handles to the child process and its primary thread
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            // Exit the current process
            ExitProcess(0);
        }
    }

    void UserInterface::HotkeyTab()
    {
        std::vector<bool> button_clicked(_limit_ptr_vector.size());
        std::vector<std::string> string_vector(_limit_ptr_vector.size());


        for (size_t i = 0; i < _limit_ptr_vector.size(); i++)
        {
            string_vector.emplace_back("");
        }
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
                // TODO make this shit into a function
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
                    if (!GetKeyNameTextA(scan_code << 16, name_buffer, static_cast<int>(std::size(name_buffer))))
                    {
                        // this should prob be a ret check for keyname or mapvk
                        std::string mouse_button = Helper::GetMouseButtonNameByVkCode(_limit_ptr_vector[i]->load().key_list[j]);
                        strcpy_s(name_buffer, sizeof(name_buffer), mouse_button.c_str());
                    }
                    string_vector[i] += name_buffer;
                }
            }
            if (_limit_ptr_vector[i]->load().binding_complete == false)
            {
                string_vector[i] = "...";
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
                    string_vector[i] = in_progress;
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
                    GetKeyNameTextA(scan_code << 16, name_buffer, static_cast<int>(std::size(name_buffer)));
                    string_vector[i] += name_buffer;
                }

                if (button_clicked[i] == true)
                {
                    string_vector[i] = in_progress;
                    button_clicked[i] = false;
                    // line_of_button_clicked = static_cast<int>(i); // this isn't required, but a static cast shows its intentional
                    _line_of_button_clicked = static_cast<int>(i);
                    std::thread([&]() { hk_instance->AsyncBindHotkey(_line_of_button_clicked); }).detach();
                }
            }

            ImGui::PopID();
        }
    }

    // show tooptip on hover
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


    void UserInterface::VisualsTab()
    {
        // TODO make it save config on every edit
        ImGui::SeparatorText("Visuals");
        {
            ImGui::Text("Window Position");
            ImGui::SameLine();
            UserInterface::HelpMarker("Both config and overlay.");
            ImGui::PushID(0xdeadbeef); // doesn't work in this case unless given an id.. why??
            const char* possible_window_pos[] = { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };
            ImGui::Combo("", &_settings->window_location, possible_window_pos, IM_ARRAYSIZE(possible_window_pos));
            ImGui::PopID();

            ImGui::Checkbox("Auto-Hide Overlay", &_settings->auto_hide_overlay);
            ImGui::SameLine();
            UserInterface::HelpMarker("Hide overlay when tabbed out.");

            // temp removed until rework
            /*
            ImGui::Checkbox("Show Overlay", &_settings->show_overlay);
            ImGui::SameLine();
            UserInterface::HelpMarker("Show/Hide overlay.");
            */

            ImGui::Checkbox("Show Hotkey", &_settings->show_hotkey);
            ImGui::SameLine();
            UserInterface::HelpMarker("Shows the hotkey to trigger a limit.");

            ImGui::Checkbox("Show Timer", &_settings->show_timer);
            ImGui::SameLine();
            UserInterface::HelpMarker("Shows how long a limit has been on.");

            /*
            ImGui::Checkbox("Frosted glass", &_settings->frosted_glass);
            ImGui::SameLine();
            UserInterface::HelpMarker("Frosted glass effect as background.");
            */

            ImGui::Checkbox("Show Limit State", &_settings->show_limit_state);
            ImGui::SameLine();
            UserInterface::HelpMarker("(on)/(off) text.");

            ImGui::Checkbox("Change Color", &_settings->change_text_color);
            ImGui::SameLine();
            UserInterface::HelpMarker("Change text color based on limit state.");

            ImGui::Text("Overlay Text Size");
            ImGui::SameLine();
            UserInterface::HelpMarker("restart required (its automatic)");
            ImGui::InputInt("", &_settings->font_size);
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                _restart_required = true;
            }
        }

        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 maxWidgetSize = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);


        ImGui::Checkbox("New Theme", &_settings->use_custom_theme);
        ImGui::SameLine();
        UserInterface::HelpMarker("by github.com/enemymouse");
    }

    void UserInterface::AdvancedTab()
    {
        ImGui::SeparatorText("Advanced");
        {
            ImGui::Checkbox("Show Console", &_settings->show_console);
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                if (_settings->show_console)
                {
                    ShowWindow(GetConsoleWindow(), SW_RESTORE);
                }
                else
                {
                    ShowWindow(GetConsoleWindow(), SW_HIDE);
                }
            }
            ImGui::SameLine();
            UserInterface::HelpMarker("dont select text with mouse, that crashes it lmao");

            ImGui::Checkbox("Debug", &_settings->debug);
            ImGui::SameLine();
            UserInterface::HelpMarker("dont use unless you know what youre doing.. like really");

            ImGui::Checkbox("passthrough", &_settings->force_passthrough);
            ImGui::SameLine();
            UserInterface::HelpMarker("dont block/limit anything");

            ImGui::Checkbox("always on top", &_settings->always_on_top);
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                if (_settings->always_on_top)
                {
                    SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                else
                {
                    SetWindowPos(GetConsoleWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
            }
            ImGui::SameLine();
            UserInterface::HelpMarker("console only");

            ImGui::SliderInt("FPS", &_settings->fps, 30, 200); // Slider for FPS adjustment
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                //_restart_required = true;
                fps_limit = std::make_unique<FrameRateLimiter>(_settings->fps);
            }
            ImGui::SameLine();
            UserInterface::HelpMarker("useful for people who have vsync off");
        }
    }

    void UserInterface::SetHwndPos(HWND hwnd)
    {
        RECT desktop_rect;
        const HWND desktop_window_handle = GetDesktopWindow();
        GetWindowRect(desktop_window_handle, &desktop_rect);
        const float PAD = 0.0f;
        ImVec2 work_pos(0, 0); // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size(static_cast<float>(desktop_rect.right), static_cast<float>(desktop_rect.bottom));
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (_settings->window_location & 1) ? (work_pos.x + work_size.x - PAD - _window_size.x) : (work_pos.x + PAD);
        window_pos.y = (_settings->window_location & 2) ? (work_pos.y + work_size.y - PAD - _window_size.y) : (work_pos.y + PAD);
        window_pos_pivot.x = (_settings->window_location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (_settings->window_location & 2) ? 1.0f : 0.0f;

        SetWindowPos(hwnd, 0, static_cast<int>(window_pos.x), static_cast<int>(window_pos.y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
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

        _custom_font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(Hack_Regular, static_cast<float>(_settings->font_size));
        ImFont* default_font = io.Fonts->AddFontDefault();

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_InitForOpenGL(window_handle);
        ImGui_ImplOpenGL3_Init();

        // Our state
        // constexpr ImVec4 clear_color = ImVec4(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);
        constexpr ImVec4 clear_color = ImVec4(1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f);

        // Main loop
        bool done = false;
        for (int i = 0; i < _limit_ptr_vector.size(); i++)
        {
            Timer timer;
            timer_vector.push_back(timer);
        }

        fps_limit = std::make_unique<FrameRateLimiter>(_settings->fps);


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

            fps_limit->StartFrame();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
            ImGuiStyle& style = ImGui::GetStyle();
            style.FrameRounding = 0.0f;
            style.WindowPadding = ImVec2(15.0f, 5.0f);


            if (show_config)
            {
                ImGui::PushFont(default_font);
                Config(window_handle);
                ImGui::PopFont();
            }


            // custom_font->FontSize = _settings->font_size;
            if (show_overlay && (!_settings->auto_hide_overlay || GetForegroundWindow() == d2_hwnd))
            {
                ImGui::PushFont(_custom_font);
                Overlay(&show_overlay, window_handle);
                ImGui::PopFont();
            }
            else
            {
                // if case will hit until d2 is activated for the first time
                if (Helper::IsDestinyTheActiveWindow(nullptr)) // no logger cuz spam
                {
                    d2_hwnd = GetForegroundWindow();
                    logger->info("d2 hwnd set");
                }
            }

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
        if (hModule != 0)
        {
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
                // SetLayeredWindowAttributes(hWnd, RGB(128, 128, 128), 0, LWA_COLORKEY);
                SetLayeredWindowAttributes(hWnd, RGB(1, 1, 1), 0, LWA_COLORKEY);
                break;
            case WM_SYSCOMMAND:
                if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                {
                    return 0;
                }
                break;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
                if (!hk_instance->done)
                {
                    hk_instance->KeyboardInputHandler(static_cast<int>(wParam), true);
                }
                break;
            case WM_SYSKEYUP:
            case WM_KEYUP:
                if (!hk_instance->done)
                {
                    hk_instance->KeyboardInputHandler(static_cast<int>(wParam), false);
                    break;
                }
        }

        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    // source https://gist.github.com/enemymouse/c8aa24e247a1d7b9fc33d45091cbb8f0
    void UserInterface::ImGuiApplyTheme_EnemyMouse()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Alpha = 1.0;
        style.Colors[ImGuiCol_WindowBg].w = 0.83f;
        style.ChildRounding = 3;
        style.WindowRounding = 3;
        style.GrabRounding = 1;
        style.GrabMinSize = 20;
        style.FrameRounding = 3;
        style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.24f, 0.22f, 0.60f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.50f, 0.50f, 0.33f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.50f, 0.50f, 0.47f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.70f, 0.70f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.13f, 0.13f, 0.90f);
    }
}
