
#include "../Generic/Japanese.h"

#include "resource.h"

JAPANESETEXTSTRUCT_DLGITEM _DlgItem_table[] =
{
	{IDC_TEXT_ABOUTTITLE       , _T("≪このソフトウェアについて≫") },
	{IDC_TEXT_CONFIGTITLE      , _T("≪環境設定≫"                ) },
	{IDC_TEXT_COPYMEASTITLE    , _T("≪小節コピー≫"              ) },
	{IDC_TEXT_COPYNOTETITLE    , _T("≪音符コピー≫"              ) },
	{IDC_TEXT_DELETEMEASTITLE  , _T("≪小節削除≫"                ) },

	{IDC_TEXT_PROJECTTITLE     , _T("≪プロジェクト設定≫"        ) },
	{IDC_TEXT_PROPERTYTITLE    , _T("≪アプリケーション情報≫"    ) },
	{IDC_TEXT_UNITTITLE        , _T("≪ユニット設定≫"            ) },
	{IDC_TEXT_DELAYTITLE       , _T("≪ディレイ設定≫"            ) },
	{IDC_TEXT_HEARSELECT       , _T("≪音源の選択≫"              ) },// 10

	{IDC_TEXT_NAME             , _T("名前"                        ) },
	{IDC_TEXT_TEMPO            , _T("ビートテンポ"                ) },
	{IDC_TEXT_MEAS             , _T("小節"                        ) },
	{IDC_TEXT_BEAT             , _T("拍子"                        ) },
	{IDC_TEXT_UNITS            , _T("ユニット"                    ) },

	{IDC_TEXT_SOUNDQUALITY     , _T("音質"                        ) },
	{IDC_TEXT_CHANNEL          , _T("チャンネル"                  ) },
	{IDC_TEXT_SPS              , _T("秒間サンプル"                ) },
	{IDC_TEXT_BUFFER           , _T("バッファ"                    ) },// 20

	{IDC_TEXT_SECOND           , _T("秒"                          ) },
	{IDC_TEXT_FROM             , _T("元"                          ) },
	{IDC_TEXT_TO               , _T("先"                          ) },
	{IDC_TEXT_MEAS2            , _T("小節"                        ) },
	{IDC_TEXT_CLOCK            , _T("クロック"                    ) },

	{IDC_TEXT_TIME             , _T("回数"                        ) },
	{IDC_TEXT_CPB              , _T("クロック／拍"                ) },
	{IDC_TEXT_UNIT             , _T("ユニット"                    ) },
	{IDC_TEXT_UNIT2            , _T("ユニット"                    ) },
	{IDC_TEXT_EVENT            , _T("イベント"                    ) },// 30

	{IDC_TEXT_STATUS           , _T("情報"                        ) },
	{IDC_TEXT_TOTALSAMPLE      , _T("サンプリングサイズ"          ) },
	{IDC_TEXT_EVENTNUM         , _T("イベント数"                  ) },
	{IDC_CHECK_BUILD           , _T("ビルドする"                  ) },
	{IDC_CHECK_WAVELOOP        , _T("ループ"                      ) },

	{IDC_CHECK_SMOOTH          , _T("音末処理"                    ) },
	{IDC_TEXT_FREQUENCY        , _T("周波数"                      ) },
	{IDC_TEXT_RATE             , _T("割合"                        ) },
	{IDC_ALLUNIT               , _T("全て"                        ) },
	{IDC_RELOAD                , _T("読み直し"                    ) },// 40

	{IDC_TEXT_COMMENTTITLE     , _T("≪コメント≫"                ) },
	{IDC_EXPORT                , _T("書き出し"                    ) },
	{IDC_TEXT_TUNING           , _T("補正"                        ) },
	{IDC_TEXT_BASICKEY         , _T("基本キー"                    ) },
	{IDC_TEXT_DELAYSCALE       , _T("単位"                        ) },

	{IDC_TEXT_SURETITLE        , _T("いいですか？"                ) },
	{IDC_TEXT_GROUP            , _T("グループ"                    ) },
	{IDC_TEXT_VALUE            , _T("値"                          ) },
	{IDC_TEXT_DEVICE           , _T("デバイス"                    ) },
	{IDC_DEFAULT               , _T("初期値"                      ) },// 50

	{IDC_CHECK_BEATFIT         , _T("拍で補正"                    ) },
	{IDC_TEXT_OVERDRIVETITLE   , _T("≪オーバードライブ設定≫"    ) },
	{IDC_CHECK_RENAME          , _T("ファイル名を採用"            ) },
	{IDC_TEXT_GATE1            , _T("ゲート(-)"                   ) },
	{IDC_TEXT_GATE2            , _T("ゲート(+)"                   ) },

	{IDC_TEXT_AMP              , _T("増幅"                        ) },
	{IDC_CHECK_LOOP            , _T("ループ"                      ) },
	{IDC_TEXT_MEASNUM          , _T("小節数"                      ) },
	{IDC_TEXT_START            , _T("先頭"                        ) },
	{IDC_TEXT_END              , _T("末尾"                        ) },// 60

	{IDC_TEXT_SCOPE            , _T("≪選択範囲≫"                ) },
	{IDC_TEXT_VOICETITLE       , _T("≪音源設定≫"                ) },
	{IDC_TEXT_TYPE             , _T("タイプ"                      ) },
	{IDC_TEXT_VOICE            , _T("音源"                        ) },
	{IDC_TEXT_ADDUNITTITLE     , _T("≪ユニット追加≫"            ) },

	{IDC_TEXT_VOICE            , _T( "音源"                       ) },
	{IDC_TEXT_FONT             , _T( "フォント"                   ) },
	{IDC_CHECK_UNITMUTE        , _T( "ユニットの消音を適用する"   ) },
	{IDC_TEXT_BUILDOPTION      , _T("≪ビルド設定≫"              ) },
	{IDC_TEXT_PLAYINFOMATION   , _T("演奏データ情報"              ) },// 70

	{IDC_TEXT_SEC1             , _T("秒"                          ) },
	{IDC_TEXT_SEC2             , _T("秒"                          ) },
	{IDC_TEXT_SEC3             , _T("秒"                          ) },
	{IDC_TEXT_SEC4             , _T("秒"                          ) },
	{IDC_TEXT_PLAYSCOPE        , _T("ビルド範囲"                  ) },

	{IDC_TEXT_PLAYTIME         , _T("演奏時間"                    ) },
	{IDC_TEXT_EXTRAFADE        , _T("追加フェードアウト"          ) },
	{IDC_TEXT_TOTALTIME        , _T("合計時間"                    ) },
	{IDC_TEXT_LOOPTIME         , _T("ループ部"                    ) },
	{IDC_TEXT_KEY              , _T("キー"                        ) },

	{IDC_TEXT_SORT             , _T("並び"                        ) },// 80
	{IDC_CHECK_ADDUNIT         , _T("ユニットも追加"              ) },
	{IDC_DELETE                , _T("削除"                        ) },
	{IDC_TEXT_CUT              , _T("カット"                      ) },
	{IDC_TEXT_MIDIDEVICE       , _T("MIDIデバイス"                ) },

	{IDC_CHK_VELOCITY          , _T("ベロシティ"                  ) },
	{IDC_TEXT_HEADTIME         , _T("前奏部"                      ) },
};


