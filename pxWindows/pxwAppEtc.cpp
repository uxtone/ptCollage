// '15/12/14 pxwAppEtc.cpp

#pragma comment(lib,"version")

bool pxwAppEtc_DisplaySize( int *pw, int *ph )
{
	if( pw ) *pw = GetSystemMetrics( SM_CXSCREEN );
	if( ph ) *ph = GetSystemMetrics( SM_CYSCREEN );
	return true;
}

void pxwAppEtc_OpenFile( HWND hwnd, const TCHAR* path_src )
{
	ShellExecute( hwnd, _T("open"), path_src, NULL, NULL, SW_SHOWNORMAL );
}

int pxwAppEtc_version( int *p1, int *p2, int *p3, int *p4 )
{
	LPVOID           p       = NULL;
	DWORD            dummy;
	DWORD            size;
	VS_FIXEDFILEINFO *info;
	UINT             vSize;
	TCHAR            path[ MAX_PATH ] = {0};

	int  v[ 4 ];

	memset( v, 0, sizeof(int ) * 4 );
	if( p1 ) *p1 = 0;
	if( p2 ) *p2 = 0;
	if( p3 ) *p3 = 0;
	if( p4 ) *p4 = 0;

	GetModuleFileName( NULL, path, MAX_PATH );

	size = GetFileVersionInfoSize( path, &dummy );
	if( !size ) goto End;

	p = malloc( size );
	if( !p                                                ) goto End;
	if( !GetFileVersionInfo( path, 0, size, p )           ) goto End;
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

void pxwAppEtc_sleep( float sec )
{
	Sleep( (DWORD)(sec * 1000) );
}


int pxwAppEtc_Local()
{
	int  ret = 0;
	LCID id  = GetUserDefaultLCID();

	switch( GetUserDefaultLCID() )
	{
	case  1041: ret = 1; break; // Japanese

	case  1034: //Spanish (Traditional Sort)
	case  2058: //Spanish (Mexican)
	case  3082: //Spanish (Modern Sort)
	case  4106: //Spanish (Guatemala)
	case  5130: //Spanish (Costa Rica)
	case  6154: //Spanish (Panama)
	case  7178: //Spanish (Dominican Republic)
	case  8202: //Spanish (Venezuela)
	case  9226: //Spanish (Colombia)
	case 10250: //Spanish (Peru)
	case 11274: //Spanish (Argentina)
	case 12298: //Spanish (Ecuador)
	case 13322: //Spanish (Chile)
	case 14346: //Spanish (Uruguay)
	case 15370: //Spanish (Paraguay)
	case 16394: //Spanish (Bolivia)
	case 17418: //Spanish (El Salvador)
	case 18442: //Spanish (Honduras)
	case 19466: //Spanish (Nicaragua)
	case 20490: //Spanish (Puerto Rico)
		ret = 2; break;

	case 1031: //German (Standard)
	case 2055: //German (Swiss)
	case 3079: //German (Austrian)
	case 4103: //German (Luxembourg)
	case 5127: //German (Liechtenstein)
		ret = 3; break;
	}

/*	int len = GetLocaleInfo( id, LOCALE_SENGLANGUAGE, NULL, 0 );
	if( len > 0 && len < 16 )
	{
		TCHAR buf[ 16 ];
		ZeroMemory( buf, 16 );
		GetLocaleInfo( id, LOCALE_SENGLANGUAGE, buf, len + 1 );
		if     ( !strcmp( buf, "Japanese" ) ) ret = 1; // Japanese
		else if( !strcmp( buf, "Espaniol" ) ) ret = 1; // Japanese
	}
*/
	return ret;
}

/*

LCID LocaleID=GetUserDefaultLCID();

switch (LocaleID) {
case 1025: //Arabic (Saudi Arabia)
case 2049: //Arabic (Iraq)
case 3073: //Arabic (Egypt)
case 4097: //Arabic (Libya)
case 5121: //Arabic (Algeria)
case 6145: //Arabic (Morocco)
case 7169: //Arabic (Tunisia)
case 8193: //Arabic (Oman)
case 9217: //Arabic (Yemen)
case 10241: //Arabic (Syria)
case 11265: //Arabic (Jordan)
case 12289: //Arabic (Lebanon)
case 13313: //Arabic (Kuwait)
case 14337: //Arabic (U.A.E.)
case 15361: //Arabic (Bahrain)
case 16385: //Arabic (Qatar)
case 1026: //Bulgarian
case 1027: //Catalan
case 1028: //Chinese (Taiwan)
case 2052: //Chinese (PRC)
case 3076: //Chinese (Hong Kong)
case 4100: //Chinese (Singapore)
case 1029: //Czech
case 1030: //Danish
case 1031: //German (Standard)
case 2055: //German (Swiss)
case 3079: //German (Austrian)
case 4103: //German (Luxembourg)
case 5127: //German (Liechtenstein)
case 1032: //Greek
case 1033: //English (United States)
case 2057: //English (United Kingdom)
case 3081: //English (Australian)
case 4105: //English (Canadian)
case 5129: //English (New Zealand)
case 6153: //English (Ireland)
case 7177: //English (South Africa)
case 8201: //English (Jamaica)
case 9225: //English (Caribbean)
case 10249: //English (Belize)
case 11273: //English (Trinidad)
case 1034: //Spanish (Traditional Sort)
case 2058: //Spanish (Mexican)
case 3082: //Spanish (Modern Sort)
case 4106: //Spanish (Guatemala)
case 5130: //Spanish (Costa Rica)
case 6154: //Spanish (Panama)
case 7178: //Spanish (Dominican Republic)
case 8202: //Spanish (Venezuela)
case 9226: //Spanish (Colombia)
case 10250: //Spanish (Peru)
case 11274: //Spanish (Argentina)
case 12298: //Spanish (Ecuador)
case 13322: //Spanish (Chile)
case 14346: //Spanish (Uruguay)
case 15370: //Spanish (Paraguay)
case 16394: //Spanish (Bolivia)
case 17418: //Spanish (El Salvador)
case 18442: //Spanish (Honduras)
case 19466: //Spanish (Nicaragua)
case 20490: //Spanish (Puerto Rico)
case 1035: //Finnish
case 1036: //French (Standard)
case 2060: //French (Belgian)
case 3084: //French (Canadian)
case 4108: //French (Swiss)
case 5132: //French (Luxembourg)
case 1037: //Hebrew
case 1038: //Hungarian
case 1039: //Icelandic
case 1040: //Italian (Standard)
case 2064: //Italian (Swiss)
case 1041: //Japanese
case 1042: //Korean
case 2066: //Korean (Johab)
case 1043: //Dutch (Standard)
case 2067: //Dutch (Belgian)
case 1044: //Norwegian (Bokmal)
case 2068: //Norwegian (Nynorsk)
case 1045: //Polish
case 1046: //Portuguese (Brazil)
case 2070: //Portuguese (Portugal)
case 1048: //Romanian
case 1049: //Russian
case 1050: //Croatian
case 2074: //Serbian (Latin)
case 3098: //Serbian (Cyrillic)
case 1051: //Slovak
case 1052: //Albanian
case 1053: //Swedish
case 2077: //Swedish (Finland)
case 1054: //Thai
case 1055: //Turkish
case 1057: //Indonesian
case 1058: //Ukrainian
case 1059: //Belarusian
case 1060: //Slovenian
case 1061: //Estonian
case 1062: //Latvian
case 1063: //Lithuanian
case 1065: //Farsi
case 1066: //Vietnamese
case 1069: //Basque
case :1078 //Afrikaans
case 1080: //Faeroese
}

*/

#include <pxError.h>
#include "./pxwUTF8.h"

void pxwAppEtc_ErrorMessageBox( HWND hwnd, const TCHAR* g_app_name )
{
	if( !pxError_is_error() )
	{
		MessageBox( NULL, _T("unknown error."), g_app_name, MB_ICONERROR );
	}
	else
	{
		TCHAR* err_txt = NULL;
#ifdef UNICODE
		if( !pxwUTF8_utf8_to_wide( pxError_get_message(), &err_txt, NULL ) )
#else
		if( !pxwUTF8_utf8_to_sjis( pxError_get_message(), &err_txt, NULL ) )
#endif
		{
			MessageBox( NULL, _T("ERR convert message."), g_app_name, MB_ICONERROR );
		}
		else
		{
			MessageBox( NULL, err_txt                   , g_app_name, MB_ICONERROR );
		}
	}
}
