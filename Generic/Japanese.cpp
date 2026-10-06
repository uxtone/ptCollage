#ifndef PX_JAPANESE_INIT

#include <pxStdDef.h>



#include "Japanese.h"

static bool                        _bJapanese             ;
static int                         _dlgitem_num     =    0;
static JAPANESETEXTSTRUCT_DLGITEM* _table_dlgitems  = NULL;
static int                         _menuitem_num    =    0;
static JAPANESETEXTSTRUCT_TEXTSET* _table_menuitems = NULL;
static int                         _message_num     =    0;
static JAPANESETEXTSTRUCT_TEXTSET* _table_messages  = NULL;

void Japanese_Release()
{
	if( _table_dlgitems  ) free( _table_dlgitems  ); _table_dlgitems  = NULL;
	if( _table_menuitems ) free( _table_menuitems ); _table_menuitems = NULL;
	if( _table_messages  ) free( _table_messages  ); _table_messages  = NULL;
}

bool Japanese_DialogItem_SetTable( JAPANESETEXTSTRUCT_DLGITEM* table, int num )
{
	if( _table_dlgitems ) free( _table_dlgitems ); _table_dlgitems = NULL;
	_table_dlgitems = (JAPANESETEXTSTRUCT_DLGITEM*)malloc( sizeof(JAPANESETEXTSTRUCT_DLGITEM) * num );
	if( !_table_dlgitems ) return false;
	memcpy( _table_dlgitems, table,                        sizeof(JAPANESETEXTSTRUCT_DLGITEM) * num );
	_dlgitem_num = num;
	return true;
}

bool Japanese_MenuItem_SetTable( JAPANESETEXTSTRUCT_TEXTSET* table, int num )
{
	if( _table_menuitems ) free( _table_menuitems ); _table_menuitems = NULL;
	_table_menuitems = (JAPANESETEXTSTRUCT_TEXTSET*)malloc( sizeof(JAPANESETEXTSTRUCT_TEXTSET) * num );
	if( !_table_menuitems ) return false;
	memcpy( _table_menuitems, table,                        sizeof(JAPANESETEXTSTRUCT_TEXTSET) * num );
	_menuitem_num = num;
	return true;
}

bool Japanese_Message_SetTable( JAPANESETEXTSTRUCT_TEXTSET* table, int num )
{
	if( _table_messages ) free( _table_messages ); _table_messages = NULL;
	_table_messages = (JAPANESETEXTSTRUCT_TEXTSET*)malloc( sizeof(JAPANESETEXTSTRUCT_TEXTSET) * num );
	if( !_table_messages ) return false;
	memcpy( _table_messages, table,                        sizeof(JAPANESETEXTSTRUCT_TEXTSET) * num );
	_message_num = num;
	return true;
}

void Japanese_DialogItem_Change(   HWND hWnd )
{
	if( !_bJapanese || !_table_dlgitems ) return;

	for( int i = 0; i < _dlgitem_num; i++ )
	{
		if( GetDlgItem( hWnd, _table_dlgitems[ i ].idc ) )
		{
			SetDlgItemText( hWnd, _table_dlgitems[ i ].idc, uxT( _table_dlgitems[ i ].p_text_j ) );
		}
	}
}

void Japanese_MenuItem_Change( HMENU hMenu )
{
	uxSS<32> str = {0};

	if( !_bJapanese || !_table_menuitems ) return;

	for( int pos = 0; pos < _menuitem_num; pos++ )
	{
		if( GetMenuString( hMenu, pos, uxTOut( str ), 32, MF_BYPOSITION ) )
		{
			// 該当を探す
			int i;
			for( i = 0; i < _menuitem_num; i++ )
			{
				if( !strcmp( str, _table_menuitems[ i ].p_text_e ) ) break;
			}
			// 書きかえ
			if( i != _menuitem_num )
			{
				int id = GetMenuItemID( hMenu, pos );
				if( id == 0xffffffff )
				{
					HMENU hSub = GetSubMenu( hMenu, pos );

					if( hSub )
					{
						ModifyMenu( hMenu, pos, MF_BYPOSITION|MFT_STRING, (uintptr_t)hSub, uxT( _table_menuitems[i].p_text_j ) );
						Japanese_MenuItem_Change( hSub ); // 再帰！！
					}
				}
				else
				{
					ModifyMenu( hMenu, pos, MF_BYPOSITION|MFT_STRING, id, uxT( _table_menuitems[ i ].p_text_j ) );
				}
			}
		}
	}
}


void Japanese_MessageBox( HWND hWnd, const uxDS& message, const uxDS& title, UINT uType )
{
	int32_t         i;
	const char* p_text  = *message;
	const char* p_title = *title;

	if( _bJapanese || !_table_messages )
	{
		for( i = 0; i < _message_num; i++ ){ if( !ux_stricmp( message, _table_messages[ i ].p_text_e ) ) break; }
		if(         i < _message_num ) p_text  = _table_messages[ i ].p_text_j;

		for( i = 0; i < _message_num; i++ ){ if( !ux_stricmp( title,   _table_messages[ i ].p_text_e ) ) break; }
		if(         i < _message_num ) p_title = _table_messages[ i ].p_text_j;
	}

	MessageBox( hWnd, uxT( p_text ), uxT( p_title ), uType );
}

bool Japanese_Is(){ return _bJapanese; }
void Japanese_Set( bool b ){ _bJapanese = b; }

#endif
#define PX_JAPANESE_INIT
