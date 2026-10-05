#pragma once

#define _WIN32_WINNT 0x0400

// ShellExecute / HDROP
// #define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <cstdint>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")
#include <commctrl.h>
#pragma comment(lib, "comctl32")
#pragma comment( lib, "imm32" )
#include              <imm.h>


enum PTP_SIGN
{
	PTP_SIGN_None = 0,
	PTP_SIGN_SelectFile,
};

#include <tchar.h>

#include <pxStdDef.h>
