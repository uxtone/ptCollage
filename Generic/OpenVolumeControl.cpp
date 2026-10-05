

// サウンドボリュームを開く
bool OpenVolumeControl( HWND hWnd )
{
	HINSTANCE hShell1;
	HINSTANCE hShell2;
	TCHAR path [MAX_PATH];
	TCHAR path1[MAX_PATH];
	TCHAR path2[MAX_PATH];

	GetSystemDirectory( path, MAX_PATH );
	_stprintf_s( path1, MAX_PATH, _T("%s\\Sndvol32.exe"), path );
	_tcscpy(  path2, path );
	PathRemoveFileSpec( path2 );
	_tcscat(  path2,              _T("\\Sndvol32.exe"  ) );

	hShell1 = ShellExecute( hWnd,_T("open"),path1,NULL,NULL,SW_SHOW);
	hShell2 = ShellExecute( hWnd,_T("open"),path2,NULL,NULL,SW_SHOW);
	if( (uintptr_t)hShell1 <= 32 && (uintptr_t)hShell2 <= 32 ) return false;

	return true;
}
