#include <pxwFilePath.h>
#include <pxwWindowRect.h>
#include <pxwFilePath.h>

#include "../Generic/Menu_History.h"

#define PXTONETOOL_EXPORTS
#include "./pxtoneTool.h"

#include "./resource.h"

#include "./Japanese.h"

#define MAX_DRIVENUM    10
#define DRIVEBUFFERSIZE ( MAX_DRIVENUM * 4 + 2 )

extern HINSTANCE g_hInst;
static HWND      _hList         = NULL;
static UINT_PTR  _timer_id      = 100;

static int32_t   _visible_flags =    0;
static bool      _b_japanese    = false;

static TCHAR*    _rect_name     = _T("heardlg.rect");

static pxFile2*  _ref_file_profile;

enum _FILETYPE
{
	_FILETYPE_DRIVE_FIXED = 0,
	_FILETYPE_DRIVE_CDROM,
	_FILETYPE_DRIVE_REMOVE,
	_FILETYPE_DRIVE_X,
	_FILETYPE_CLOSE,
	_FILETYPE_FOLDER,
	_FILETYPE_PCM ,
	_FILETYPE_PTV ,
	_FILETYPE_PTN ,
	_FILETYPE_OGGV,
	_FILETYPE_LNK ,
};

enum _SORTMODE
{
	_SORTMODE_NAME = 0,
	_SORTMODE_EXT,
};

typedef struct
{
	int    image ;
	TCHAR* p_name;

}_FILERECORDSTRUCT;

static	vector <_FILERECORDSTRUCT> _v_record;

static pxtonewinWoice* _woi = NULL;

enum
{
	COLUMN_NAME = 0,
};


static void _ClearRecords()
{
	for( uint32_t v = 0; v < _v_record.size(); v++ ) free( _v_record.at( v ).p_name );
	_v_record.clear();
}

static void _Resort( HWND hDlg )
{
	_FILERECORDSTRUCT record;
	LV_ITEM           item = {0};
	int32_t           index = 0;
	TCHAR             path[ MAX_PATH ];

	ListView_DeleteAllItems( _hList );

	GetDlgItemText( hDlg, IDC_DIRECTORYPATH, path, MAX_PATH );

	if( PathIsDirectory( path ) )
	{
		item.mask     = LVIF_TEXT | LVIF_IMAGE;
		item.iSubItem = COLUMN_NAME;
		item.pszText  = _T("..");
		item.iItem    = 0;
		item.iImage   = _FILETYPE_CLOSE;
		ListView_InsertItem( _hList, &item );

		index++;
	}

	if( !_v_record.size() ) return;
	{
		uint32_t top = (uint32_t)_v_record.size() - 1;

		while( top )
		{
			for( unsigned int v = 0; v < top; v++ )
			{
				if(_tcsicmp( _v_record.at( v ).p_name, _v_record.at( v + 1 ).p_name ) > 0 )
				{
					record               = _v_record.at( v     );
					_v_record.at( v     ) = _v_record.at( v + 1 );
					_v_record.at( v + 1 ) = record;
				}
			}
			top--;
		}
	}

	if( SendDlgItemMessage( hDlg, IDC_COMBO_SORT, CB_GETCURSEL, 0, 0 ) )
	{
		uint32_t top = (uint32_t)_v_record.size() - 1;

		while( top )
		{
			for( unsigned int v = 0; v < top; v++ )
			{
				if(_tcsicmp(
					PathFindExtension( _v_record.at( v     ).p_name ),
					PathFindExtension( _v_record.at( v + 1 ).p_name ) ) > 0 )
				{
					record               = _v_record.at( v     );
					_v_record.at( v     ) = _v_record.at( v + 1 );
					_v_record.at( v + 1 ) = record;
				}
			}
			top--;
		}
	}



	for( uint32_t v = 0; v < _v_record.size(); v++ )
	{
		item.mask     = LVIF_TEXT | LVIF_IMAGE;
		item.iSubItem = COLUMN_NAME;
		item.pszText  = _v_record.at( v ).p_name;
		item.iImage   = _v_record.at( v ).image ;
		item.iItem    = index++;
		ListView_InsertItem( _hList, &item );
	}

}


static TCHAR *_file_name_lastdirectory = _T("load_voice.lastdirectory");
static TCHAR *_file_name_flags         = _T("load_voice.flags"        );

