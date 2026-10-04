
#include "resource.h"

typedef struct
{
	int    id    ;
	TCHAR* pTextE;
	TCHAR* pTextJ;
}
DLGITEMIDTEXT;

static DLGITEMIDTEXT _DlgItem_table[] =
{
	{IDC_TEXT_HEARSELECT       , _T("== Select File ==") , _T("≪音源の選択≫") },
	{IDC_CHECK_LOOP            , _T("Loop"             ) , _T("ループ"        ) },
	{IDC_TEXT_KEY              , _T("Key"              ) , _T("キー"          ) },
	{IDC_TEXT_SORT             , _T("Sort"             ) , _T("並び"          ) },
	{IDC_CHECK_ADDUNIT         , _T("Add Unit"         ) , _T("ユニットも追加") },
};

#define CTRLNUM 5

void Japanese_Change_DialogItem( HWND hWnd, bool b_japanese )
{
	for( int i = 0; i < CTRLNUM; i++ )
	{
		if( GetDlgItem( hWnd, _DlgItem_table[i].id ) )
		{
			if( b_japanese ) SetDlgItemText( hWnd, _DlgItem_table[i].id, _DlgItem_table[i].pTextJ );
			else             SetDlgItemText( hWnd, _DlgItem_table[i].id, _DlgItem_table[i].pTextE );
		}
	}
}

