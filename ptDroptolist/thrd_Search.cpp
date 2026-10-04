
#include <pxwFilePath.h>

#include "resource.h"

#pragma comment(lib,"version")

// リソースバージョンを取得
int32_t GetCompileVersion( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 )
{
	LPVOID           p       = NULL;
	DWORD            dummy;
	DWORD            size;
	VS_FIXEDFILEINFO *info;
	UINT             vSize;
	TCHAR            path[ MAX_PATH ];

	int32_t v[ 4 ] = {0};

	if( p1 ) *p1 = 0;
	if( p2 ) *p2 = 0;
	if( p3 ) *p3 = 0;
	if( p4 ) *p4 = 0;

	GetModuleFileName( NULL, path, MAX_PATH );

	size = GetFileVersionInfoSize( path, &dummy );
	if( !size ) goto End;

	p = malloc( size );
	if( !p                                                    ) goto End;
	if( !GetFileVersionInfo( path, 0, size, p )               ) goto End;
	if( !VerQueryValue( p, _T("\\"), (LPVOID*)&info, &vSize ) ) goto End;

	v[ 0 ] = HIWORD(info->dwFileVersionMS);
	v[ 1 ] = LOWORD(info->dwFileVersionMS);
	v[ 2 ] = HIWORD(info->dwFileVersionLS);
	v[ 3 ] = LOWORD(info->dwFileVersionLS);

	if( p1 ) *p1 = v[ 0 ];
	if( p2 ) *p2 = v[ 1 ];
	if( p3 ) *p3 = v[ 2 ];
	if( p4 ) *p4 = v[ 3 ];

End:
	if( p ) free( p );

	return v[ 0 ] * 1000 + v[ 1 ] * 100 + v[ 2 ] * 10 + v[ 3 ];
}


#include <pxtnService.h>
pxtnService *g_pxtn = NULL;


typedef struct
{
	TCHAR* p_path;
}
_PROJECTSTRUCT;

extern HWND g_hDlg;

static bool _FindProjectFiles( const TCHAR* dir_start, TCHAR *dir_ext, vector <_PROJECTSTRUCT>* p_v, bool* pbSuspend )
{
	WIN32_FIND_DATA wfd;
	TCHAR           path[ MAX_PATH ];
	HANDLE          hFind = NULL;
	_PROJECTSTRUCT  project;

	if( _tcslen( dir_ext ) ) _stprintf_s( path, MAX_PATH, _T("%s%s\\*"), dir_start, dir_ext );
	else                     _stprintf_s( path, MAX_PATH, _T("%s\\*"  ), dir_start          );

	hFind = FindFirstFile( path, &wfd );
	while( hFind != INVALID_HANDLE_VALUE )
	{
		// 中止
		if( *pbSuspend ){ FindClose( hFind ); return false; }

		if(_tcsicmp( wfd.cFileName, _T(".") ) &&_tcsicmp( wfd.cFileName, _T("..") ) )
		{
			// ディレクトリなら再起
			if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( _tcslen( dir_ext ) ) _tcscpy( path, dir_ext );
				else                     _tcscpy( path, _T("")  );
				_tcscat( path, _T("\\")      );
				_tcscat( path, wfd.cFileName );
				if( !_FindProjectFiles( dir_start, path, p_v, pbSuspend ) ){ FindClose( hFind ); return false; }
			}
			// ファイルなら登録
			else
			{
				if( !_tcsicmp( PathFindExtension( wfd.cFileName ), _T(".ptcop" ) ) ||
					!_tcsicmp( PathFindExtension( wfd.cFileName ), _T(".pttune") ) )
				{
					if( _tcslen( dir_ext ) ) _stprintf_s( path, _T("%s\\%s"), dir_ext, wfd.cFileName );
					else                     _stprintf_s( path, _T("\\%s"  ),          wfd.cFileName );
					project.p_path = (TCHAR*)malloc( (_tcslen( path ) + 1) * sizeof(TCHAR) );
					if( !project.p_path ){ FindClose( hFind ); return false; }
					_tcscpy( project.p_path, path );
					p_v->push_back( project );

					SetDlgItemInt( g_hDlg, IDC_FINDNUM, p_v->size(), false );
				}
			}
		}
		if( !FindNextFile( hFind, &wfd ) ){ FindClose( hFind ); break; }
	}

	return true;
}


/*
// pttune -> ptcop
bool pxtnService::PttuneToPtcop()
{
	if( !_b_init ) return false;
	int32_t beat_clock = master->get_beat_clock();
	if( beat_clock != EVENTDEFAULT_BEATCLOCK )
	{
		int32_t rate = EVENTDEFAULT_BEATCLOCK / beat_clock;
		master->set_beat_clock( EVENTDEFAULT_BEATCLOCK );
		evels ->BeatClockOperation( rate );
	}
	return true;
}
*/

