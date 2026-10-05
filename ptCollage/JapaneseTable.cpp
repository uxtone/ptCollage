
#include "../Generic/Japanese.h"

#include "resource.h"

JAPANESETEXTSTRUCT_DLGITEM _DlgItem_table[] =
{
	{IDC_TEXT_ABOUTTITLE       , "≪このソフトウェアについて≫" },
	{IDC_TEXT_CONFIGTITLE      , "≪環境設定≫"               },
	{IDC_TEXT_COPYMEASTITLE    , "≪小節コピー≫"             },
	{IDC_TEXT_COPYNOTETITLE    , "≪音符コピー≫"             },
	{IDC_TEXT_DELETEMEASTITLE  , "≪小節削除≫"               },

	{IDC_TEXT_PROJECTTITLE     , "≪プロジェクト設定≫"        },
	{IDC_TEXT_PROPERTYTITLE    , "≪アプリケーション情報≫"    },
	{IDC_TEXT_UNITTITLE        , "≪ユニット設定≫"            },
	{IDC_TEXT_DELAYTITLE       , "≪ディレイ設定≫"            },
	{IDC_TEXT_HEARSELECT       , "≪音源の選択≫"              },// 10

	{IDC_TEXT_NAME             , "名前"                        },
	{IDC_TEXT_TEMPO            , "ビートテンポ"                },
	{IDC_TEXT_MEAS             , "小節"                        },
	{IDC_TEXT_BEAT             , "拍子"                        },
	{IDC_TEXT_UNITS            , "ユニット"                    },

	{IDC_TEXT_SOUNDQUALITY     , "音質"                        },
	{IDC_TEXT_CHANNEL          , "チャンネル"                  },
	{IDC_TEXT_SPS              , "秒間サンプル"                },
	{IDC_TEXT_BUFFER           , "バッファ"                    },// 20

	{IDC_TEXT_SECOND           , "秒"                          },
	{IDC_TEXT_FROM             , "元"                          },
	{IDC_TEXT_TO               , "先"                          },
	{IDC_TEXT_MEAS2            , "小節"                        },
	{IDC_TEXT_CLOCK            , "クロック"                    },

	{IDC_TEXT_TIME             , "回数"                        },
	{IDC_TEXT_CPB              , "クロック／拍"                },
	{IDC_TEXT_UNIT             , "ユニット"                    },
	{IDC_TEXT_UNIT2            , "ユニット"                    },
	{IDC_TEXT_EVENT            , "イベント"                    },// 30

	{IDC_TEXT_STATUS           , "情報"                        },
	{IDC_TEXT_TOTALSAMPLE      , "サンプリングサイズ"          },
	{IDC_TEXT_EVENTNUM         , "イベント数"                  },
	{IDC_CHECK_BUILD           , "ビルドする"                  },
	{IDC_CHECK_WAVELOOP        , "ループ"                      },

	{IDC_CHECK_SMOOTH          , "音末処理"                    },
	{IDC_TEXT_FREQUENCY        , "周波数"                      },
	{IDC_TEXT_RATE             , "割合"                        },
	{IDC_ALLUNIT               , "全て"                        },
	{IDC_RELOAD                , "読み直し"                    },// 40

	{IDC_TEXT_COMMENTTITLE     , "≪コメント≫"                },
	{IDC_EXPORT                , "書き出し"                    },
	{IDC_TEXT_TUNING           , "補正"                        },
	{IDC_TEXT_BASICKEY         , "基本キー"                    },
	{IDC_TEXT_DELAYSCALE       , "単位"                        },

	{IDC_TEXT_SURETITLE        , "いいですか？"                },
	{IDC_TEXT_GROUP            , "グループ"                    },
	{IDC_TEXT_VALUE            , "値"                          },
	{IDC_TEXT_DEVICE           , "デバイス"                    },
	{IDC_DEFAULT               , "初期値"                      },// 50

	{IDC_CHECK_BEATFIT         , "拍で補正"                    },
	{IDC_TEXT_OVERDRIVETITLE   , "≪オーバードライブ設定≫"    },
	{IDC_CHECK_RENAME          , "ファイル名を採用"            },
	{IDC_TEXT_GATE1            , "ゲート(-)"                   },
	{IDC_TEXT_GATE2            , "ゲート(+)"                   },

	{IDC_TEXT_AMP              , "増幅"                        },
	{IDC_CHECK_LOOP            , "ループ"                      },
	{IDC_TEXT_MEASNUM          , "小節数"                      },
	{IDC_TEXT_START            , "先頭"                        },
	{IDC_TEXT_END              , "末尾"                        },// 60

	{IDC_TEXT_SCOPE            , "≪選択範囲≫"                },
	{IDC_TEXT_VOICETITLE       , "≪音源設定≫"                },
	{IDC_TEXT_TYPE             , "タイプ"                      },
	{IDC_TEXT_VOICE            , "音源"                        },
	{IDC_TEXT_ADDUNITTITLE     , "≪ユニット追加≫"            },

	{IDC_TEXT_VOICE            ,  "音源"                       },
	{IDC_TEXT_FONT             ,  "フォント"                   },
	{IDC_CHECK_UNITMUTE        ,  "ユニットの消音を適用する"   },
	{IDC_TEXT_BUILDOPTION      , "≪ビルド設定≫"              },
	{IDC_TEXT_PLAYINFOMATION   , "演奏データ情報"              },// 70

	{IDC_TEXT_SEC1             , "秒"                          },
	{IDC_TEXT_SEC2             , "秒"                          },
	{IDC_TEXT_SEC3             , "秒"                          },
	{IDC_TEXT_SEC4             , "秒"                          },
	{IDC_TEXT_PLAYSCOPE        , "ビルド範囲"                  },

	{IDC_TEXT_PLAYTIME         , "演奏時間"                    },
	{IDC_TEXT_EXTRAFADE        , "追加フェードアウト"          },
	{IDC_TEXT_TOTALTIME        , "合計時間"                    },
	{IDC_TEXT_LOOPTIME         , "ループ部"                    },
	{IDC_TEXT_KEY              , "キー"                        },

	{IDC_TEXT_SORT             , "並び"                        },// 80
	{IDC_CHECK_ADDUNIT         , "ユニットも追加"              },
	{IDC_DELETE                , "削除"                        },
	{IDC_TEXT_CUT              , "カット"                      },
	{IDC_TEXT_MIDIDEVICE       , "MIDIデバイス"                },

	{IDC_CHK_VELOCITY          , "ベロシティ"                  },
	{IDC_TEXT_HEADTIME         , "前奏部"                      },
};


