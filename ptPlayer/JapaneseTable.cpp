
#include "../Generic/Japanese.h"

#include "resource.h"


JAPANESETEXTSTRUCT_DLGITEM _DlgItem_table[] =
{
	{IDC_TEXT_ABOUTTITLE    , "≪このソフトウェアについて≫" },
	{IDC_TEXT_CONFIGTITLE   , "≪環境設定≫" },
	{IDC_DEFAULT            , "初期値" },
	{IDC_TEXT_DEVICE        , "デバイス" },
	{IDC_TEXT_SECOND        , "秒" },

	{IDC_TEXT_SOUNDQUALITY  , "音質" },
	{IDC_TEXT_CHANNEL       , "チャンネル" },
	{IDC_TEXT_SPS           , "秒間サンプル" },
	{IDC_TEXT_BUFFER        , "バッファ" },

	{IDC_TEXT_CONFIGTITLE   , "≪環境設定≫" },
	{IDC_TEXT_FONT          , "フォント"},
	{IDC_TEXT_BUILDOPTION   , "≪ビルド設定≫" },
	{IDC_TEXT_PLAYINFOMATION, "演奏データ情報" },
	{IDC_TEXT_SEC1          , "秒" },

	{IDC_TEXT_SEC2          , "秒" },
	{IDC_TEXT_SEC3          , "秒" },
	{IDC_TEXT_SEC4          , "秒" },
	{IDC_TEXT_PLAYSCOPE     , "ビルド範囲" },
	{IDC_TEXT_PLAYTIME      , "演奏時間" },

	{IDC_TEXT_EXTRAFADE     , "追加フェードアウト" },
	{IDC_TEXT_TOTALTIME     , "合計時間" },
	{IDC_TEXT_LOOPTIME      , "ループ部" },
	{IDC_TEXT_HEADTIME      , "前奏部" },
};

JAPANESETEXTSTRUCT_TEXTSET _MenuItem_table[] =
{
	{ "About",    "情報" },
	{ "Quit",    "終了" },
	{ "Volume",    "音量" },
	{ "Etc",    "その他" },
	{ "Config",    "環境設定" },

	{ "Setting",    "設定" },
	{ "File",    "ファイル" },
	{ "Load",    "読み込み" },
	{ "History",    "履歴" },
	{ "Export *.wav",    "wavファイルに出力" },

	{ "pxtone Collage",    "ピストンコラージュ" },
};

JAPANESETEXTSTRUCT_TEXTSET _Message_table[] =
{
	{ "open file", "ファイルが開けませんでした" },
	{ "read file", "ファイルが読めませんでした" },
	{ "unknown format", "無効なフォーマットです" },
	{ "error", "エラー" },
};

void JapaneseTable_init( bool b_japanese )
{
	Japanese_Set( b_japanese );
	Japanese_DialogItem_SetTable( _DlgItem_table , sizeof(_DlgItem_table ) / sizeof(_DlgItem_table [0]) );
	Japanese_MenuItem_SetTable  ( _MenuItem_table, sizeof(_MenuItem_table) / sizeof(_MenuItem_table[0]) );
	Japanese_Message_SetTable   ( _Message_table , sizeof(_Message_table ) / sizeof(_Message_table [0]) );
}

void JapaneseTable_Release()
{
	Japanese_Release();
}
