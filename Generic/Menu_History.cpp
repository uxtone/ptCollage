
#include <pxMem.h>


#include <pxDebugLog.h>
#include <pxCSV2.h>

#include <pxwUTF8.h>

// #include <pxTText.h>



#include <pxFile2.h>

#include "./Menu_History.h"

static UINT    _idm_dummy   =    0;
static int32_t _max_history =    0;

static UINT*   _p_idms      = NULL;
static int32_t _dim_num     =    0;
static HMENU   _hMenu       = NULL;

static const bool _b_UTF8 = true;


static const pxFile2* _ref_file_profile = NULL;

/*
                                  0123456789abcdef
static const char*  _code      = "path-his160916a";
*/

static const uxSS<16>  _code      = "path-his170121a";
static const uxSS<13>  _file_name = "path.history";
static const uxSS<13>  _dir_name  = "path_history";


bool Menu_History_init( HMENU hMenu, int32_t max_history, const UINT* idms, uint32_t idm_dummy, const pxFile2* file_profile )
{
	dlog_c( "pxhis: set menu" );

	if( !max_history ) return false;

	pxMem_free( (void**)&_p_idms );

	_hMenu = NULL;
	if( !pxMem_zero_alloc( (void**)&_p_idms, sizeof(UINT) * max_history ) ) return false;
	memcpy( _p_idms, idms,                   sizeof(UINT) * max_history );

	_max_history = max_history;
	_idm_dummy   = idm_dummy  ;
	_hMenu       = hMenu      ;
	_ref_file_profile = file_profile;

	return true;
}

void Menu_History_Release()
{
	dlog_c( "pxhis: release" );
	_hMenu = NULL;
	pxMem_free( (void**)&_p_idms );
}

static void _ClearHistory()
{
	dlog_c( "pxhis: clear" );

	if( !_hMenu ) return;

	for( int32_t i = 0; i < _max_history; i++ )
	{
		if( !i ) ModifyMenu ( _hMenu, 0, MF_BYPOSITION|MFT_STRING|MFS_GRAYED, _idm_dummy, uxT( "=" ) );
		else     DeleteMenu ( _hMenu, 1, MF_BYPOSITION );
	}
}

bool Menu_History_Load()
{
	dlog_c( "pxhis: load" );

	if( !_hMenu ) return false;

	bool        b_ret = false;
	pxCSV2      csv;
	int32_t     row =    0;
	const char* p_v = NULL;
	// pxTText     tt;
	uxDS     tt;

	pxDescriptor* desc = NULL;

	_ClearHistory();

	if( !_ref_file_profile->open_r( &desc, _dir_name, _file_name, NULL ) ) goto term;
	if( !csv.read( desc, _b_UTF8 )   ) goto term;
	SAFE_DELETE( desc );
	if( !csv.get_value( &p_v, 0, 0 ) ) goto term;
	if( strcmp( p_v, _code ) ) goto term;

	row = 1;
	for( int32_t i = 0; i < _max_history; i++, row++ )
	{
		if( !csv.get_value( &p_v, row, 0 ) ) break;
		if( !p_v[0] ) break;
		// if( !tt.set_UTF8_to_t( p_v ) ) goto term;
		if( !( tt = uxDS( p_v ) ) ) goto term;
		// if( !i ) ModifyMenu( _hMenu, 0, MF_BYPOSITION|MFT_STRING, _p_idms[ i ], tt.str() );
		if( !i ) ModifyMenu( _hMenu, 0, MF_BYPOSITION|MFT_STRING, _p_idms[ i ], uxT( tt ) );
		// else     AppendMenu( _hMenu,                  MFT_STRING, _p_idms[ i ], tt.str() );
		else     AppendMenu( _hMenu,                  MFT_STRING, _p_idms[ i ], uxT( tt ) );
	}

	b_ret = true;
term:
	SAFE_DELETE( desc );
	if( !b_ret ) _ClearHistory();

	return b_ret;
}