JAPANESETEXTSTRUCT_TEXTSET _MenuItem_table[] =
{
	{ _T("Edit"                ), _T("編集"                      ) },
	{ _T("Output [*.wav]"      ), _T("曲ファイルの出力[*.wav]"   ) },
	{ _T("Output [*.pttune]"   ), _T("曲ファイルの出力[*.pttune]") },
	{ _T("About"               ), _T("情報"                      ) },
	{ _T("Load Project"        ), _T("読み込み"                  ) },

	{ _T("Save Project\tCtrl+S"), _T("上書き保存\tCtrl+S"        ) },
	{ _T("Save Project as"     ), _T("名前を付けて保存"          ) },
	{ _T("Initialize Project"  ), _T("初期化"                    ) },
	{ _T("Quit"                ), _T("終了"                      ) },
	{ _T("Property"            ), _T("プロパティ"                ) }, // 10

	{ _T("Stop"                ), _T("中止"                      ) },
	{ _T("Undo\tCtrl+Z"        ), _T("１つ戻す\tCtrl+Z"          ) },
	{ _T("Redo\tCtrl+Y"        ), _T("やり直し\tCtrl+Y"          ) },
	{ _T("Volume Control"      ), _T("音量"                      ) },
	{ _T("etc"                 ), _T("その他"                    ) },

	{ _T("Effect"              ), _T("効果"                      ) },
	{ _T("Transpose"           ), _T("トランスポーズ"            ) },
	{ _T("Config"              ), _T("環境設定"                  ) },
	{ _T("Setting"             ), _T("設定"                      ) },
	{ _T("File"                ), _T("ファイル"                  ) },// 20

	{ _T("*"                   ), _T("*"                         ) },
	{ _T("Output Tune File"    ), _T("曲ファイルの出力"          ) },
	{ _T("Copy Meas"           ), _T("小節コピー"                ) },
	{ _T("Volume Pan"          ), _T("パン(音量)"                ) },
	{ _T("Velocity"            ), _T("ベロシティ"                ) },

	{ _T("Volume"              ), _T("ボリューム"                ) },
	{ _T("Scope"               ), _T("範囲"                      ) },
	{ _T("History"             ), _T("履歴"                      ) },
	{ _T("Time Pan"            ), _T("パン(時間差)"              ) },
};


