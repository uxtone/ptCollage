
#include "../Generic/Japanese.h"

#include "resource.h"


JAPANESETEXTSTRUCT_DLGITEM _DlgItem_table[] =
{
	{IDC_TEXT_ABOUTTITLE       , _T("≪このソフトウェアについて≫") },
	{IDC_TEXT_CONFIGTITLE      , _T("≪環境設定≫"                ) },
	{IDC_DEFAULT               , _T("初期値"                      ) },
	{IDC_TEXT_SURETITLE        , _T("いいですか？"                ) },
	{IDC_TEXT_DEVICE           , _T("デバイス"                    ) },

	{IDC_TEXT_QUALITYTITLE     , _T("編集音質"                    ) },
	{IDC_TEXT_CHANNEL          , _T("チャンネル"                  ) },
	{IDC_TEXT_SPS              , _T("秒間サンプル"                ) },
	{IDC_TEXT_BUFFER           , _T("バッファ"                    ) }, // 10

	{IDC_TEXT_BUILD_QUALITY    , _T("ビルド音質"                  ) },
	{IDC_TEXT_BUILD_CHANNEL    , _T("チャンネル"                  ) },
	{IDC_TEXT_BUILD_SPS        , _T("秒間サンプル"                ) },
	{IDC_TEXT_SECOND           , _T("秒"                          ) },

	{IDC_TEXT_CONFIGTITLE      , _T("≪波形の設定≫"              ) },
	{IDC_TEXT_BASICKEY         , _T("基本キー"                    ) },
	{IDC_TEXT_HEARSELECT       , _T("≪音源の選択≫"              ) },
	{IDC_TEXT_SORT             , _T("並び"                        ) },
	{IDC_TEXT_FREQUENCY        , _T("周波数"                      ) }, // 20

	{IDC_TEXT_SEC_1            , _T("秒"                          ) },
	{IDC_TEXT_SEC_2            , _T("秒"                          ) },
	{IDC_TEXT_SEC_3            , _T("秒"                          ) },
	{IDC_TEXT_SEC_4            , _T("秒"                          ) },
	{IDC_TEXT_PAN              , _T("パン"                        ) },

	{IDC_TEXT_TYPE             , _T("タイプ"                      ) },
	{IDC_TEXT_VOLUME           , _T("ボリューム"                  ) },
	{IDC_TEXT_OFFSET           , _T("オフセット"                  ) },
	{IDC_TEXT_SAMPLE_NUM       , _T("サンプル数"                  ) },
	{IDC_TEXT_BUILD_QUALITY    , _T("ビルド音質"                  ) }, // 30

	{IDC_TEXT_ENVELOPE         , _T("エンベロープ"                ) },
	{IDC_TEXT_REVERSE          , _T("逆転"                        ) },
	{IDC_BUILD                 , _T("ビルド"                      ) },
	{IDC_TEXT_QUALITY          , _T("音質"                        ) },
	{IDC_TEXT_FROM             , _T("元"                          ) },

	{IDC_TEXT_TO               , _T("先"                          ) },
	{IDC_TEXT_VOLUMERATE       , _T("音量比率"                    ) },
};

JAPANESETEXTSTRUCT_TEXTSET _MenuItem_table[] =
{
	{ _T("About"            ),    _T("情報"                ) },
	{ _T("Quit"             ),    _T("終了"                ) },
	{ _T("Volume"           ),    _T("音量"                ) },
	{ _T("etc"              ),    _T("その他"              ) },
	{ _T("Config"           ),    _T("環境設定"            ) },

	{ _T("Unit"             ),    _T("音源設定"            ) },
	{ _T("Setting"          ),    _T("設定"                ) },
	{ _T("File"             ),    _T("ファイル"            ) },
	{ _T("Save As *.ptvoice"),    _T("別名で保存"          ) },
	{ _T("Save"             ),    _T("保存"                ) }, // 10

	{ _T("Load *.ptvoice"   ),    _T("読み込み(*.ptvoice)" ) },
	{ _T("Initialize"       ),    _T("初期化"              ) },
	{ _T("Edit"             ),    _T("編集"                ) },
	{ _T("Copy Layer"       ),    _T("レイヤーをコピー"    ) },
	{ _T("Build Quality"    ),    _T("ビルド音質"          ) },

	{ _T("Load *.ptnoise"   ),    _T("読み込み *.ptnoise"  ) },
	{ _T("Save *.ptnoise"   ),    _T("保存 *.ptnoise"      ) },
	{ _T("Save As *.ptnoise"),    _T("別名で保存 *.ptnoise") },
	{ _T("Load *.wav"       ),    _T("読み込み *.wav"      ) },
	{ _T("Save *.wav"       ),    _T("保存 *.wav"          ) }, // 20

	{ _T("Save As *.wav"    ),    _T("別名で保存 *.wav"    ) },
};

JAPANESETEXTSTRUCT_TEXTSET _Message_table[] =
{
	{ _T("open file"     ) , _T("ファイルが開けませんでした") },
	{ _T("read file"     ) , _T("ファイルが読めませんでした") },
	{ _T("unknown format") , _T("無効なフォーマットです"    ) },
	{ _T("error"         ) , _T("エラー"                    ) },
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
