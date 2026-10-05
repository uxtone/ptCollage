
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"


INT_PTR CALLBACK
dlg_Property( HWND hWnd, UINT msg, WPARAM w, LPARAM l )
{

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:
	{
		int32_t max, num;
		num = g_pxtn->evels->get_Count(); //TuneData_Unit_Event_CountAll();
		max = pxtnMAX_EVENTNUM;
		SetDlgItemInt( hWnd, IDC_EVENTNUM, num, true );
		SetDlgItemInt( hWnd, IDC_EVENTMAX, max, true );

		num = g_pxtn->Unit_Num();
		max = pxtnMAX_TUNEUNITSTRUCT;
		SetDlgItemInt( hWnd, IDC_UNITNUM , num, true );
		SetDlgItemInt( hWnd, IDC_UNITMAX , max, true );

		num = g_pxtn->Woice_Num();
		max = pxtnMAX_TUNEWOICESTRUCT;
		SetDlgItemInt( hWnd, IDC_WOICENUM, num, true );
		SetDlgItemInt( hWnd, IDC_WOICEMAX, max, true );
	}

		pxwWindowRect_center( hWnd );
		Japanese_DialogItem_Change( hWnd );

		return 1;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		//OK
		case IDOK:
			EndDialog( hWnd, true );
			break;
		}

	default: return false;

	}
	return true;
}
