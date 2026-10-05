
#include "../Generic/Japanese.h"

#include "resource.h"


JAPANESETEXTSTRUCT_DLGITEM _DlgItem_table[] =
{
	{IDC_TEXT_ABOUTTITLE    , _T("≪このソフトウェアについて≫") },
	{IDC_TEXT_CONFIGTITLE   , _T("≪環境設定≫"                ) },
	{IDC_DEFAULT            , _T("初期値"                      ) },
	{IDC_TEXT_DEVICE        , _T("デバイス"                    ) },
	{IDC_TEXT_SECOND        , _T("秒"                          ) },

	{IDC_TEXT_SOUNDQUALITY  , _T("音質"                        ) },
	{IDC_TEXT_CHANNEL       , _T("チャンネル"                  ) },
	{IDC_TEXT_SPS           , _T("秒間サンプル"                ) },
	{IDC_TEXT_BUFFER        , _T("バッファ"                    ) },

	{IDC_TEXT_CONFIGTITLE   , _T("≪環境設定≫"                ) },
	{IDC_TEXT_FONT          , _T("フォント"                    )},
	{IDC_TEXT_BUILDOPTION   , _T("≪ビルド設定≫"              ) },
	{IDC_TEXT_PLAYINFOMATION, _T("演奏データ情報"              ) },
	{IDC_TEXT_SEC1          , _T("秒"                          ) },

	{IDC_TEXT_SEC2          , _T("秒"                          ) },
	{IDC_TEXT_SEC3          , _T("秒"                          ) },
	{IDC_TEXT_SEC4          , _T("秒"                          ) },
	{IDC_TEXT_PLAYSCOPE     , _T("ビルド範囲"                  ) },
	{IDC_TEXT_PLAYTIME      , _T("演奏時間"                    ) },

	{IDC_TEXT_EXTRAFADE     , _T("追加フェードアウト"          ) },
	{IDC_TEXT_TOTALTIME     , _T("合計時間"                    ) },
	{IDC_TEXT_LOOPTIME      , _T("ループ部"                    ) },
	{IDC_TEXT_HEADTIME      , _T("前奏部"                      ) },
};

JAPANESETEXTSTRUCT_TEXTSET _MenuItem_table[] =
{
	{ _T("About"         ),    _T("情報"              ) },
	{ _T("Quit"          ),    _T("終了"              ) },
	{ _T("Volume"        ),    _T("音量"              ) },
	{ _T("Etc"           ),    _T("その他"            ) },
	{ _T("Config"        ),    _T("環境設定"          ) },

	{ _T("Setting"       ),    _T("設定"              ) },
	{ _T("File"          ),    _T("ファイル"          ) },
	{ _T("Load"          ),    _T("読み込み"          ) },
	{ _T("History"       ),    _T("履歴"              ) },
	{ _T("Export *.wav"  ),    _T("wavファイルに出力" ) },

	{ _T("pxtone Collage"),    _T("ピストンコラージュ") },
};

JAPANESETEXTSTRUCT_TEXTSET _Message_table[] =
{
	{ _T("open file"     ), _T("ファイルが開けませんでした") },
	{ _T("read file"     ), _T("ファイルが読めませんでした") },
	{ _T("unknown format"), _T("無効なフォーマットです"    ) },
	{ _T("error"         ), _T("エラー"                    ) },
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