static bool _ConvertPTTUNEtoPTCOP( const TCHAR *dir_start, const TCHAR *path_ext, const TCHAR *dir_dst )
{
	bool b_ret = false;

	// load..
	{
		TCHAR path_src[ MAX_PATH ] = {0}; _stprintf_s( path_src, MAX_PATH, _T("%s%s"), dir_start, path_ext );
		FILE* fp = _tfopen( path_src, _T("rb") ); if( !fp ) return false;
		if( !g_pxtn->read(fp ) ){ fclose( fp ); goto End; }
		fclose( fp );
	}

	goto End;
	// beat clock 24,48 -> 480
//	if( !g_pxtn->PttuneToPtcop() ) goto End;

	// save..
	{
		TCHAR path_dst[ MAX_PATH ];
		_stprintf_s( path_dst, _T("%s%s"), dir_dst, path_ext ); PathRemoveFileSpec ( path_dst ); CreateDirectory( path_dst, NULL );
		_stprintf_s( path_dst, _T("%s%s"), dir_dst, path_ext ); PathRemoveExtension( path_dst );
		_tcscat    ( path_dst, _T(".ptcop") );

		FILE* fp = _tfopen( path_dst, _T("wb") ); if( !fp ) return false;
		if( !g_pxtn->write( fp, false, (unsigned short)GetCompileVersion( 0, 0, 0, 0 ) ) )
		{
			fclose( fp ); goto End;
		}
		fclose( fp );
	}

	b_ret = true;
End:
	return b_ret;
}

DWORD CALLBACK thrd_Search( LPVOID l )
{
	bool* pbSuspend = (bool*)l;
	static vector <_PROJECTSTRUCT> vProject;
	int32_t  ok_num    = 0;
	int32_t  found_num = 0;

	TCHAR dir_src[ MAX_PATH ] = {0};
	TCHAR dir_dst[ MAX_PATH ] = {0};
	TCHAR dir_ext[ MAX_PATH ] = {0};

	FILE *fp_csv = NULL;

	SetDlgItemInt(  g_hDlg, IDC_OKNUM, 0, false );
	GetDlgItemText( g_hDlg, IDC_PATH, dir_src, MAX_PATH );
	if( !_tcslen( dir_src ) ) goto End;

	_tcscpy( dir_dst, dir_src );
	_tcscat( dir_dst, _T("_converted") );

	CreateDirectory( dir_dst, NULL );

	{
		TCHAR path_csv[ MAX_PATH ] = {0};
		_stprintf_s( path_csv, MAX_PATH, _T("%s\\results.csv"), dir_dst );
		fp_csv = _tfopen( path_csv, _T("wt") );
	}

	// ファイルを検索
	if( !_FindProjectFiles( dir_src, dir_ext, &vProject, pbSuspend ) ) goto End;

	found_num = vProject.size();

	for( int32_t v = 0; v < found_num; v++ )
	{
//			if( CheckPxtoneProject_OutputRecord( fp, vProject.at( v ).p_path ) )
		if( _ConvertPTTUNEtoPTCOP( dir_src, vProject.at( v ).p_path, dir_dst ) )
		{
			SetDlgItemInt( g_hDlg, IDC_OKNUM, ++ok_num, false );
			if( fp_csv ) _ftprintf( fp_csv, _T("OK ,%s\n"), PathFindFileName( vProject.at( v ).p_path ) );
		}
		else
		{
			if( fp_csv ) _ftprintf( fp_csv, _T("ERR,%s\n"), PathFindFileName( vProject.at( v ).p_path ) );
		}
	}


End:

	if( fp_csv ) fclose( fp_csv ); fp_csv = NULL;

	// 開放
	for( uint32_t i = 0; i < vProject.size(); i++ ){ if( vProject.at( i ).p_path ) free( vProject.at( i ).p_path ); }

	SetDlgItemText( g_hDlg, IDC_STOP, _T("Exit") );

	while( *pbSuspend == false ){ Sleep( 1 ); }

	EndDialog( g_hDlg, true );

	return 1;
}


// I/O..
static bool _io_read( void* user, void* p_dst, int32_t size, int32_t num )
{
	if( fread( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}
static bool _io_write( void* user, const void* p_dst, int32_t size, int32_t num )
{
	if( fwrite( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}
static  bool _io_seek( void* user,       int   mode , int32_t size              )
{
	if( fseek( (FILE*)user, size, mode ) ) return false; return true;
}
static bool _io_pos( void* user, int32_t* p_pos )
{
	fpos_t sz{};
	if( fgetpos( (FILE*)user, &sz ) ) return false;
	*p_pos  = pxFPOS_OFFSET(sz);
	return true;
}


bool thrd_Search_Init()
{
	g_pxtn = new pxtnService( _io_read, _io_write, _io_seek, _io_pos );
	if( g_pxtn->init() != pxtnOK ) return false;
	return true;
}

void thrd_Search_Release()
{
	SAFE_DELETE( g_pxtn );
}