static bool _LastFolder_Load( TCHAR* path_last )
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_r( &desc, _file_name_lastdirectory, NULL, NULL ) ) goto term;
	if( !desc->r( path_last, sizeof(TCHAR), MAX_PATH ) ) goto term;
	if( !PathIsDirectory( path_last ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );

	return b_ret;
}

static bool _LastFolder_Save( const TCHAR* path_last )
{
	bool  b_ret = false;
	TCHAR path[ MAX_PATH ];

	pxDescriptor* desc  = NULL ;

	_tcscpy( path, path_last );

	while( !PathIsDirectory( path ) ){ PathRemoveFileSpec( path ); if( !_tcslen( path ) ) goto term; }

	if( !_ref_file_profile->open_w( &desc, _file_name_lastdirectory, NULL, NULL ) ) goto term;
	if( !desc->w_asfile( path, sizeof(TCHAR), MAX_PATH ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}

static bool _StatusFlag_Save( int    flags )
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _file_name_flags, NULL, NULL ) ) goto term;
	if( !desc->w_asfile( &flags, sizeof(int), 1 ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}

static bool _StatusFlag_Load( int* p_flags )
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _file_name_flags, NULL, NULL ) ) goto term;
	if( !desc->r( p_flags, sizeof(int), 1 )             ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );

	return b_ret;
}


// パスを指定してリスト/カレントパスを更新
static bool _RefreshList( HWND hDlg )
{
	bool              b_ret = false;
	HANDLE            hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA   ffd;
	TCHAR             path[ MAX_PATH ];
	TCHAR             directory_path[ MAX_PATH ];

	TCHAR*            p_ext   ;
	bool              b_insert;
	_FILERECORDSTRUCT record  ;
	int32_t           size    ;

	_ClearRecords();

	GetDlgItemText( hDlg, IDC_DIRECTORYPATH, directory_path, MAX_PATH );
	if( PathIsDirectory( directory_path ) )
	{
		_stprintf_s( path, MAX_PATH, _T("%s\\*.*"), directory_path );

		hFind = FindFirstFile( path, &ffd );
		while( hFind != INVALID_HANDLE_VALUE )
		{
			p_ext = PathFindExtension( ffd.cFileName );
			if(      !_tcscmp( ffd.cFileName, _T(".") ) || !_tcscmp( ffd.cFileName, _T("..") ) ) b_insert = false;
			else if( ffd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM                    ) b_insert = false;
			else if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY                 ) b_insert = true ;
			else if( _visible_flags & HEARSELECTVISIBLE_PCM  && !_tcsicmp( p_ext, _T(".wav"    ) ) ) b_insert = true ;
			else if( _visible_flags & HEARSELECTVISIBLE_PTV  && !_tcsicmp( p_ext, _T(".ptvoice") ) ) b_insert = true ;
			else if( _visible_flags & HEARSELECTVISIBLE_PTN  && !_tcsicmp( p_ext, _T(".ptnoise") ) ) b_insert = true ;
			else if( _visible_flags & HEARSELECTVISIBLE_OGGV && !_tcsicmp( p_ext, _T(".ogg"    ) ) ) b_insert = true ;
			else if( !_tcsicmp( p_ext, _T(".lnk") ) )
			{
				TCHAR path_dst[ MAX_PATH ];
				_stprintf_s( path, MAX_PATH, _T("%s\\%s"), directory_path, ffd.cFileName );
				if( pxwFilePath_GetShortcutDirectory( path, path_dst ) ) b_insert = true ;
				else                                                     b_insert = false;
			}
			else b_insert = false;

			if( b_insert )
			{
				if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) record.image = _FILETYPE_FOLDER;
				else if( !_tcsicmp( p_ext, _T(".wav"    ) )         ) record.image = _FILETYPE_PCM   ;
				else if( !_tcsicmp( p_ext, _T(".ptvoice") )         ) record.image = _FILETYPE_PTV   ;
				else if( !_tcsicmp( p_ext, _T(".ptnoise") )         ) record.image = _FILETYPE_PTN   ;
				else if( !_tcsicmp( p_ext, _T(".ogg"    ) )         ) record.image = _FILETYPE_OGGV  ;
				else if( !_tcsicmp( p_ext, _T(".lnk"    ) )         ) record.image = _FILETYPE_LNK   ;

				size = (int32_t)_tcslen( ffd.cFileName ) + 1;
				if( !( record.p_name = (TCHAR*)malloc( size * sizeof(TCHAR) ) ) ) goto End;
				_tcscpy( record.p_name, ffd.cFileName );
				_v_record.push_back( record );
			}

			if( !FindNextFile( hFind, &ffd ) ){ FindClose( hFind ); hFind = INVALID_HANDLE_VALUE; break; }
		}

	}
	// ドライブ一覧
	else
	{

		int i;
		TCHAR buf[ DRIVEBUFFERSIZE ];
		UINT drive;

		memset( buf, 0, DRIVEBUFFERSIZE );
		GetLogicalDriveStrings( DRIVEBUFFERSIZE, buf );


		for( i = 0; i < DRIVEBUFFERSIZE; i += 4 )
		{
			if( !buf[ i ] ) break;
			// ハードディスクのみ認識
			drive = GetDriveType( &buf[ i ] );
			if( drive == DRIVE_FIXED || drive == DRIVE_CDROM || drive == DRIVE_REMOVABLE || drive == DRIVE_RAMDISK )
			{
				switch( drive )
				{
				case DRIVE_FIXED    : record.image = _FILETYPE_DRIVE_FIXED ; break;
				case DRIVE_CDROM    : record.image = _FILETYPE_DRIVE_CDROM ; break;
				case DRIVE_REMOVABLE: record.image = _FILETYPE_DRIVE_REMOVE; break;
				case DRIVE_RAMDISK  : record.image = _FILETYPE_DRIVE_X     ; break;
				}
				size = (int32_t)_tcslen(  &buf[ i ] ) + 1;
				if( !( record.p_name = (TCHAR*)malloc( size * sizeof(TCHAR) ) ) ) goto End;
				_tcscpy( record.p_name,  &buf[ i ] );
				_v_record.push_back( record );
			}
		}
	}


	_Resort( hDlg );

	b_ret = true;
