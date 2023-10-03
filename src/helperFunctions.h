#pragma once
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <psapi.h>


bool isD2Active();
const wchar_t* GetFileName( const wchar_t *path );