JAPANESETEXTSTRUCT_TEXTSET _Message_table[] =
{
	{ _T("meas"                     ), _T("小節の指定が異常です"               ) },
	{ _T("tempo (20 - 600)"         ), _T("テンポは 20 から 600 までです"      ) },
	{ _T("copy time"                ), _T("回数の指定が異常です"               ) },
	{ _T("from meas"                ), _T("小節の指定が異常です"               ) },
	{ _T("init event"               ), _T("イベントの初期化に失敗しました"     ) },

	{ _T("open file"                ), _T("ファイルが開けませんでした"         ) },
	{ _T("output event"             ), _T("イベントが書き出しに失敗しました"   ) },
	{ _T("read file"                ), _T("ファイルが読めませんでした"         ) },
	{ _T("unknown format"           ), _T("無効なフォーマットです"             ) },
	{ _T("make wave data"           ), _T("波形データを作れませんでした"       ) },// 10

	{ _T("make active wave"         ), _T("再生用波形データを作れませんでした" ) },
	{ _T("read event"               ), _T("イベントが読み込みに失敗しました"   ) },
	{ _T("unit full"                ), _T("最大ユニット数を超えています"       ) },
	{ _T("Delay Frequency (0 - 100)"), _T("ディレイの周波数は 100Hz が最大です") },
	{ _T("Delay Rate (0 - 100)"     ), _T("ディレイの割合は 100% が最大です"   ) },

	{ _T("thread"                   ), _T("スレッドが作れませんでした"         ) },
	{ _T("no unit"                  ), _T("ユニットがありません"               ) },
	{ _T("material unit no"         ), _T("素材に対応するユニットがありません" ) },
	{ _T("build"                    ), _T("ビルドに失敗しました"               ) },
	{ _T("ready build"              ), _T("ビルドの準備に失敗しました"         ) },// 20

	{ _T("anti operation"           ), _T("編集できないファイルです"           ) },
	{ _T("initialize sound"         ), _T("音声の初期化に失敗しました"         ) },
	{ _T("group"                    ), _T("グループ"                           ) },
	{ _T("error"                    ), _T("エラー"                             ) },
	{ _T("not found"                ), _T("見つかりません"                     ) },
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
