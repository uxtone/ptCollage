#include <uxStr.h>
#include <uxStr.h>

extern uxSS<MAX_PATH> g_dir_module;

bool Find_ptCollage()
{
	uxSS<MAX_PATH> path;
	ux_sprintf_s( path, "%s\\ptCollage.exe", g_dir_module );

	WIN32_FIND_DATA find ;
	HANDLE          hFind;

	hFind = FindFirstFile( uxT( path ), &find );
	if( hFind == INVALID_HANDLE_VALUE ) return false;
	FindClose( hFind );

	return true;
}

bool Call_ptCollage( HWND hWnd, const uxDS& path )
{
	HINSTANCE hShell;
	uxSS<MAX_PATH>     cmd;

	ux_sprintf_s( cmd, MAX_PATH, "%s\\ptCollage.exe", g_dir_module );
	hShell = ShellExecute( hWnd, uxT( "open" ), uxT( cmd ), uxT( path ), NULL, SW_SHOW );
	if( (INT_PTR)hShell <= 32 ) return false;

	return true;
}
