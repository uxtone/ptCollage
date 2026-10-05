#include <uxStr.h>

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>
#include <pxwUTF8.h>
#include <pxMem.h>

#include "../../Generic/Japanese.h"

#include "../interface/if_ScopeField.h"
#include "../interface/if_ToolPanel.h"

#include "../resource.h"
#include "../UndoEvent.h"
#include "../ScopeEvent.h"
#include "../UnitFocus.h"

static bool _SetOparatedUnitName( HWND hDlg )
{
	bool     b_ret = false;
	int32_t  u;

	char*    p_unit_names = NULL;
	wchar_t* p_wide       = NULL;

	if( !UnitFocus_CountFocusedOrOperated() ) return false;

	int32_t  unit_num     = g_pxtn->Unit_Num();
	int32_t  buf_size     = ( pxtnMAX_TUNEUNITNAME + 2 ) * unit_num + 1;

	if( !pxMem_zero_alloc( (void**)&p_unit_names, buf_size ) ) return false;

	if( UnitFocus_IsFocusOnly() )
	{
		{
			u = UnitFocus_Get();
			if( u != -1 && g_pxtn->Unit_Get( u )->is_name_buf() )
			{
				strncat( p_unit_names, g_pxtn->Unit_Get( u )->get_name_buf( NULL ), buf_size );
				strncat( p_unit_names, "\r\n", buf_size );
			}
		}
	}
	else
	{
		for( u = 0; u < unit_num; u++ )
		{
			if( UnitFocus_IsFocusedOrOperated( u ) )
			{
				if( g_pxtn->Unit_Get( u )->is_name_buf() ) strncat( p_unit_names, g_pxtn->Unit_Get( u )->get_name_buf( NULL ), buf_size );
				;                                          strncat( p_unit_names, "\r\n", buf_size );
			}
		}
	}

	{
		uxDS    p;
		if( !( p = uxDS_from_sjis( p_unit_names ) ) ) goto term; // SJIS -> UTF-8
		SetDlgItemText( hDlg, IDC_UNITS, uxT( p ) );
	}

	b_ret = true;
term:

	pxMem_free( (void**)&p_unit_names );
	pxMem_free( (void**)&p_wide       );

	return true;
}

static bool _InitDialog( HWND hDlg )
{
	int32_t       meas1, beat1, clock1;
	int32_t       meas2, beat2, clock2;

	if( !_SetOparatedUnitName( hDlg ) ) return false;

	if_ScopeField_GetSelected( &meas1, &beat1, &clock1, &meas2, &beat2, &clock2 );

	SendMessage( GetDlgItem( hDlg, IDC_SPIN_VALUE ), UDM_SETRANGE32, (WPARAM)-96, (LPARAM)96 );

	SetDlgItemInt( hDlg, IDC_FROMMEAS1 , meas1               , true );
	SetDlgItemInt( hDlg, IDC_FROMBEAT1 , beat1               , true );
	SetDlgItemInt( hDlg, IDC_FROMCLOCK1, clock1 / CLOCK_ROUGH, true );
	SetDlgItemInt( hDlg, IDC_FROMMEAS2 , meas2               , true );
	SetDlgItemInt( hDlg, IDC_FROMBEAT2 , beat2               , true );
	SetDlgItemInt( hDlg, IDC_FROMCLOCK2, clock2 / CLOCK_ROUGH, true );

	SetDlgItemInt( hDlg, IDC_TOMEAS    , 0, true );
	SetDlgItemInt( hDlg, IDC_TOBEAT    , 0, true );
	SetDlgItemInt( hDlg, IDC_TOCLOCK   , 0, true );

	SetDlgItemInt( hDlg, IDC_TIME      , 1, true );

	SetDlgItemInt( hDlg, IDC_BEATCLOCK, g_pxtn->master->get_beat_clock() / CLOCK_ROUGH, true );

	// check ----
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_ON        , true );
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_KEY       , true );
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_PAN_VOLUME, true );
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_VOLUME    , true );
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_PORTAMENT , true );
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_VOICENO   , true );
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_GROUPNO   , true );
	CheckDlgButton( hDlg,IDC_CHECK_EVENT_TUNING    , true );

	return true;
}


