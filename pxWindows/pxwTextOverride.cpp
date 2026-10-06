
#include <pxMem.h>


#include <vector>
#include <pxStrT.h>
#include <pxCSV2.h>
#include <pxUTF8.h>
// #include <pxTText.h>



#include "./pxwUTF8.h"

#include "./pxwTextOverride.h"


pxwTextOverride::pxwTextOverride()
{
	_inv_name   = NULL;
	_csv        = NULL;
}

pxwTextOverride::~pxwTextOverride()
{
	SAFE_DELETE( _csv );
}


const uxDS& pxwTextOverride::get_inv_name() const
{
	return _inv_name;
}

static bool _is_ignore_text( const uxDS& p_t )
{
	int32_t res = 0;
	if( !pxStrT_compare( p_t, "*-", 2, &res ) ) return true;
	if( res ) return true;
	return false;
}

bool pxwTextOverride::_find_original_to_tt( uxDS& tt_ovr, const uxDS& t_src )
{
	int32_t     r          =     0;
	const char* p_str_ride = NULL ;

	if( !t_src ) return false;

	if( !_csv->find_value( &r, 0, *t_src ) ) return false; // find (both sides are UTF-8)
	if( !_csv->get_value ( &p_str_ride, r, 1    ) ) return false; // get

	tt_ovr = p_str_ride;
	return true;
}

#if 0 // was: pxTText out-parameter, and a TCHAR -> UTF-8 conversion of the source
bool pxwTextOverride::_find_original_to_tt( pxTText* tt_ovr, const uxDS& t_src )
bool pxwTextOverride::_find_original_to_tt( uxDS& tt_ovr, const uxDS& t_src )
{
	bool        b_ret      = false;
	char*       utf8_src   = NULL ;
	int32_t     r          =     0;
	const char* p_str_ride = NULL ;

	// -> utf8
#ifdef UNICODE
	if( !pxwUTF8_wide_to_utf8( t_src, &utf8_src, NULL ) ) goto term;
#else
	if( !pxwUTF8_sjis_to_utf8( t_src, &utf8_src, NULL ) ) goto term;
#endif

	if( !_csv->find_value     ( &r, 0, utf8_src   ) ) goto term; // find
	if( !_csv->get_value      ( &p_str_ride, r, 1 ) ) goto term; // get
	if( !tt_ovr->set_UTF8_to_t(  p_str_ride       ) ) goto term; // -> tchar

	b_ret = true;
term:
	pxMem_free( (void**)&utf8_src );
	return b_ret;
}
#endif

bool pxwTextOverride::_update_title     ( HWND hwnd   )
{
	uxSS<100>   text_src = {0};
	// pxTText tt_dst;
	uxDS tt_dst;

	GetWindowText( hwnd, uxTOut( text_src ), 100 );

	if( _is_ignore_text( text_src ) ) return true; // ignore..

	// if     ( _find_original_to_tt( &tt_dst, text_src ) ) SetWindowText( hwnd, tt_dst.tchr() );
	if     ( _find_original_to_tt( tt_dst, text_src ) ) SetWindowText( hwnd, uxT( tt_dst ) );
	else if( !_inv_name                                ){ if( !pxStrT_copy_allocate( _inv_name, text_src, 0 ) ) return false; }
	return true;
}

bool pxwTextOverride::_update_combo_box( HWND h_ctrl )
{
	bool    b_ret = false;
	int     sel   = SendMessage( h_ctrl, CB_GETCURSEL, 0, 0 );
	int     count = SendMessage( h_ctrl, CB_GETCOUNT , 0, 0 );
	std::vector<uxDS> list( count > 0 ? count : 0 );
	// pxTText tt_dst;
	uxDS tt_dst;

	for( int i = 0; i < count; i++ )
	{
		int len = SendMessage( h_ctrl, CB_GETLBTEXTLEN, i, 0);
		if( len > 0 )
		{
			std::vector<TCHAR> text( len + 1, 0 );
			if( SendMessage( h_ctrl, CB_GETLBTEXT, i, (LPARAM)text.data() ) == CB_ERR ) goto term;
			list[ i ] = uxDS_from_t( text.data() );
		}
	}

	// clear
	while( SendMessage(h_ctrl, CB_GETCOUNT, 0, 0 ) ) SendMessage(h_ctrl, CB_DELETESTRING, 0, 0);

	for( int i = 0; i < count; i++ )
	{
		if( list[ i ] )
		{
			if     ( _is_ignore_text     (          list[ i ] ) ) SendMessage( h_ctrl, CB_ADDSTRING, i, uxLP( list[ i ] ) );
			// else if( _find_original_to_tt( &tt_dst, list[ i ] ) ) SendMessage( h_ctrl, CB_ADDSTRING, i, (LPARAM)tt_dst.tchr()                  );
			else if( _find_original_to_tt( tt_dst, list[ i ] ) ) SendMessage( h_ctrl, CB_ADDSTRING, i, uxLP( tt_dst ) );
			else                                                  SendMessage( h_ctrl, CB_ADDSTRING, i, uxLP( list[ i ] ) );
		}
	}

	if( sel != CB_ERR ) SendMessage( h_ctrl, CB_SETCURSEL, sel, 0 );

	b_ret = true;
term:
	return b_ret;
}

