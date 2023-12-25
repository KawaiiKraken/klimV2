#ifndef USERINTERFACE_H
#define USERINTERFACE_H
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include <windows.h>
#include <GL/GL.h>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include "HotkeyManager.h"

struct Settings;

class UserInterface {
public:
    UserInterface(std::vector<limit*> limit_ptr_vector, wchar_t* path_to_config_file, Settings* settings); 
    int run_gui();
    static UserInterface* instance;
    static HotkeyManager* hotkeyInstance;
	bool show_overlay = false;
    bool show_config = false;

private:
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    std::vector<limit*> limit_ptr_vector;
    // Data stored per platform window
	struct WGL_WindowData { HDC hDC; };
    // Data
	WGL_WindowData   g_MainWindow;
	HGLRC            g_hRC = 0;
    int              g_Width;
	int              g_Height;
    // Forward declarations of helper functions
	bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
	void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
	void ResetDeviceWGL();
    wchar_t* path_to_config_file;
    Settings* settings;
    void Overlay(bool* p_open, HWND hwnd);
    void Config(HWND hwnd);
};


#endif USERINTERFACE_H
