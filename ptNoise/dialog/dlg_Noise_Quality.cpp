
#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../../ptConfig/ptConfig_DlgCtrl.h"

#include "../resource.h"
#include "../NoiseTable.h"

static void _InitDialog( HWND hDlg )
{
	ptConfig_cmb_quality_init( hDlg, IDC_COMBO_CHANNEL, IDC_COMBO_SPS, Japanese_Is() );

	Japanese_DialogItem_Change( hDlg );
}

static void _SetParameter( HWND hDlg, SAMPLINGQUALITY *p_c )
{

	ptConfig_cmb_quality_set( hDlg,
		IDC_COMBO_CHANNEL, p_c->ch ,
		IDC_COMBO_SPS    , p_c->sps );
}

static bool _GetInputParameter( HWND hDlg, SAMPLINGQUALITY *p_c )
{
	memset( p_c, 0, sizeof(SAMPLINGQUALITY) );

	ptConfig_cmb_quality_get( hDlg,
		IDC_COMBO_CHANNEL, &p_c->ch ,
		IDC_COMBO_SPS    , &p_c->sps );

	return true;
}

//コールバック
INT_PTR CALLBACK
dlg_NoiseDesign_Quality( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static SAMPLINGQUALITY *_p_config;
	static bool          _bInit = false;


	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		_p_config = (SAMPLINGQUALITY*)l;

		pxwWindowRect_center( hDlg );
		_InitDialog(   hDlg );
		_SetParameter( hDlg, _p_config );
		_bInit = true;

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		case IDOK:
			{
				SAMPLINGQUALITY config;
				if( _GetInputParameter( hDlg, &config ) )
				{
					*_p_config = config;
					EndDialog( hDlg, true );
				}
			}
			break;

		case IDCANCEL:
			_bInit = false;
			EndDialog( hDlg, false );
			break;

		case IDC_DEFAULT:
			{
				SAMPLINGQUALITY config;
				NoiseTable_SetDefaultQuality( &config );
				_SetParameter( hDlg,          &config );
			}
			break;

		}
		break;

	default:return false;

	}
	return true;
}
