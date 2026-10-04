#pragma once

#include <windows.h>
#include "pxwEntryPoint.h"
#include "pxwXAudio2Keep.h"

// Application runtime state RAII manager; initializes common controls, COM, and the XAudio2 keep-alive to prevent xaudio2_7.dll from unloading early (known bug)
// Use by declaring one at the top of the entry point, before any goto
class pxwRuntime
{
	pxwXAudio2Keep_loadlib* _xa2_keep = nullptr;
	bool                    _com      = false;

public:
	 pxwRuntime() = default;
	~pxwRuntime();
	pxwRuntime( const pxwRuntime& ) = delete;
	pxwRuntime& operator=( const pxwRuntime& ) = delete;

	bool init( const TCHAR* app_name, int* p_exit_code );
};
