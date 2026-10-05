
#include <pxwWindowRect.h>
#include <pxwPathDialog.h>
extern pxwPathDialog* g_path_dlg_ptn;

#include <pxtnPulse_NoiseBuilder.h>
extern pxtnPulse_NoiseBuilder *g_noise_bldr;

#include "../../Generic/Japanese.h"

#include "../resource.h"
#include "../NoiseTable.h"

#include <vector>
using namespace std;

void dlg_Noise_Design_Push      ( pxtnPulse_Noise *p_ptn );
void dlg_Noise_Design_GetQuality( SAMPLINGQUALITY *p_q   );

#define _DATE_TEXTSIZE 64

static TCHAR* _rect_name = _T("list_history.rect");
static HWND   _hList     = NULL ;
static BOOL   _b_Ret     = FALSE;

typedef struct
{
	TCHAR date[ _DATE_TEXTSIZE ];
	TCHAR *name;
	pxtnPulse_Noise *p_noise;
}
_NOISERECORD;

static vector <_NOISERECORD> _v_rec;

enum
{
	_COLUMN_INDEX = 0,
	_COLUMN_FILENAME ,
	_COLUMN_DATE,
};

static void _ListView_Resize( HWND hDlg )
{
	if( _hList )
	{
		RECT rc;
		GetClientRect( hDlg, &rc );
		SetWindowPos( _hList, NULL, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOZORDER );
	}
}

static void _ListView_Clear()
{
	ListView_DeleteAllItems( _hList );
}

static void _ListView_AddColumn( int fmt, int cx, LPTSTR text, int i )
{
	LV_COLUMN clmn;

	memset( &clmn, 0, sizeof(LV_COLUMN) );

	clmn.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	clmn.fmt      = fmt ;
	clmn.cx       = cx  ;
	clmn.pszText  = text;
	clmn.iSubItem = i   ;

	ListView_InsertColumn( _hList, i, &clmn );
}

static bool _dlg_List_FieldUnit_Init( HWND hDlg )
{
	{
		DWORD flags;

		_b_Ret = FALSE;

		_hList = GetDlgItem( hDlg, IDC_LIST );
		flags = ListView_GetExtendedListViewStyle( _hList );
		ListView_SetExtendedListViewStyle( _hList, flags | LVS_EX_FULLROWSELECT );
	}

	_ListView_AddColumn( LVCFMT_RIGHT,  30, _T("idx"      ), _COLUMN_INDEX    );
	_ListView_AddColumn( LVCFMT_LEFT , 100, _T("Name"     ), _COLUMN_FILENAME );
	_ListView_AddColumn( LVCFMT_LEFT ,  70, _T("Date Time"), _COLUMN_DATE     );

	pxwWindowRect_load( hDlg, _rect_name, TRUE, true );

	_ListView_Resize( hDlg );
	return true;
}

bool dlg_History_Push( const pxtnPulse_Noise *p_noise )
{
	if( _v_rec.size() && !_v_rec.at( 0 ).p_noise->Compare( p_noise ) ) return false;

	_NOISERECORD rec = {0};
	SYSTEMTIME   st  = {0};
	TCHAR        file_name[ MAX_PATH ] = {0};

	if( !g_path_dlg_ptn->last_filename_get( file_name ) ) return false;

	memset( &rec, 0, sizeof(rec) );
	rec.p_noise = new pxtnPulse_Noise( NULL, NULL, NULL, NULL );
	if( !rec.p_noise->copy_from( p_noise ) ) return false;

	GetLocalTime( &st );
	_stprintf_s( rec.date, _DATE_TEXTSIZE, _T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond );
	int len = _tcslen( file_name ) + 1;
	rec.name = (TCHAR*)malloc( len * sizeof(TCHAR) );
	memcpy( rec.name, file_name,  len * sizeof(TCHAR) );
	PathRemoveExtension( rec.name );

	_v_rec.insert( _v_rec.begin(), rec );

	{
		LV_ITEM item          = {0};
		TCHAR   str_num [ 8 ] = {0};

		item.mask     = LVIF_TEXT;

		_itot_s( ListView_GetItemCount(_hList), str_num, 8, 10 );
		item.iSubItem = _COLUMN_INDEX   ; item.pszText = str_num ; ListView_InsertItem( _hList, &item );
		item.iSubItem = _COLUMN_FILENAME; item.pszText = rec.name; ListView_SetItem   ( _hList, &item );
		item.iSubItem = _COLUMN_DATE    ; item.pszText = rec.date; ListView_SetItem   ( _hList, &item );
	}

	return true;
}


static void _IDM_CLEAR( HWND hDlg )
{
	for( unsigned int i = 0; i < _v_rec.size(); i++ )
	{
		SAFE_DELETE( _v_rec.at( i ).p_noise );
		if( _v_rec.at( i ).name ) free( _v_rec.at( i ).name );
	}
	ListView_DeleteAllItems( _hList );
	_v_rec.clear();
}

static void _NM_DBLCLK( HWND hDlg )
{
	int v = ListView_GetNextItem( _hList, -1, LVNI_ALL | LVNI_SELECTED );
	if( v == -1 ) return;
	dlg_Noise_Design_Push( _v_rec.at( v ).p_noise );
}

#include "../PcmTable.h"


static void _LVN_ITEMCHANGED( HWND hDlg )
{
	int v = ListView_GetNextItem( _hList, -1, LVNI_ALL | LVNI_SELECTED );
	if( v == -1 ) return;

	SAMPLINGQUALITY quality;

	dlg_Noise_Design_GetQuality( &quality );
	PcmTable_BuildAndPlay( _v_rec.at( v ).p_noise, &quality );
}

INT_PTR CALLBACK
dlg_History( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	case WM_INITDIALOG:

		_dlg_List_FieldUnit_Init( hDlg );
		break;

	case WM_CLOSE:
		_IDM_CLEAR( hDlg );
		EndDialog( hDlg, _b_Ret );
		break;

	case WM_MOVE:
		pxwWindowRect_save( hDlg, _rect_name );
		break;

	case WM_SIZE:

		switch( w )
		{
		case SIZE_MAXIMIZED:
		case SIZE_MINIMIZED:
		case SIZE_RESTORED :
			_ListView_Resize( hDlg );
			pxwWindowRect_save( hDlg, _rect_name );
			break;
		}
		break;


	case WM_COMMAND:

		switch( LOWORD(w) )
		{
		case IDM_CLEAR: _IDM_CLEAR( hDlg ); break;

		default: return FALSE;
		}
		break;

	case WM_NOTIFY:

		if( (int)w == IDC_LIST )
		{
			LPNMHDR pNtfy = (LPNMHDR)l;
			if( pNtfy->code == NM_DBLCLK       ) _NM_DBLCLK      ( hDlg );
			if( pNtfy->code == LVN_ITEMCHANGED ) _LVN_ITEMCHANGED( hDlg );
		}
		break;

	default: return FALSE;
	}
	return TRUE;
}
