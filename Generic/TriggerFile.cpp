
//#include "FilePath.h"
//#include <pxwfilepath

extern const TCHAR *_p_temporary_path = NULL;

void TriggerFile_SetTemporaryPath( const TCHAR *path )
{
	_p_temporary_path = path;
}

// ステータスフォルダにキーファイルを作る
bool TriggerFile_Make( const TCHAR *name )
{
	TCHAR path[MAX_PATH];
	FILE *fp;

	_stprintf_s( path, MAX_PATH, _T("%s\\%s"), _p_temporary_path, name );
	if( !( fp = _tfopen( path, _T("wb") ) ) ) return false;
	fclose( fp );

	return true;
}

// ステータスフォルダにキーファイルを確認する
bool TriggerFile_Is( const TCHAR *name )
{
	TCHAR path[MAX_PATH];
	FILE *fp;

	_stprintf_s( path, MAX_PATH, _T("%s\\%s"), _p_temporary_path, name );
	if( !( fp = _tfopen( path, _T("rb") ) ) ) return false;
	fclose( fp );

	return true;
}

// ステータスフォルダのキーファイルを消す
void TriggerFile_Delete( const TCHAR *name )
{
	TCHAR path[MAX_PATH];

	_stprintf_s( path, MAX_PATH, _T("%s\\%s"), _p_temporary_path, name );
	SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );
	DeleteFile( path );
}