bool pxwTextOverride::_override_callback( HWND h_ctrl )
{
	bool    b_ret             = false;
	uxSS<100>   text_src = { 0 };
	uxSS<100>   class_name = { 0 };
	// pxTText tt_dst;
	uxDS tt_dst;

	GetWindowText( h_ctrl, uxTOut( text_src )  , 100 );
	GetClassName ( h_ctrl, uxTOut( class_name ), 100 );

	if( _is_ignore_text( text_src ) ) return true;

	if( !strcmp( class_name, "Static" ) ||
		!strcmp( class_name, "Button" ) )
	{
		// if     ( _find_original_to_tt( &tt_dst, text_src ) ) SetWindowText( h_ctrl, tt_dst.tchr() );
		if     ( _find_original_to_tt( tt_dst, text_src ) ) SetWindowText( h_ctrl, uxT( tt_dst ) );
		else if( !_inv_name                                ){ if( !pxStrT_copy_allocate( _inv_name, text_src, 0 ) ) return false; }
	}
	// combo..
	else if( !strcmp( class_name, "ComboBox" ) )
	{
		if( !_update_combo_box( h_ctrl ) ) goto term;
	}

	b_ret = true;
term:

	return b_ret;
}

BOOL CALLBACK pxwTextOverride::_override_sttc    ( HWND h_ctrl, LPARAM lp )
{
	pxwTextOverride* cls = (pxwTextOverride*)lp;
	return cls->_override_callback( h_ctrl ) ? TRUE : FALSE;
}

bool pxwTextOverride::override_dialog( HWND hdlg, pxDescriptor* desc, bool b_UTF8 )
{
	if( !hdlg ) return false;

	_inv_name = uxDS();
	SAFE_DELETE( _csv ); _csv = new pxCSV2( ';', '"', '"' );
	if( !_csv->read( desc, b_UTF8 ) ) return false;

	_update_title( hdlg );

	if( !EnumChildWindows( hdlg, _override_sttc, (LPARAM)this ) ) return false;

	return true;
}


bool pxwTextOverride::override_menu( HMENU hmenu, pxDescriptor* desc, bool b_UTF8 )
{
	if( !hmenu ) return false;

	_inv_name = uxDS();
	SAFE_DELETE( _csv ); _csv = new pxCSV2( ';', '"', '"' );
	if( !_csv->read( desc, b_UTF8 ) ) return false;

	return _override_menu( hmenu );
}

bool pxwTextOverride::_override_menu( HMENU hmenu )
{
	if( !hmenu ) return false;

	bool    b_ret    = false;
	int32_t len      =     0;
	uxDS    p_src ;
	int32_t item_num = GetMenuItemCount( hmenu );

	// pxTText tt_dst;
	uxDS tt_dst;

	for( int i = 0; i < item_num; i++ )
	{
		if( len = GetMenuString( hmenu, i, NULL, 0, MF_BYPOSITION ) )
		{
			if( !GetMenuString( hmenu, i, uxTOut( p_src, len + 1 ), len + 1, MF_BYPOSITION ) ) goto term; // into a uxDS

			int id = GetMenuItemID( hmenu, i );
			if( id == -1 )
			{
				HMENU h_sub = GetSubMenu( hmenu, i );
				if( h_sub )
				{
					// if( _find_original_to_tt( &tt_dst, p_src ) )
					if( _find_original_to_tt( tt_dst, p_src ) )
					{
						// ModifyMenu( hmenu, i, MF_BYPOSITION|MFT_STRING, (uintptr_t)h_sub, tt_dst.tchr() );
						ModifyMenu( hmenu, i, MF_BYPOSITION|MFT_STRING, (uintptr_t)h_sub, uxT( tt_dst ) );
					}
					_override_menu( h_sub ); // 再帰！！
				}
			}
			// else if( _find_original_to_tt( &tt_dst, p_src ) )
			else if( _find_original_to_tt( tt_dst, p_src ) )
			{
				// ModifyMenu( hmenu, i, MF_BYPOSITION|MFT_STRING, id, tt_dst.tchr() );
				ModifyMenu( hmenu, i, MF_BYPOSITION|MFT_STRING, id, uxT( tt_dst ) );
			}
		}
	}
	b_ret = true;
term:
	return b_ret;
}
