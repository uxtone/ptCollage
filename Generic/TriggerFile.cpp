


//#include "FilePath.h"
//#include <pxwfilepath

uxDS _p_temporary_path;

void TriggerFile_SetTemporaryPath( const uxDS& path )
{
	_p_temporary_path = path;
}

// ステータスフォルダにキーファイルを作る
bool TriggerFile_Make( const uxDS& name )
{
	uxSS<MAX_PATH> path;
	FILE *fp;

	ux_sprintf_s( path, MAX_PATH, "%s\\%s", *_p_temporary_path, *name );
	if( !( fp = ux_fopen( path, "wb" ) ) ) return false;
	fclose( fp );

	return true;
}

// ステータスフォルダにキーファイルを確認する
bool TriggerFile_Is( const uxDS& name )
{
	uxSS<MAX_PATH> path;
	FILE *fp;

	ux_sprintf_s( path, MAX_PATH, "%s\\%s", *_p_temporary_path, *name );
	if( !( fp = ux_fopen( path, "rb" ) ) ) return false;
	fclose( fp );

	return true;
}

// ステータスフォルダのキーファイルを消す
void TriggerFile_Delete( const uxDS& name )
{
	uxSS<MAX_PATH> path;

	ux_sprintf_s( path, MAX_PATH, "%s\\%s", *_p_temporary_path, *name );
	SetFileAttributes( uxT( path ), FILE_ATTRIBUTE_NORMAL );
	DeleteFile( uxT( path ) );
}