static void _GetInputParameter( HWND hDlg )
{
	int32_t meas1, beat1, clock1;
	int32_t meas2, beat2, clock2;
	int32_t meas3, beat3, clock3;
	int32_t time;

	bool bEnables[ EVENTKIND_NUM ];

	memset( bEnables, 0, sizeof(bool) * EVENTKIND_NUM );

	meas1  = GetDlgItemInt( hDlg, IDC_FROMMEAS1,  NULL, true );
	beat1  = GetDlgItemInt( hDlg, IDC_FROMBEAT1,  NULL, true );
	clock1 = GetDlgItemInt( hDlg, IDC_FROMCLOCK1, NULL, true ) * CLOCK_ROUGH;
	meas2  = GetDlgItemInt( hDlg, IDC_FROMMEAS2,  NULL, true );
	beat2  = GetDlgItemInt( hDlg, IDC_FROMBEAT2,  NULL, true );
	clock2 = GetDlgItemInt( hDlg, IDC_FROMCLOCK2, NULL, true ) * CLOCK_ROUGH;
	meas3  = GetDlgItemInt( hDlg, IDC_TOMEAS,     NULL, true );
	beat3  = GetDlgItemInt( hDlg, IDC_TOBEAT,     NULL, true );
	clock3 = GetDlgItemInt( hDlg, IDC_TOCLOCK,    NULL, true ) * CLOCK_ROUGH;

	time   = GetDlgItemInt( hDlg, IDC_TIME,  NULL, true );

	clock1 = g_pxtn->master->get_this_clock( meas1, beat1, clock1 );
	clock2 = g_pxtn->master->get_this_clock( meas2, beat2, clock2 );
	clock3 = g_pxtn->master->get_this_clock( meas3, beat3, clock3 );

	// enables
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_ON ) )
	{
		bEnables[ EVENTKIND_ON       ] = true;
		bEnables[ EVENTKIND_VELOCITY ] = true;
	}
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_KEY        ) ) bEnables[ EVENTKIND_KEY       ] = true;
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_PAN_TIME   ) ) bEnables[ EVENTKIND_PAN_TIME  ] = true;
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_PAN_VOLUME ) ) bEnables[ EVENTKIND_PAN_VOLUME   ] = true;
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_VOLUME     ) ) bEnables[ EVENTKIND_VOLUME    ] = true;
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_PORTAMENT  ) ) bEnables[ EVENTKIND_PORTAMENT ] = true;
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_VOICENO    ) ) bEnables[ EVENTKIND_VOICENO   ] = true;
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_GROUPNO    ) ) bEnables[ EVENTKIND_GROUPNO   ] = true;
	if( IsDlgButtonChecked( hDlg, IDC_CHECK_EVENT_TUNING     ) ) bEnables[ EVENTKIND_TUNING    ] = true;

	ScopeEvent_Copy(  clock1, clock2, bEnables );
	ScopeEvent_Paste( clock3, time  , bEnables );
}


//コールバック
INT_PTR CALLBACK
dlg_CopyMeas( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{

	// IME off
	switch( msg ){
	case WM_COMMAND:
		switch( LOWORD( w ) ){
		case IDC_FROMMEAS1:
		case IDC_FROMBEAT1:
		case IDC_FROMCLOCK1:
		case IDC_FROMBEAT2:
		case IDC_FROMMEAS2:
		case IDC_FROMCLOCK2:
		case IDC_TOBEAT:
		case IDC_TOMEAS:
		case IDC_TOCLOCK:
		case IDC_TIME:

			if( HIWORD( w ) == EN_SETFOCUS ){
				HIMC hImc = ImmGetContext( hDlg );
				ImmSetOpenStatus( hImc, false );
				ImmReleaseContext( hDlg, hImc );
			}
			break;
		}
		break;
	}

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		if( !_InitDialog( hDlg ) ){
			EndDialog( hDlg, false );
			break;
		}

		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		case IDOK:

			_GetInputParameter( hDlg );
			EndDialog( hDlg, true );
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
