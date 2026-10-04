
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../interface/if_ScopeField.h"

#include "../resource.h"
#include "../UndoEvent.h"
#include "../ScopeEvent.h"
#include "../UnitFocus.h"



static bool _InitDialog( HWND hDlg, bool bMultiUnit )
{
	int32_t       meas1, beat1, clock1;
	int32_t       meas2, beat2, clock2;

	if_ScopeField_GetSelected( &meas1, &beat1, &clock1, &meas2, &beat2, &clock2 );

	SetDlgItemInt( hDlg, IDC_FROMMEAS1 , meas1               , true );
	SetDlgItemInt( hDlg, IDC_FROMBEAT1 , beat1               , true );
	SetDlgItemInt( hDlg, IDC_FROMCLOCK1, clock1 / CLOCK_ROUGH, true );
	SetDlgItemInt( hDlg, IDC_FROMMEAS2 , meas2               , true );
	SetDlgItemInt( hDlg, IDC_FROMBEAT2 , beat2               , true );
	SetDlgItemInt( hDlg, IDC_FROMCLOCK2, clock2 / CLOCK_ROUGH, true );

	SetDlgItemInt( hDlg, IDC_BEATCLOCK, g_pxtn->master->get_beat_clock() / CLOCK_ROUGH, true );

	return true;
}

static void _GetInputParameter( HWND hDlg )
{
	int32_t meas1, beat1, clock1;
	int32_t meas2, beat2, clock2;

	meas1  = GetDlgItemInt( hDlg, IDC_FROMMEAS1,  NULL, true );
	beat1  = GetDlgItemInt( hDlg, IDC_FROMBEAT1,  NULL, true );
	clock1 = GetDlgItemInt( hDlg, IDC_FROMCLOCK1, NULL, true ) * CLOCK_ROUGH;
	meas2  = GetDlgItemInt( hDlg, IDC_FROMMEAS2,  NULL, true );
	beat2  = GetDlgItemInt( hDlg, IDC_FROMBEAT2,  NULL, true );
	clock2 = GetDlgItemInt( hDlg, IDC_FROMCLOCK2, NULL, true ) * CLOCK_ROUGH;

	clock1 = g_pxtn->master->get_this_clock( meas1, beat1, clock1 );
	clock2 = g_pxtn->master->get_this_clock( meas2, beat2, clock2 );
	if_ScopeField_SetScope( clock1, clock2 );
}


//コールバック
INT_PTR CALLBACK
dlg_Scope( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{

	// IME off
	switch( msg )
	{
	case WM_COMMAND:
		switch( LOWORD( w ) )
		{
		case IDC_FROMMEAS1:
		case IDC_FROMBEAT1:
		case IDC_FROMCLOCK1:
		case IDC_FROMBEAT2:
		case IDC_FROMMEAS2:
		case IDC_FROMCLOCK2:

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

		if( !_InitDialog(           hDlg, l?true:false ) ){
			EndDialog( hDlg, false );
			break;
		}
		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK:

			_GetInputParameter( hDlg );
			EndDialog( hDlg, true );

			break;

		case IDCANCEL:
			EndDialog( hDlg, false );
			break;


		}
		break;

	default:return false;

	}
	return true;
}
