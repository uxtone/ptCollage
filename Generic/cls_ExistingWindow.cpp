
#include "cls_ExistingWindow.h"
// #include <errhandlingapi.h>

cls_EXISTINGWINDOW::cls_EXISTINGWINDOW()
{
	_hMapping   = NULL;
	_hMutex     = NULL;
	_p_hwnd_map = NULL;
}

cls_EXISTINGWINDOW::~cls_EXISTINGWINDOW()
{
	if( _hMutex     ) ReleaseMutex(    _hMutex     );
	if( _hMapping   ) CloseHandle(     _hMapping   );
	if( _p_hwnd_map ) UnmapViewOfFile( _p_hwnd_map );
}


static void _ForgrandExistingWindow( const TCHAR *mapping_name, UINT msg )
{
	// マテックスが既に有るなら、既存ウインドウを表示してfalseを返す
	HANDLE  hMapping    = NULL;
	HWND    *p_hwnd_map = NULL;

	hMapping = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND), mapping_name );
	if( !hMapping ) return;

	if( GetLastError() != ERROR_ALREADY_EXISTS )
	{
		CloseHandle( hMapping );
		return;
	}

	p_hwnd_map = (HWND *)MapViewOfFile( hMapping, FILE_MAP_WRITE, 0, 0, sizeof(HWND) );
	if( !p_hwnd_map ){
		CloseHandle( hMapping );
		return;
	}

	if( IsWindowVisible(       *p_hwnd_map ) ){
		SetForegroundWindow(   *p_hwnd_map );
		ShowWindow(            *p_hwnd_map, SW_NORMAL );
		if( msg ) SendMessage( *p_hwnd_map, msg, 0, 0 );
	}

	UnmapViewOfFile( p_hwnd_map );
	CloseHandle(     hMapping   );
}


bool cls_EXISTINGWINDOW::Check( const TCHAR  *mutex_name, const TCHAR *mapping_name, UINT msg )
{
	// マテックスが無ければ生成して終わり
	_hMutex = OpenMutex( MUTEX_ALL_ACCESS, false, mutex_name );
	if( !_hMutex ){
		_hMutex = CreateMutex( nullptr, 0, mutex_name );
		if( !_hMutex ) return false;
		return true;
	}

	_ForgrandExistingWindow( mapping_name, msg );
	return false;
}

bool cls_EXISTINGWINDOW::Mapping( const TCHAR  *mutex_name, const TCHAR *mapping_name, HWND hWnd )
{
	_hMutex = CreateMutex(NULL, 0, mutex_name );
	if( !_hMutex     ) return false;

	_hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND), mapping_name );
	if( !_hMapping   ){
		ReleaseMutex(    _hMutex     ); _hMutex = NULL;
		return false;
	}

	_p_hwnd_map = (HWND *)MapViewOfFile( _hMapping, FILE_MAP_WRITE, 0, 0, sizeof(HWND) );
	if( !_p_hwnd_map ){
		ReleaseMutex(    _hMutex     ); _hMutex   = NULL;
		CloseHandle(     _hMapping   ); _hMapping = NULL;

		return false;
	}

	*_p_hwnd_map = hWnd;

	return true;

}