End:
	if( hFind != INVALID_HANDLE_VALUE ) FindClose( hFind );
	if( !b_ret )
	{
		_ClearRecords();
		ListView_DeleteAllItems( _hList );
	}


	return b_ret;
}


#define _CTRL_TEXT_H  32

#define _CTRL_INFO_H 128
#define _CTRL_SIDE_W 208
#define _CTRL_SORT_LBL_W   64
#define _CTRL_SORT_CMB_W  104

#define _CTRL_KEY_LBL_W  48
#define _CTRL_KEY_VAL_W  56
#define _CTRL_KEY_DRG_W  64

static void _update_layout( HWND hdlg )
{
	RECT rc_clnt = {0};
	RECT rc      = {0};
	HWND hw = 0;

	GetClientRect( hdlg, &rc_clnt );

	SendMessage( hdlg, WM_SETREDRAW, 0, 0 );

	// path..
	{
		hw = GetDlgItem( hdlg, IDC_DIRECTORYPATH );
		rc.left   = rc_clnt.left   +   8;
		rc.top    = rc_clnt.top    +   8;
		rc.right  = rc_clnt.right  -   8;
		rc.bottom = rc.top         +  _CTRL_TEXT_H;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}

	// list..
	{
		hw = GetDlgItem( hdlg, IDC_LIST );
		rc.left   = rc_clnt.left   +   8;
		rc.top    = rc_clnt.top    +  48;
		rc.right  = rc_clnt.right  - _CTRL_SIDE_W;
		rc.bottom = rc_clnt.bottom -  12;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}

	// sort..
	{
		hw = GetDlgItem( hdlg, IDC_TEXT_SORT );
		rc.left   = rc_clnt.right  - _CTRL_SIDE_W + 8;
		rc.top    = rc_clnt.top    +  48;
		rc.right  = rc.left        + _CTRL_SORT_LBL_W;
		rc.bottom = rc.top         + _CTRL_TEXT_H;
		SetWindowPos( hw, NULL, rc.left, rc.top + 2, rc.right - rc.left, rc.bottom - rc.top, 0 );

		LONG old_btm = rc.bottom;

		hw = GetDlgItem( hdlg, IDC_COMBO_SORT );
		rc.left   = rc.right       +   8;
		rc.right  = rc.left        + _CTRL_SORT_CMB_W;
		rc.top    += 8;
		rc.bottom = rc.top         + _CTRL_TEXT_H * 3;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );

		rc.bottom = old_btm;
	}

	// infomation..
	{
		hw = GetDlgItem( hdlg, IDC_INFORMATION );
		rc.left   = rc_clnt.right  - _CTRL_SIDE_W + 8;
		rc.top    = rc.bottom      +   8;
		rc.right  = rc_clnt.right  -   8;
		rc.bottom = rc.top         + _CTRL_INFO_H;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}

	// add unit..
	{
		hw = GetDlgItem( hdlg, IDC_CHECK_ADDUNIT );
		rc.left   = rc_clnt.right  - _CTRL_SIDE_W + 8;
		rc.top    = rc.bottom      +   8;
		rc.right  = rc_clnt.right  -   8;
		rc.bottom = rc.top         +  _CTRL_TEXT_H;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}

	// loop..
	{
		hw = GetDlgItem( hdlg, IDC_CHECK_LOOP );
		rc.left   = rc_clnt.right  - _CTRL_SIDE_W + 8;
		rc.top    = rc.bottom      +   8;
		rc.right  = rc_clnt.right  -   8;
		rc.bottom = rc.top         +  _CTRL_TEXT_H;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}

	// key..
	{
		hw = GetDlgItem( hdlg, IDC_TEXT_KEY );
		rc.left   = rc_clnt.right  - _CTRL_SIDE_W + 8;
		rc.top    = rc.bottom      +   8;
		rc.right  = rc.left        +  _CTRL_KEY_LBL_W;
		rc.bottom = rc.top         +  _CTRL_TEXT_H;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );

		hw = GetDlgItem( hdlg, IDC_KEY      );
		rc.left   = rc.right       +   8;
		rc.right  = rc.left        +  _CTRL_KEY_VAL_W;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );

		hw = GetDlgItem( hdlg, IDC_KEY_DRAG );
		rc.left   = rc.right       +   8;
		rc.right  = rc.left        +  _CTRL_KEY_DRG_W;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}

	// ok / cancel.
	{
		hw = GetDlgItem( hdlg, IDCANCEL );
		rc.left   = rc_clnt.right  - _CTRL_SIDE_W + 8;
		rc.right  = rc_clnt.right  -   8;
		rc.bottom = rc_clnt.bottom -   8;
		rc.top    = rc.bottom      -  _CTRL_TEXT_H;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );

		hw = GetDlgItem( hdlg, IDOK );
		rc.bottom = rc.top         -   8;
		rc.top    = rc.bottom      -  _CTRL_TEXT_H;
		SetWindowPos( hw, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}

	SendMessage( hdlg, WM_SETREDRAW, 1, 0 );
	InvalidateRect( hdlg, NULL, TRUE );
}


