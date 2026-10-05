
#include "../Generic/Japanese.h"

#include "resource.h"


JAPANESETEXTSTRUCT_DLGITEM _DlgItem_table[] =
{
	{IDC_TEXT_ABOUTTITLE    , "≪このソフトウェアについて≫" },
	{IDC_TEXT_CONFIGTITLE   , "≪環境設定≫" },
	{IDC_DEFAULT            , "初期値" },
	{IDC_TEXT_SURETITLE     , "いいですか？" },
	{IDC_TEXT_AT_DEVICE     , "デバイス" },

	{IDC_TEXT_AT_QUALITY    , "編集音質" },
	{IDC_TEXT_AT_CHANNEL    , "チャンネル" },
	{IDC_TEXT_AT_SPS        , "秒間サンプル" },
	{IDC_TEXT_AT_BUFFER     , "バッファ" },

	{IDC_TEXT_BUILD_QUALITY , "ビルド音質" },
	{IDC_TEXT_BUILD_CHANNEL , "チャンネル" },
	{IDC_TEXT_BUILD_SPS     , "秒間サンプル" },
	{IDC_TEXT_SECOND        , "秒" },

	{IDC_TEXT_CONFIGTITLE   , "≪波形の設定≫" },
	{IDC_TEXT_BASICKEY      , "基本キー" },
	{IDC_TEXT_HEARSELECT    , "≪音源の選択≫" },
	{IDC_TEXT_KEY           , "キー" },
	{IDC_TEXT_SORT          , "並び" },

	{IDC_TEXT_MIDIDEVICE    , "MIDIデバイス" },
	{IDC_CHK_VELOCITY       , "ベロシティ" },
	{IDC_TEXT_KEYTUNING     , "キー補正" },

};

JAPANESETEXTSTRUCT_TEXTSET _MenuItem_table[] =
{
	{ "About", "情報" },
	{ "Quit", "終了" },
	{ "Volume", "音量" },
	{ "etc", "その他" },
	{ "Config", "環境設定" },

	{ "Unit", "音源設定" },
	{ "Setting", "設定" },
	{ "File", "ファイル" },
	{ "Save As *.ptvoice", "別名で保存" },
	{ "Save", "保存" },

	{ "Load *.ptvoice", "読み込み(*.ptvoice)" },
	{ "Initialize", "初期化" },
	{ "Edit", "編集" },
	{ "Copy A to B", "コピー( A -> B )" },
	{ "Copy B to A", "コピー( B -> A )" },
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