JAPANESETEXTSTRUCT_TEXTSET _MenuItem_table[] =
{
	{ "Edit"                , "編集"                     },
	{ "Output [*.wav]"      , "曲ファイルの出力[*.wav]"    },
	{ "Output [*.pttune]"   , "曲ファイルの出力[*.pttune]" },
	{ "About"               , "情報"                     },
	{ "Load Project"        , "読み込み"                  },

	{ "Save Project\tCtrl+S", "上書き保存\tCtrl+S"        },
	{ "Save Project as"     , "名前を付けて保存"           },
	{ "Initialize Project"  , "初期化"                   },
	{ "Quit"                , "終了"                     },
	{ "Property"            , "プロパティ"                }, // 10

	{ "Stop"                , "中止"                     },
	{ "Undo\tCtrl+Z"        , "１つ戻す\tCtrl+Z"          },
	{ "Redo\tCtrl+Y"        , "やり直し\tCtrl+Y"          },
	{ "Volume Control"      , "音量"                     },
	{ "etc"                 , "その他"                    },

	{ "Effect"              , "効果"                     },
	{ "Transpose"           , "トランスポーズ"             },
	{ "Config"              , "環境設定"                  },
	{ "Setting"             , "設定"                     },
	{ "File"                , "ファイル"                  },// 20

	{ "*"                   , "*"                        },
	{ "Output Tune File"    , "曲ファイルの出力"            },
	{ "Copy Meas"           , "小節コピー"                 },
	{ "Volume Pan"          , "パン(音量)"                 },
	{ "Velocity"            , "ベロシティ"                 },

	{ "Volume"              , "ボリューム"                },
	{ "Scope"               , "範囲"                      },
	{ "History"             , "履歴"                      },
	{ "Time Pan"            , "パン(時間差)"              },
};


JAPANESETEXTSTRUCT_TEXTSET _Message_table[] =
{
	{ "meas", "小節の指定が異常です" },
	{ "tempo (20 - 600)", "テンポは 20 から 600 までです" },
	{ "copy time", "回数の指定が異常です" },
	{ "from meas", "小節の指定が異常です" },
	{ "init event", "イベントの初期化に失敗しました" },

	{ "open file", "ファイルが開けませんでした" },
	{ "output event", "イベントが書き出しに失敗しました" },
	{ "read file", "ファイルが読めませんでした" },
	{ "unknown format", "無効なフォーマットです" },
	{ "make wave data", "波形データを作れませんでした" },// 10

	{ "make active wave", "再生用波形データを作れませんでした" },
	{ "read event", "イベントが読み込みに失敗しました" },
	{ "unit full", "最大ユニット数を超えています" },
	{ "Delay Frequency (0 - 100)", "ディレイの周波数は 100Hz が最大です" },
	{ "Delay Rate (0 - 100)", "ディレイの割合は 100% が最大です" },

	{ "thread", "スレッドが作れませんでした" },
	{ "no unit", "ユニットがありません" },
	{ "material unit no", "素材に対応するユニットがありません" },
	{ "build", "ビルドに失敗しました" },
	{ "ready build", "ビルドの準備に失敗しました" },// 20

	{ "anti operation", "編集できないファイルです" },
	{ "initialize sound", "音声の初期化に失敗しました" },
	{ "group", "グループ" },
	{ "error", "エラー" },
	{ "not found", "見つかりません" },
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