static void _InitDialog( HWND hDlg, LPARAM l )
{
	//list------------------------------------------------------------
	LV_COLUMN column;
	HEARSELECTDIALOGSTRUCT* p_hear = (HEARSELECTDIALOGSTRUCT*)l;
	bool b_crnt_path = false;
	TCHAR dir_crnt[ MAX_PATH ] = {0};

	memset( &column, 0, sizeof(LV_COLUMN) );

	_hList = GetDlgItem( hDlg, IDC_LIST );

	// リスト項目--------
	column.mask     = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	column.fmt      = LVCFMT_LEFT;

	column.cx       = 192;
	column.pszText  = _T("name");
	column.iSubItem = COLUMN_NAME;
	ListView_InsertColumn( _hList, COLUMN_NAME, &column );

	//アイコンのロード
	HIMAGELIST hImage = ImageList_Create( 16, 16, ILC_COLOR8|ILC_MASK, 11, 1 );
	ListView_SetImageList( _hList, hImage, LVSIL_SMALL );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_DRIVE_FIXED" ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_DRIVE_CDROM" ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_DRIVE_REMOVE") ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_DRIVE_X"     ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_CLOSE"       ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_FOLDER"      ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_PCM"         ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_PTV"         ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_PTN"         ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_OGGV"        ) ) );
	ImageList_AddIcon( hImage, LoadIcon( g_hInst, _T("ICON_FILE_LNK"         ) ) );

	EnableWindow( GetDlgItem( hDlg, IDOK ), false );

	int status_flags = 0;

	if( !_StatusFlag_Load( &status_flags ) ) status_flags = 0;

	// current directory path
	if( _tcslen( p_hear->path_selected ) )
	{
		_tcscpy( dir_crnt, p_hear->path_selected );
		PathRemoveFileSpec( dir_crnt );
	}
	if( !PathIsDirectory( dir_crnt ) )
	{
		if( PathIsDirectory( p_hear->dir_default ) ) _tcscpy( dir_crnt, p_hear->dir_default );
		else
		{
			_LastFolder_Load( dir_crnt );
			if( !PathIsDirectory( dir_crnt ) ) pxwFilePath_GetDesktop( dir_crnt );
		}
	}

	const TCHAR* _sort_table_en[] = { _T("Name"), _T("Type") };
	const TCHAR* _sort_table_jp[] = { _T("名前"), _T("種類") };
	int i;

	if( p_hear->b_japanese ) for( i = 0; i < 2; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_SORT, CB_ADDSTRING, 0, (LPARAM)_sort_table_jp[ i ] );
	else                     for( i = 0; i < 2; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_SORT, CB_ADDSTRING, 0, (LPARAM)_sort_table_en[ i ] );

	if( status_flags & HEARSELECTSTATUS_SORT_TYPE ) SendDlgItemMessage( hDlg, IDC_COMBO_SORT, CB_SETCURSEL, 1, 0 );
	else                                            SendDlgItemMessage( hDlg, IDC_COMBO_SORT, CB_SETCURSEL, 0, 0 );

	if( status_flags & HEARSELECTSTATUS_ADDUNIT   ) CheckDlgButton    ( hDlg, IDC_CHECK_ADDUNIT, 1 );
	else                                            CheckDlgButton    ( hDlg, IDC_CHECK_ADDUNIT, 0 );
	if( p_hear->b_loop                            ) CheckDlgButton    ( hDlg, IDC_CHECK_LOOP   , 1 );
	else                                            CheckDlgButton    ( hDlg, IDC_CHECK_LOOP   , 0 );

	if( !( _visible_flags & HEARSELECTVISIBLE_ADDUNIT ) ) ShowWindow( GetDlgItem( hDlg, IDC_CHECK_ADDUNIT ), SW_HIDE );

	SetDlgItemText( hDlg, IDC_DIRECTORYPATH, dir_crnt );

	_RefreshList  ( hDlg );

	_update_layout( hDlg );
}

