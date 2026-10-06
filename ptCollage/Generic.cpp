
#include <pxwFilePath.h>




// コンパイル日の取得
void GetCompileDate( int32_t *year, int32_t *month, int32_t *day )
{
	int32_t i;
	char strMonth[ 16 ];

	char *table[] =
	{
		"XXX",
		"Jan","Feb","Mar","Apr",
		"May","Jun","Jul","Aug",
		"Sep","Oct","Nov","Dec"
	};

	sscanf(__DATE__, "%s %d %d", strMonth, day, year );

	for( i = 0; i < 12; i++)
	{
		if( !memcmp( strMonth, table[i], 3 ) ) break;
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
	uxSS<MAX_PATH>             path = {0};

	int32_t v[ 4 ];

	memset( v, 0, sizeof(int32_t) * 4 );
	if( p1 ) *p1 = 0;
	if( p2 ) *p2 = 0;
	if( p3 ) *p3 = 0;
	if( p4 ) *p4 = 0;

	GetModuleFileName( NULL, uxTOut( path ), MAX_PATH );

	size = GetFileVersionInfoSize( uxT( path ), &dummy );
	if( !size ) goto End;

	p = malloc( size );
	if( !p                                                ) goto End;
	if( !GetFileVersionInfo( uxT( path ), 0, size, p )           ) goto End;
	if( !VerQueryValue( p, uxT( "\\" ), (LPVOID*)&info, &vSize ) ) goto End;

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


// 日付テスト
int32_t GetDateLimit( SYSTEMTIME *st_limit1, SYSTEMTIME *st_limit2 )
{
	SYSTEMTIME st_now;
	FILETIME ft_limit, ft_now;

	// 今の時間を取得
	GetSystemTime( &st_now );
	SystemTimeToFileTime( &st_now, &ft_now );

	SystemTimeToFileTime( st_limit1, &ft_limit );
	if( CompareFileTime( &ft_limit, &ft_now ) >= 0) return -1;

	SystemTimeToFileTime( st_limit2, &ft_limit );
	if( CompareFileTime( &ft_limit, &ft_now ) <= 0) return  1;

	return 0;
}

// ファイル名に追加 "c:\test.wav" -> "c:\test2.wav"
// 全角の拡張子は未対応
bool AddStringToFileName( uxDS& path, const uxDS& str )
{
	int32_t a = (int32_t)path.size();
	while( a > 0 && ( *path )[ a ] != '.' ) a--;
	if( a == 0 ) return false;
	uxDS ext( *path + a ); // ".wav"
	path.truncate( (size_t)a );
	path += str;
	path += ext;
	return true;
}

//サイズ取得 2147483647byteまで
int32_t GetFileSizeLong( uxDS& path )
{
	HANDLE  hFile;
	int32_t size ;

	hFile = CreateFile( uxT( path ), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE ) return -1;
	size = GetFileSize( hFile, NULL );
	CloseHandle       ( hFile );

	return size;
}



// シフトJISを確認
bool IsShiftJIS( unsigned char c )
{
	if( c >= 0x81 && c <= 0x9F ) return true;
	if( c >= 0xE0 && c <= 0xEF ) return true;

	return false;
}

//乱数の発生
int32_t Random( int32_t min, int32_t max )
{
	int32_t range;
	int32_t a;

	range = max - min + 1;

	SYSTEMTIME st;
	GetLocalTime( &st );

	for( a = 0; a < st.wSecond%10; a++ ) rand();

	return( ( rand() % range ) + min );
}

// 秒間描画フレーム数を取得（ループ内で呼ぶ）
int32_t CountFramePerSecond()
{
	static bool     bFirst    = true;
	static uint32_t wait      =    0;
	static uint32_t count     =    0;
	static int32_t  max_count =    0;

	if( bFirst )
	{
		wait   = GetTickCount();
		bFirst = false;
	}

	count++;
	if( wait + 1000 <= GetTickCount() )
	{
		wait     += 1000;
		max_count = count;
		count     = 0;
	}
	return max_count;
}
