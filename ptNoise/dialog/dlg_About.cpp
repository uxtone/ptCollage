
#include <pxStdDef.h>

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"

TCHAR *gStrVersion = _T("version.%d.%d.%d.%d - %04d/%02d/%02d");

// コンパイル日の取得
void GetCompileDate( int32_t *year, int32_t *month, int32_t *day )
{
	int32_t i;
	char strMonth[16];

	char *table[] =
	{
		"XXX",
		"Jan","Feb","Mar","Apr",
		"May","Jun","Jul","Aug",
		"Sep","Oct","Nov","Dec"
	};

	sscanf(__DATE__, "%s %d %d", strMonth, day, year );

	for(i = 0; i < 12; i++){
		if( !memcmp( strMonth, table[i], 3 ) )break;
	}
	*month = i;

	return;
}

#pragma comment(lib,"version")

// リソースバージョンを取得
int32_t GetCompileVersion( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 )
{
	LPVOID           p       = NULL;
	DWORD            dummy;
	DWORD            size;
	VS_FIXEDFILEINFO *info;
	UINT             vSize;
	TCHAR path[ MAX_PATH ] = {0};

	int32_t v[ 4 ];

	memset( v, 0, sizeof(int32_t) * 4 );
	if( p1 ) *p1 = 0;
	if( p2 ) *p2 = 0;
	if( p3 ) *p3 = 0;
	if( p4 ) *p4 = 0;

	GetModuleFileName( NULL, path, MAX_PATH );

	size = GetFileVersionInfoSize( path, &dummy );
	if( !size ) goto End;

	p = malloc( size );
	if( !p                                                    ) goto End;
	if( !GetFileVersionInfo( path, 0, size, p )               ) goto End;
	if( !VerQueryValue( p, _T("\\"), (LPVOID*)&info, &vSize ) ) goto End;

	v[ 0 ] = HIWORD(info->dwFileVersionMS);
	v[ 1 ] = LOWORD(info->dwFileVersionMS);
	v[ 2 ] = HIWORD(info->dwFileVersionLS);
	v[ 3 ] = LOWORD(info->dwFileVersionLS);

	if( p1 ) *p1 = v[ 0 ];
	if( p2 ) *p2 = v[ 1 ];
	if( p3 ) *p3 = v[ 2 ];
	if( p4 ) *p4 = v[ 3 ];

End:
	if( p ) free( p );

	return v[ 0 ] * 1000 + v[ 1 ] * 100 + v[ 2 ] * 10 + v[ 3 ];
}

INT_PTR CALLBACK
dlg_About( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		{
			TCHAR str[64];
			int32_t y, m, d;
			int32_t v1,v2,v3,v4;
			GetCompileDate( &y, &m, &d );
			GetCompileVersion( &v1, &v2, &v3, &v4 );
			_stprintf_s( str, 64, gStrVersion, v1, v2, v3, v4, y, m, d );
			SetDlgItemText( hDlg, IDC_VERSION, str );
		}

		SetDlgItemText( hDlg, IDC_MESSAGE, _T("beta test") );

		pxwWindowRect_center      ( hDlg );
		Japanese_DialogItem_Change( hDlg );


		return 1;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		//キャンセル
		case IDCANCEL:
			EndDialog( hDlg, false );
			break;
		//OK
		case IDOK:
			EndDialog( hDlg, true );
			break;
		}

		default:return false;

	}
	return true;
}