/*
	Select Case GetDriveType(p$)
	Case DRIVE_NODETERMINE_DRIVETYPE  szDriveType = "判断不能"
	Case DRIVE_NOEXIST_ROOTDIRECTORY  szDriveType = "ルートディレクトリなし"
	Case DRIVE_REMOVABLE              szDriveType = "ディスク着脱可能"
	Case DRIVE_FIXED                  szDriveType = "ハードディスク"
	Case DRIVE_REMOTE                 szDriveType = "リモート"
	Case DRIVE_CDROM                  szDriveType = "ＣＤ－ＲＯＭ"
	Case DRIVE_RAMDISK                szDriveType = "ＲＡＭ－ＤＩＳＫ"
*/

static bool _GetSelected( TCHAR *p_name, _FILETYPE* p_image_index )
{
	int32_t     index;
	LV_ITEM item ;

	index = ListView_GetNextItem( _hList, -1, LVNI_ALL | LVNI_SELECTED );
	if( index == -1 ) return false;

	// 情報を取得
	memset( &item, 0, sizeof(LV_ITEM) );
	item.mask       = LVIF_TEXT | LVIF_IMAGE;
	item.iItem      = index;
	item.iSubItem   = COLUMN_NAME;
	item.pszText    = p_name;
	item.cchTextMax = MAX_PATH;
	ListView_GetItem( _hList, &item );

	*p_image_index  = (_FILETYPE)item.iImage;
	return true;
}

static void _CloseDialog( HWND hDlg, HEARSELECTDIALOGSTRUCT *p_hear, bool bApply )
{
	if( bApply )
	{
		TCHAR     name[ MAX_PATH ];
		TCHAR     path[ MAX_PATH ];
		_FILETYPE image_index;

		if( !_GetSelected( name, &image_index ) ) return;

		GetDlgItemText( hDlg, IDC_DIRECTORYPATH, path, MAX_PATH );

		if( image_index != _FILETYPE_PCM  &&
			image_index != _FILETYPE_PTV  &&
			image_index != _FILETYPE_PTN  &&
			image_index != _FILETYPE_OGGV ) return;

		Menu_History_Add( path );
		_stprintf_s( p_hear->path_selected, MAX_PATH, _T("%s%s%s"), path,
					 pxwFilePath_IsDrive( path ) ? _T("") : _T("\\"), name );
	}
	else
	{
		GetDlgItemText( hDlg, IDC_DIRECTORYPATH, p_hear->path_selected, MAX_PATH );
	}

	int32_t status_flags = 0;

	if( IsDlgButtonChecked( hDlg, IDC_CHECK_ADDUNIT                  ) ) status_flags |=  HEARSELECTSTATUS_ADDUNIT  ;
	if( SendDlgItemMessage( hDlg, IDC_COMBO_SORT, CB_GETCURSEL, 0, 0 ) ) status_flags |=  HEARSELECTSTATUS_SORT_TYPE;
	p_hear->b_add_unit = IsDlgButtonChecked( hDlg, IDC_CHECK_ADDUNIT ) ? true : false;
	p_hear->b_loop     = IsDlgButtonChecked( hDlg, IDC_CHECK_LOOP    ) ? true : false;

	_StatusFlag_Save( status_flags          );
	_LastFolder_Save( p_hear->path_selected );

	Menu_History_Save();

	KillTimer( hDlg, _timer_id );

	if( bApply ) EndDialog( hDlg, true  );
	else         EndDialog( hDlg, false );
}

