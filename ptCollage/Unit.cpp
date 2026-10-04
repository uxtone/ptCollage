
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwUTF8.h>
#include <pxwFilePath.h>

#include <pxwAlteration.h>
extern pxwAlteration* g_alte;

#include <pxShiftJIS.h>
#include <pxMem.h>

#include "resource.h"

//#include "../Generic/FilePath.h"
#include "../Generic/Japanese.h"

#include "interface/if_UnitTray.h"
#include "dialog/dlg_AddUnit.h"

#include "UnitFocus.h"
#include "UndoEvent.h"

static char* _default_material_folder = "my_material";

#define _BUFNUM_DELETE_UNIT (pxtnMAX_TUNEUNITNAME + 32)

extern HINSTANCE g_hInst;
extern HWND      g_hWnd_Main;

int32_t  GetCompileVersion( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );
bool IsShiftJIS( unsigned char c );
bool InquireOperation();

INT_PTR CALLBACK
dlg_YesNo( HWND hWnd, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_Unit(  HWND hWnd, UINT msg, WPARAM w, LPARAM l );

// ユニット削除
bool Unit_Remove( int32_t u )
{
	if( !InquireOperation() ) return false;

	const pxtnUnit *p_unit = g_pxtn->Unit_Get( u );
	if( !p_unit )
	{
		Japanese_MessageBox( g_hWnd_Main, _T("no unit"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	bool        b_ret = false;
	TCHAR       str[ _BUFNUM_DELETE_UNIT ] = {0};
	const TCHAR* p_name_t = NULL;
	wchar_t*    p_wide   = NULL;
	const char* p_c      = p_unit->get_name_buf( NULL );

#ifdef UNICODE
	if( !pxwUTF8_sjis_to_wide( p_c, &p_wide, NULL ) ) goto term;
	p_name_t = p_wide;
#else
	p_name_t = p_c;
#endif

	if( Japanese_Is() ) _stprintf_s( str, _BUFNUM_DELETE_UNIT, _T("ユニット \'%s\'を削除します"), p_name_t );
	else                _stprintf_s( str, _BUFNUM_DELETE_UNIT, _T("Remove : '%s\'"             ), p_name_t );

    if( !DialogBoxParam( g_hInst, _T("DLG_YESNO"), g_hWnd_Main, dlg_YesNo, (LPARAM)str ) ){ b_ret = true; goto term; }

	g_pxtn->Unit_Remove( u );
	g_pxtn->evels->Record_UnitNo_Miss( (unsigned char)u );
	if_UnitTray_RedrawAllName( NULL );
	UnitFocus_Set( 0, true );

	g_alte->set();

	UndoEvent_ReleaseUndoRedo();

	b_ret = true;
term:
	pxMem_free( (void**)&p_wide );
	return b_ret;
}

bool Unit_Replace( int32_t old_pos, int32_t new_pos )
{
	if( !InquireOperation() ) return false;
	if( old_pos == new_pos  ) return true ;

	g_pxtn->Unit_Replace( old_pos, new_pos );
	if( g_pxtn->evels->Record_UnitNo_Replace( (unsigned char)old_pos, (unsigned char)new_pos ) )
	{
		UndoEvent_Replace_UnitNo( old_pos, new_pos );
	}
	if_UnitTray_RedrawAllName( NULL );
	UnitFocus_Set( new_pos, true );

	g_alte->set();

	return true;
}

// ユニット登録
bool Unit_Dialog_Add()
{
	if( !InquireOperation() ) return false;

	int32_t u = g_pxtn->Unit_Num();

	if( u >= g_pxtn->Unit_Max() )
	{
		Japanese_MessageBox( g_hWnd_Main, _T("full unit"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	ADDUNITSTRUCT addunit = {0}; strcpy( addunit.name, "unit" );

	if( !DialogBoxParam( g_hInst, _T("DLG_ADDUNIT"), g_hWnd_Main, dlg_AddUnit, (LPARAM)&addunit ) ) return false;

	if( !g_pxtn->Unit_AddNew() ){ Japanese_MessageBox( g_hWnd_Main, _T("add unit"), _T("error"), MB_OK|MB_ICONEXCLAMATION ); return false; }

	{
		int32_t name_size = 0;
		pxShiftJIS_check_size( addunit.name, &name_size, true );
		g_pxtn->Unit_Get_variable( u )->set_name_buf( addunit.name, name_size );
	}

	// add a basic event.
	if( g_pxtn->Woice_Num() ) g_pxtn->evels->Record_Add_i( 0, (unsigned char)u, EVENTKIND_VOICENO, addunit.voice_no );

	g_pxtn->Unit_SetOpratedAll( false );
	UnitFocus_Set( u, true );

	if_UnitTray_RedrawAllName( NULL );
	if_UnitTray_JustScroll   ();

	g_alte->set();

	return true;
}

bool Unit_Dialog_Edit( int32_t u )
{
	if( !InquireOperation() ) return false;

	if( DialogBoxParam( g_hInst, _T("DLG_UNIT"), g_hWnd_Main, dlg_Unit, (LPARAM)u ) )
	{
		if_UnitTray_RedrawAllName( NULL );
		g_alte->set();
	}

	return true;
}