bool Menu_History_Save()
{
	bool    b_ret = false;
	char*   p_dst = NULL;
	pxDescriptor* desc = NULL;

	dlog_c( "pxhis: save" );

	if( !_hMenu ) return false;

	if( !_ref_file_profile->open_w( &desc, _dir_name, _file_name, NULL ) ) goto term;

	if( !desc->w_arg_asfile( "%s\r\n", _code ) ) goto term;

	for( int32_t i = 0; i < _max_history; i++ )
	{
		uxSS<MAX_PATH> path = {0};
		if( !GetMenuString( _hMenu, i, uxTOut( path ), MAX_PATH, MF_BYPOSITION ) ) break;
		pxMem_free( (void**)&p_dst );
		#ifdef UNICODE
		if( !pxwUTF8_wide_to_utf8( uxT( path ), &p_dst, NULL ) ) goto term;
		#else
		if( !pxwUTF8_sjis_to_utf8( path, &p_dst, NULL ) ) goto term;
		#endif

		if( !desc->w_arg_asfile( "%s\r\n", p_dst ) ) goto term;
	}

	b_ret = true;
term:
	SAFE_DELETE( desc );
	pxMem_free( (void**)&p_dst );

	return b_ret;
}

// ヒストリーを追加
void Menu_History_Add( const uxDS& path_new )
{
	uxSS<MAX_PATH> path;
	int32_t i;

	dlog_c( "pxhis: add" );


	if( !_hMenu ) return;
	if( path_new.empty() ) return;

	// 一番上に挿入
	InsertMenu( _hMenu, 0, MF_BYPOSITION|MFT_STRING, _idm_dummy, uxT( path_new ) );

	for( i = _max_history + 1; i > 0 ; i-- )
	{
		if( GetMenuString( _hMenu, i, uxTOut( path ), MAX_PATH, MF_BYPOSITION ) )
		{
			// 同じパスもしくは "=" なら削除
			if( !ux_stricmp( path, path_new ) || !ux_stricmp( path, "=" ) )
			{
				DeleteMenu( _hMenu, i, MF_BYPOSITION );
			}
		}
	}

	// はみ出たヤツを削除
	DeleteMenu( _hMenu, _max_history, MF_BYPOSITION );

	{ // IDM_HISTORY_.. を打ち直し
		MENUITEMINFO info;

		memset( &info, 0, sizeof(MENUITEMINFO) );
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask  = MIIM_ID;

		for( i = 0; i < _max_history; i++ )
		{
			info.wID    = _p_idms[ i ];
			SetMenuItemInfo( _hMenu, i, true, &info );
		}
	}

}

// ヒストリーを削除
void Menu_History_Delete( UINT idm )
{
	int32_t i = 0;

	dlog_c( "pxhis: delete" );

	if( !_hMenu ) return;

	for( i = 0; i < _max_history; i++ )
	{
		if( _p_idms[ i ] == idm ) break;
	}
	if( i == _max_history ) return;

	if( GetMenuItemID( _hMenu, 1 ) == -1 )
	{
		ModifyMenu( _hMenu, 0, MF_BYPOSITION|MFT_STRING|MFS_GRAYED, _idm_dummy, uxT( "=" ) );
		return;
	}
	DeleteMenu( _hMenu, i, MF_BYPOSITION );

	{ // IDM_HISTORY_.. を打ち直し
		MENUITEMINFO info;

		memset( &info, 0, sizeof(MENUITEMINFO) );
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask  = MIIM_ID;

		for( i = 0; i < _max_history; i++ )
		{
			info.wID    = _p_idms[ i ];
			SetMenuItemInfo( _hMenu, i, true, &info );
		}
	}
}

bool Menu_History_GetPath( UINT idm, uxDS& path )
{
	dlog_c( "pxhis: get path" );

	if( !_hMenu ) return false;

	return GetMenuString( _hMenu, idm, uxTOut( path ), MAX_PATH, MF_BYCOMMAND ) ? true : false;
}