static void _Proc_NM_DBLCLK( HWND hDlg )
{
	TCHAR     name[ MAX_PATH ];
	TCHAR     path[ MAX_PATH ];
	_FILETYPE image_index;

	if( !_GetSelected( name, &image_index ) ) return;
	GetDlgItemText( hDlg, IDC_DIRECTORYPATH, path, MAX_PATH );

	SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	switch( image_index )
	{
	// drives.
	case _FILETYPE_DRIVE_FIXED :
	case _FILETYPE_DRIVE_CDROM :
	case _FILETYPE_DRIVE_REMOVE:
	case _FILETYPE_DRIVE_X     :
		_tcscpy( path, name );
		break;

	// close directory.
	case _FILETYPE_CLOSE:
		GetDlgItemText( hDlg, IDC_DIRECTORYPATH, path, MAX_PATH );
		if( _tcslen( path ) < 4 ) _tcscpy( path, _T("") );
		else                     PathRemoveFileSpec( path );
		break;

	// open directory.
	case _FILETYPE_FOLDER:
		GetDlgItemText( hDlg, IDC_DIRECTORYPATH, path, MAX_PATH );
		if( _tcslen( path ) > 3 ) _tcscat( path, _T("\\") );
		_tcscat( path, name );
		break;

	case _FILETYPE_PCM :
	case _FILETYPE_PTV :
	case _FILETYPE_PTN :
	case _FILETYPE_OGGV:
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
		SendMessage( hDlg, WM_COMMAND, IDOK, NULL );
		return;

	case _FILETYPE_LNK:
		_tcscat( path, _T("\\") ); _tcscat( path, name );
		if( !pxwFilePath_GetShortcutDirectory( path, name ) )
		{
			SetCursor( LoadCursor( NULL, IDC_ARROW ) );
			return;
		}
		_tcscpy( path, name );
		break;
	}

	SetDlgItemText( hDlg, IDC_DIRECTORYPATH, path );
	_RefreshList( hDlg );
	SetCursor( LoadCursor( NULL, IDC_ARROW ) );
}

// change selected item.
static void _Proc_LVN_ITEMCHANGED( HWND hDlg )
{
	TCHAR     path[MAX_PATH];
	TCHAR     name[MAX_PATH];
	_FILETYPE image_index;

	EnableWindow( GetDlgItem( hDlg, IDOK), false );
	SetDlgItemText( hDlg, IDC_INFORMATION, _T("") );

	if( !_GetSelected( name, &image_index ) ) return;
	if( image_index != _FILETYPE_PCM  &&
		image_index != _FILETYPE_PTV  &&
		image_index != _FILETYPE_PTN  &&
		image_index != _FILETYPE_OGGV ) return;

	// 停止と解放
	KillTimer( hDlg, _timer_id );

	_woi->stop( true );

	GetDlgItemText( hDlg, IDC_DIRECTORYPATH, path, MAX_PATH );
	if(     path[ _tcslen( path ) - 1 ] != '\\' ) _tcscat( path, _T("\\") );
	_tcscat( path, name );

	// PTVを再生する

	bool b_timer = false;
	bool b_loop  = IsDlgButtonChecked( hDlg, IDC_CHECK_LOOP ) ? true : false;
	int  key     = GetDlgItemInt( hDlg, IDC_KEY, NULL, true ) * 0x100;
	switch( image_index )
	{
	case _FILETYPE_PCM :
	case _FILETYPE_PTV :
	case _FILETYPE_PTN :
	case _FILETYPE_OGGV:
		SetCursor( LoadCursor( NULL, IDC_WAIT ) );

		if( _woi->load_and_play( path, b_loop, key, &b_timer ) )
		{
			if( b_timer ) _timer_id = SetTimer( hDlg, _timer_id, 500, NULL );
			TCHAR status_text[ MAX_PATH ] = {0};
			_woi->get_text( status_text, _b_japanese );
			SetDlgItemText( hDlg, IDC_INFORMATION, status_text );
			EnableWindow( GetDlgItem( hDlg, IDOK), true );
		}
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
		break;
	}

}

