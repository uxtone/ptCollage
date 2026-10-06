


#include "resource.h"

typedef struct
{
	int    id    ;
	uxDS  pTextE;
	uxDS  pTextJ;
}
DLGITEMIDTEXT;

static DLGITEMIDTEXT _DlgItem_table[] =
{
	{IDC_TEXT_HEARSELECT       , "== Select File ==" , "≪音源の選択≫" },
	{IDC_CHECK_LOOP            , "Loop" , "ループ" },
	{IDC_TEXT_KEY              , "Key" , "キー" },
	{IDC_TEXT_SORT             , "Sort" , "並び" },
	{IDC_CHECK_ADDUNIT         , "Add Unit" , "ユニットも追加" },
};

#define CTRLNUM 5

void Japanese_Change_DialogItem( HWND hWnd, bool b_japanese )
{
	for( int i = 0; i < CTRLNUM; i++ )
	{
		if( GetDlgItem( hWnd, _DlgItem_table[i].id ) )
		{
			if( b_japanese ) SetDlgItemText( hWnd, _DlgItem_table[i].id, uxT( _DlgItem_table[i].pTextJ ) );
			else             SetDlgItemText( hWnd, _DlgItem_table[i].id, uxT( _DlgItem_table[i].pTextE ) );
		}
	}
}
