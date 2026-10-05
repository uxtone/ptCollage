#include <windows.h>
#include <stdio.h>
#include <winnt.h>

#include "resource.h"
#include "dialog/dlg_Message.h"

#define PX_JAPANESE_INIT
#ifndef PX_JAPANESE_INIT

extern HINSTANCE g_hInst;

static BOOL _bJapanese;

typedef struct {
	int id;
	char *pTextE;
	char *pTextJ;
}DLGITEMIDTEXT;

DLGITEMIDTEXT _DlgItem_table[] = {
	{IDC_TEXT_ABOUTTITLE       , "== About ==",            "≪このソフトウェアについて≫" },
	{IDC_TEXT_CONFIGTITLE      , "== Config ==",           "≪環境設定≫"                 },
	{IDC_DEFAULT               , "Default",                "初期値"                       },
	{IDC_TEXT_SURETITLE        , "Are you sure?",          "いいですか？"                 },
	{IDC_TEXT_AT_DEVICE        , "Device",                 "デバイス"                     },

	{IDC_TEXT_AT_QUALITY       , "Operation Sound",        "編集音質"                     },
	{IDC_TEXT_AT_CHANNEL       , "Channel",                "チャンネル"                   },
//	{IDC_TEXT_AT_BPS           , "Bit/Sample",             "サンプルビット"               },
	{IDC_TEXT_AT_SPS           , "Sample/Second",          "秒間サンプル"                 },
	{IDC_TEXT_AT_BUFFER        , "Buffer",                 "バッファ"                     },

	{IDC_TEXT_BUILD_QUALITY    , "Build Quality",          "ビルド音質"                   },
	{IDC_TEXT_BUILD_CHANNEL    , "Channel",                "チャンネル"                   },
//	{IDC_TEXT_BUILD_BPS        , "Bit/Sample",             "サンプルビット"               },
	{IDC_TEXT_BUILD_SPS        , "Sample/Second",          "秒間サンプル"                 },
	{IDC_TEXT_SECOND           , "sec",                    "秒"                           },

	{IDC_TEXT_CONFIGTITLE      , "== Unit Config ==",      "≪波形の設定≫"               },
	{IDC_TEXT_BASICKEY         , "basic key"        ,      "基本キー"                     },
	{IDC_TEXT_HEARSELECT       , "== Select File ==",      "≪音源の選択≫"               },
	{IDC_TEXT_KEY              , "Key"                   , "キー"                         },
	{IDC_TEXT_SORT             , "Sort"                  , "並び"                         },
};


#define CTRLNUM 20

void Japanese_DialogItem_Change( HWND hWnd )
{
	long i;

	for( i = 0; i < CTRLNUM; i++ ){
		if( GetDlgItem( hWnd, _DlgItem_table[i].id ) ){
			if( _bJapanese ) SetDlgItemText( hWnd, _DlgItem_table[i].id, (LPTSTR)_DlgItem_table[i].pTextJ );
			else             SetDlgItemText( hWnd, _DlgItem_table[i].id, (LPTSTR)_DlgItem_table[i].pTextE );
		}
	}
}



DLGITEMIDTEXT _MenuItem_table[] = {
	{0   , "About",                "情報"                 },
	{0   , "Quit",                 "終了"                 },
	{0   , "Volume",               "音量"                 },
	{0   , "etc",                  "その他"               },
	{0   , "Config",               "環境設定"             },

	{0   , "Unit",                 "音源設定"             },
	{0   , "Setting",              "設定"                 },
	{0   , "File",                 "ファイル"             },
	{0   , "Save As *.ptvoice",    "別名で保存"           },
	{0   , "Save",                 "保存"                 },

	{0   , "Load *.ptvoice",       "読み込み(*.ptvoice)"  },
	{0   , "Initialize"    ,       "初期化"               },
	{0   , "Edit"          ,       "編集"                 },
	{0   , "Copy A to B"   ,       "コピー( A -> B )"     },
	{0   , "Copy B to A"   ,       "コピー( B -> A )"     },
};

#define MENUNUM 15

void Japanese_MenuItem_Change( HMENU hMenu )
{
	long pos;
	long i;
	long id;
	char str[32];

	HMENU hSub;


	for( pos = 0; pos < MENUNUM; pos++ ){
		if( GetMenuString( hMenu, pos, (LPTSTR)str, 32, MF_BYPOSITION ) ){

			// 該当を探す
			for( i = 0; i < MENUNUM; i++ ){
				if( _bJapanese ){
					if( !strcmp( str, _MenuItem_table[i].pTextE ) ) break;
				}else{
					if( !strcmp( str, _MenuItem_table[i].pTextJ ) ) break;
				}
			}
			// 書きかえ
			if( i != MENUNUM ){

				id = GetMenuItemID( hMenu, pos );
				if( id == 0xffffffff ){
					hSub = GetSubMenu( hMenu, pos );
					if( hSub ){
						if( _bJapanese ){
							ModifyMenu( hMenu, pos, MF_BYPOSITION|MFT_STRING, (INT_PTR)hSub, (LPTSTR)_MenuItem_table[i].pTextJ );
						}else{
							ModifyMenu( hMenu, pos, MF_BYPOSITION|MFT_STRING, (INT_PTR)hSub, (LPTSTR)_MenuItem_table[i].pTextE );
						}
						// 再帰！！
						Japanese_MenuItem_Change( hSub );
					}
				}else{
					if( _bJapanese ){
						ModifyMenu( hMenu, pos, MF_BYPOSITION|MFT_STRING, id,   (LPTSTR)_MenuItem_table[i].pTextJ );
					}else{
						ModifyMenu( hMenu, pos, MF_BYPOSITION|MFT_STRING, id,   (LPTSTR)_MenuItem_table[i].pTextE );
					}
				}
			}
		}
	}
}


DLGITEMIDTEXT _Message_table[] = {
	{0   , "open file",                 "ファイルが開けませんでした"          },
	{0   , "read file",                 "ファイルが読めませんでした"          },
	{0   , "unknown format",            "無効なフォーマットです"              },
	{0   , "error",                     "エラー"                              },
};


#define MESSAGENUM 4


void Japanese_MessageBox( HWND hWnd, const char *message, const char *title )
{
	MESSAGEDIALOGSTRUCT msg;
	long i;

	msg.p_message = (LPTSTR)message;
	msg.p_title   = (LPTSTR)title;

	if( _bJapanese ){
		for( i = 0; i < MESSAGENUM; i++ ){ if( !stricmp( message, _Message_table[ i ].pTextE ) ) break; }
		if(         i < MESSAGENUM ) msg.p_message = (LPTSTR)_Message_table[ i ].pTextJ;

		for( i = 0; i < MESSAGENUM; i++ ){ if( !stricmp( title,   _Message_table[ i ].pTextE ) ) break; }
		if(         i < MESSAGENUM ) msg.p_title   = (LPTSTR)_Message_table[ i ].pTextJ;
	}

//TODO: find out where this symbol comes from
//    DialogBoxParam( g_hInst, "DLG_MESSAGE", hWnd, DialogProc_Message, (LPARAM)&msg );
}

BOOL Japanese_Is( void ){ return _bJapanese; }
void Japanese_Set( BOOL b ){ _bJapanese = b; }

#endif
#define PX_JAPANESE_INIT