static bool _IDM_HISTORY( HWND hDlg, UINT idm )
{
	TCHAR path[ MAX_PATH ];

	if( !Menu_History_GetPath( idm, path ) ) return false;
	if( !PathIsDirectory( path ) )
	{
		const TCHAR* p_msg   = _T("Not found.");
		const TCHAR* p_title = _T("error"     );

		if( _b_japanese )
		{
			p_msg   = _T("見つかりません");
			p_title = _T("エラー"        );
		}

		MessageBox( hDlg, p_msg, p_title, MB_OK );
		Menu_History_Delete( idm );
		return false;
	}

	SetDlgItemText( hDlg, IDC_DIRECTORYPATH, path );
	_RefreshList( hDlg );

	return true;
}

static const UINT _history_table[ 10 ] =
{
	IDM_HISTORY_0, IDM_HISTORY_1, IDM_HISTORY_2,
	IDM_HISTORY_3, IDM_HISTORY_4, IDM_HISTORY_5,
	IDM_HISTORY_6, IDM_HISTORY_7, IDM_HISTORY_8,
	IDM_HISTORY_9,
};

static int _wnd_min_w = 0;
static int _wnd_min_h = 0;

static void _set_min_window_size( HWND hwnd )
{
	RECT rc = {0};
	GetWindowRect( hwnd, &rc );
	rc.left = rc.left;
	_wnd_min_w = rc.right - rc.left;
	_wnd_min_h = rc.bottom - rc.top;
}

