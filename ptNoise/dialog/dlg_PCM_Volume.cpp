
#include <pxwWindowRect.h>
#include <pxwFile.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"


typedef struct
{
	float v;
}
CONVERTPCM_VOLUME;

static void _SetParameter( HWND hDlg, float *p_v )
{
	{
		TCHAR str[ 10 ];
		_stprintf_s( str, 10, _T("%0.2f"), *p_v );
		SetDlgItemText( hDlg, IDC_VOLUME_RATE, str );
	}
}

static bool _GetInputParameter( HWND hDlg, float *p_v )
{
	TCHAR str[ 10 ] = {0}; GetDlgItemText( hDlg, IDC_VOLUME_RATE, str, 10 ); *p_v = _ttof( str );
	return true;
}

//コールバック
INT_PTR CALLBACK
dlg_PCM_Volume( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static float* _p_v = NULL;

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		_p_v = (float*)l;

		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );
		_SetParameter( hDlg, _p_v );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		case IDOK:
			if( _GetInputParameter( hDlg, _p_v ) )
			{
				EndDialog( hDlg, true );
			}
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
