
#include <pxMem.h>
// #include <pxTText.h>
#include <uxStr.h>

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>



#include "../../Generic/Japanese.h"

#include "../interface/if_ScopeField.h"
#include "../interface/if_ToolPanel.h"

#include "../resource.h"
#include "../UndoEvent.h"
#include "../ScopeEvent.h"
#include "../UnitFocus.h"

#include "dlg_EventVolume.h"

static bool _SetOparatedUnitName( HWND hDlg )
{
	bool     b_ret        = false;
	int32_t  unit_num     =     0;
	char*    p_unit_names = NULL ;
	// pxTText tt;
	uxDS tt;

	// 対象ユニットなし
	if( !UnitFocus_CountFocusedOrOperated() ) return false;

	unit_num     = g_pxtn->Unit_Num();

	if( !pxMem_zero_alloc( (void**)&p_unit_names, (pxtnMAX_TUNEUNITNAME+2) * unit_num + 1 ) ) goto term;

	if( UnitFocus_IsFocusOnly() )
	{
		{
			int u = UnitFocus_Get();
			if( u != -1 )
			{
				strcat( p_unit_names, g_pxtn->Unit_Get( u )->get_name_buf( NULL ) );
				strcat( p_unit_names, "\r\n" );
			}
		}
	}
	else
	{
		int count = 0;
		for( int u = 0; u < unit_num; u++ )
		{
			if( UnitFocus_IsFocusedOrOperated( u ) )
			{
				if( count ) strcat( p_unit_names, "\r\n" );
				strcat( p_unit_names, g_pxtn->Unit_Get( u )->get_name_buf( NULL ) );
				count++;
			}
		}
	}

	// if( !tt.set_sjis_to_t( p_unit_names ) ) goto term;
	if( !( tt = uxDS_from_sjis( p_unit_names ) ) ) goto term;
	// SetDlgItemText( hDlg, IDC_UNITS, tt.tchr() );
	SetDlgItemText( hDlg, IDC_UNITS, uxT( tt ) );

	b_ret = true;
term:

	pxMem_free( (void**)&p_unit_names );

	return b_ret;
}

/*
enum enum_EventKind
{
	enum_EventKind_Key
	enum_EventKind_Velocity
	enum_EventKind_TimePan
	enum_EventKind_VolPan
	enum_EventKind_Volume
}
*/

static bool _InitDialog( HWND hDlg, enum_EventKind mode )
{
	int32_t meas1, beat1, clock1;
	int32_t meas2, beat2, clock2;

	const char* title_table_j[] = { "≪トランスポーズ≫", "≪ベロシティ≫", "≪時間パン≫", "≪音量パン≫", "≪ボリューム≫" };
	const char* title_table_e[] = { "== Transpose ==", "== Velocity ==", "== Pan (time) ==", "== Pan (Volume) ==", "== Volume ==" };

	if( Japanese_Is() ) SetDlgItemText( hDlg, IDC_TEXT_EVENTVOLUMETITLE, uxT( title_table_j[ mode ] ) );
	else                SetDlgItemText( hDlg, IDC_TEXT_EVENTVOLUMETITLE, uxT( title_table_e[ mode ] ) );

	if( !_SetOparatedUnitName( hDlg ) ) return false;

	if_ScopeField_GetSelected( &meas1, &beat1, &clock1, &meas2, &beat2, &clock2 );

	SendMessage( GetDlgItem( hDlg, IDC_SPIN_VALUE ), UDM_SETRANGE32, (WPARAM)-96, (LPARAM)96 );


	SetDlgItemInt( hDlg, IDC_FROMMEAS1,  meas1,  true );
	SetDlgItemInt( hDlg, IDC_FROMMEAS2,  meas2,  true );
	SetDlgItemInt( hDlg, IDC_FROMBEAT1,  beat1,  true );
	SetDlgItemInt( hDlg, IDC_FROMBEAT2,  beat2,  true );
	SetDlgItemInt( hDlg, IDC_FROMCLOCK1, clock1 / CLOCK_ROUGH, true );
	SetDlgItemInt( hDlg, IDC_FROMCLOCK2, clock2 / CLOCK_ROUGH, true );

	SetDlgItemInt( hDlg, IDC_TRANSPOSE,  0,      true );

	SetDlgItemInt( hDlg, IDC_BEATCLOCK, g_pxtn->master->get_beat_clock() / CLOCK_ROUGH, true );

	return true;
}

static void _Apply( HWND hDlg, enum_EventKind mode )
{
	int32_t meas1, beat1, clock1;
	int32_t meas2, beat2, clock2;
	int32_t volume;


	meas1  = GetDlgItemInt( hDlg, IDC_FROMMEAS1,  NULL, true );
	meas2  = GetDlgItemInt( hDlg, IDC_FROMMEAS2,  NULL, true );
	beat1  = GetDlgItemInt( hDlg, IDC_FROMBEAT1,  NULL, true );
	beat2  = GetDlgItemInt( hDlg, IDC_FROMBEAT2,  NULL, true );
	clock1 = GetDlgItemInt( hDlg, IDC_FROMCLOCK1, NULL, true ) * CLOCK_ROUGH;
	clock2 = GetDlgItemInt( hDlg, IDC_FROMCLOCK2, NULL, true ) * CLOCK_ROUGH;

	volume = GetDlgItemInt( hDlg, IDC_VOLUME    ,  NULL, true );

	clock1 = g_pxtn->master->get_this_clock( meas1, beat1, clock1 );
	clock2 = g_pxtn->master->get_this_clock( meas2, beat2, clock2 );

	switch( mode )
	{
	case enum_EventKind_Key     : ScopeEvent_ChangeVolume( clock1, clock2, EVENTKIND_KEY       , volume * 0x100 ); break;
	case enum_EventKind_TimePan : ScopeEvent_ChangeVolume( clock1, clock2, EVENTKIND_PAN_TIME  , volume         ); break;
	case enum_EventKind_VolPan  : ScopeEvent_ChangeVolume( clock1, clock2, EVENTKIND_PAN_VOLUME, volume         ); break;
	case enum_EventKind_Velocity: ScopeEvent_ChangeVolume( clock1, clock2, EVENTKIND_VELOCITY  , volume         ); break;
	case enum_EventKind_Volume  : ScopeEvent_ChangeVolume( clock1, clock2, EVENTKIND_VOLUME    , volume         ); break;
	}
}

//コールバック
INT_PTR CALLBACK
dlg_EventVolume( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static enum_EventKind _mode;

	// IME off
	switch( msg )
	{
	case WM_COMMAND:
		switch( LOWORD( w ) )
		{
		case IDC_FROMMEAS1 :
		case IDC_FROMMEAS2 :
		case IDC_FROMBEAT1 :
		case IDC_FROMBEAT2 :
		case IDC_FROMCLOCK1:
		case IDC_FROMCLOCK2:
		case IDC_TRANSPOSE :

			if( HIWORD( w ) == EN_SETFOCUS )
			{
				HIMC hImc = ImmGetContext( hDlg );
				ImmSetOpenStatus( hImc, false );
				ImmReleaseContext( hDlg, hImc );
			}
			break;
		}
		break;
	}

	switch( msg )
	{

	//ダイアログ起動
	case WM_INITDIALOG:

		_mode = (enum_EventKind)l;
		if( !_InitDialog( hDlg, _mode ) ){ EndDialog( hDlg, false ); break; }
		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK:

			_Apply(    hDlg, _mode );
			EndDialog( hDlg, true  );
			break;

		case IDCANCEL:
			EndDialog( hDlg, false );
			break;
		}
		break;

	default: return false;

	}
	return true;
}