static INT_PTR CALLBACK _Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static HEARSELECTDIALOGSTRUCT *_p_hear = NULL;

	static bool    _bCapture = false;
	static int32_t _old_x    =     0;


	switch( msg )
	{
	case WM_INITDIALOG:

		_p_hear        = (HEARSELECTDIALOGSTRUCT*)l;
		_visible_flags = _p_hear->visible_flags;
		_b_japanese    = _p_hear->b_japanese   ;
		pxwWindowRect_load( hDlg, _rect_name, true, true );
		Japanese_Change_DialogItem( hDlg, _p_hear->b_japanese );
		if( _b_japanese ) SetWindowText( hDlg, _T("音源ファイルの選択") );

		_InitDialog( hDlg, l );
		_set_min_window_size( hDlg );

		Menu_History_init( GetSubMenu( GetMenu( hDlg ), 1 ), 10, _history_table, IDM_HISTORY_D, _ref_file_profile );
		Menu_History_Load();

		break;

	case WM_DESTROY:
		_woi->stop( true );
		break;

	case WM_CLOSE  :
		_CloseDialog( hDlg, _p_hear, false ); break;
		break;

	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK    : _CloseDialog( hDlg, _p_hear, true  ); break;
		case IDCANCEL: _CloseDialog( hDlg, _p_hear, false ); break;

		case IDC_CHECK_LOOP: if( !IsDlgButtonChecked( hDlg, IDC_CHECK_LOOP ) ) _woi->stop  ( false ); break;
		case IDC_COMBO_SORT: if( HIWORD( w ) == CBN_SELCHANGE                ) _RefreshList( hDlg  ); break;

		case IDM_DIR_DESKTOP:
		{
			TCHAR path[ MAX_PATH ] = {0};
			pxwFilePath_GetSpecial( path, SPECIALPATH_DESKTOP );
			SetDlgItemText( hDlg, IDC_DIRECTORYPATH, path );
			_RefreshList( hDlg );
		}
		break;

		case IDM_DIR_MYCOMPUTER:
		{
			TCHAR path[ MAX_PATH ] = {0};
			pxwFilePath_GetSpecial( path, SPECIALPATH_MYCOMPUTER );
			SetDlgItemText( hDlg, IDC_DIRECTORYPATH, path );
			_RefreshList( hDlg );
		}
		break;

		case IDM_DIR_MYDOCUMENT:
		{
			TCHAR path[ MAX_PATH ] = {0};
			pxwFilePath_GetSpecial( path, SPECIALPATH_MYDOCUMENT );
			SetDlgItemText( hDlg, IDC_DIRECTORYPATH, path );
			_RefreshList( hDlg );
		}
		break;

		case IDM_HISTORY_0:
		case IDM_HISTORY_1:
		case IDM_HISTORY_2:
		case IDM_HISTORY_3:
		case IDM_HISTORY_4:
		case IDM_HISTORY_5:
		case IDM_HISTORY_6:
		case IDM_HISTORY_7:
		case IDM_HISTORY_8:
		case IDM_HISTORY_9: _IDM_HISTORY( hDlg, LOWORD( w ) ); break;

		}
		break;

	case WM_TIMER:
		KillTimer( hDlg, _timer_id );
		_woi->stop( false );
		break;

	case WM_NOTIFY:
		if( (int)w == IDC_LIST )
		{
			LPNMHDR pNtfy = (LPNMHDR)l;
			switch( pNtfy->code ){
			case NM_DBLCLK      : _Proc_NM_DBLCLK( hDlg ); break;
			case LVN_ITEMCHANGED:
				if( ( (LPNMLISTVIEW)l )->uNewState & LVIS_SELECTED) _Proc_LVN_ITEMCHANGED( hDlg );
				break;
//			case NM_CLICK:
			}
		}
		break;

	case WM_LBUTTONDOWN:
		POINT pt;
		RECT  rc;

		GetCursorPos( &pt );
		_bCapture = false;
		GetWindowRect( GetDlgItem( hDlg, IDC_KEY_DRAG ), &rc );
		if( pt.x >= rc.left && pt.x < rc.right && pt.y >= rc.top  && pt.y < rc.bottom )
		{
			_bCapture = true;
			SetDlgItemText( hDlg, IDC_KEY_DRAG, _T("<->") );
			SetDlgItemInt(  hDlg, IDC_KEY, 0, true );
			SetCapture( hDlg );
			_old_x = pt.x;
		}
		break;

	case WM_MOUSEMOVE:
		if( _bCapture )
		{
			POINT pt ;
			int32_t   key;
			GetCursorPos( &pt );
			key = ( pt.x - _old_x ) / 4;
			if( key >  48 ) key =  48;
			if( key < -48 ) key = -48;
			SetDlgItemInt( hDlg, IDC_KEY, key, true );
		}
		break;

	case WM_LBUTTONUP:
		if( _bCapture )
		{
			ReleaseCapture();
			SetDlgItemText( hDlg, IDC_KEY_DRAG, _T("---") );
			_Proc_LVN_ITEMCHANGED( hDlg );
			_bCapture = false;
		}
		break;

	case WM_MOVE:
		pxwWindowRect_save( hDlg, _rect_name );
		break;

	case WM_SIZE:

		switch( w )
		{
		case SIZE_MINIMIZED: break;
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			_update_layout( hDlg );
			pxwWindowRect_save( hDlg, _rect_name );
			break;
		}
		break;

	// minimum size
	case WM_GETMINMAXINFO:
	{
		RECT rc_work;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rc_work, 0 );

		MINMAXINFO* pmm = (MINMAXINFO*)l;
		pmm->ptMinTrackSize.x = _wnd_min_w      ; // min w
		pmm->ptMinTrackSize.y = _wnd_min_h      ; // min h
		pmm->ptMaxTrackSize.x = rc_work.right   ; // max w
		pmm->ptMaxTrackSize.y = rc_work.bottom  ; // max h
	}
	break;
	default:
		return false;
	}
	return true;
}

//====================================================

bool DLLAPI pxtoneTool_HearSelect_Initialize( pxtonewinWoice* woi, pxFile2* app_file_profile )
{
	if( !woi ) return false;
	_woi = woi;
	_ref_file_profile = app_file_profile;
	pxwWindowRect_init( app_file_profile );
	return true;
}

bool DLLAPI pxtoneTool_HearSelect_Dialog( HWND hWnd, HEARSELECTDIALOGSTRUCT* p_hear )
{
	static HEARSELECTDIALOGSTRUCT hear = {0};
	hear = *p_hear;
	bool b = DialogBoxParam( g_hInst, _T("DLG_HEARSELECT"), hWnd, _Procedure, (LPARAM)&hear ) ? true : false;
	if( b ) *p_hear = hear;
	return b ? true : false;
}
