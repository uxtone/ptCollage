



#include <pxPath.h>


// サウンドボリュームを開く
bool OpenVolumeControl( HWND hWnd )
{
	HINSTANCE hShell1;
	HINSTANCE hShell2;
	uxSS<MAX_PATH> path;
	uxSS<MAX_PATH> path1;
	uxSS<MAX_PATH> path2;

	GetSystemDirectory( uxTOut( path ), MAX_PATH );
	ux_sprintf_s( path1, MAX_PATH, "%s\\Sndvol32.exe", path );
	strcpy(  path2, path );
	pxPath_remove_filename( path2 );
	strcat(  path2,              "\\Sndvol32.exe"   );

	hShell1 = ShellExecute( hWnd,uxT( "open" ),uxT( path1 ),NULL,NULL,SW_SHOW);
	hShell2 = ShellExecute( hWnd,uxT( "open" ),uxT( path2 ),NULL,NULL,SW_SHOW);
	if( (uintptr_t)hShell1 <= 32 && (uintptr_t)hShell2 <= 32 ) return false;

	return true;
}
