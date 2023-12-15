// for some reason the .h file path would never resolve for this file, hopefully i can split it after some vs update
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


class UserInterface {
public:
    UserInterface(std::vector<limit*> limit_ptr_vector); 
    int run_gui();
    static UserInterface* instance;
    static HotkeyManager* hotkeyInstance;

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
};


#endif USERINTERFACE_H
