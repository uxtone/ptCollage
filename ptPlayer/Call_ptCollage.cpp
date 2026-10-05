
extern TCHAR g_dir_module[];

bool Find_ptCollage()
{
	TCHAR path[ MAX_PATH ];
	_stprintf_s( path, _T("%s\\ptCollage.exe"), g_dir_module );

	WIN32_FIND_DATA find ;
	HANDLE          hFind;

	hFind = FindFirstFile( path, &find );
	if( hFind == INVALID_HANDLE_VALUE ) return false;
	FindClose( hFind );

	return true;
}

bool Call_ptCollage( HWND hWnd, const TCHAR* path )
{
	HINSTANCE hShell;
	TCHAR     cmd[ MAX_PATH ];

	_stprintf_s( cmd, MAX_PATH, _T("%s\\ptCollage.exe"), g_dir_module );
	hShell = ShellExecute( hWnd, _T("open"), cmd, path, NULL, SW_SHOW );
	if( (INT_PTR)hShell <= 32 ) return false;

	return true;
}
