
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include "interface/if_ToolPanel.h"

#include "UndoEvent.h"
#include "UnitFocus.h"

static pxtnEvelist *_evels = NULL;

static bool _bFocusOnly_copy;
static int32_t  _clock_width;


void _change( int32_t *p1, int32_t *p2 )
{
	int32_t w;
	w   = *p1;
	*p1 = *p2;
	*p2 = w  ;
}


// 貼り付け処理
static int32_t _Paste( int32_t clock, int32_t u, int32_t kind, int32_t times )
{
	// 書き込み領域を削除
	int32_t count = g_pxtn->evels->Record_Delete( clock, clock + _clock_width * times, (unsigned char)u, (unsigned char)kind );

	// 貼り付け
	for( int t = 0; t < times; t++ )
	{
		for( const EVERECORD* p_eve = _evels->get_Records(); p_eve; p_eve = p_eve->next )
		{
			if( p_eve->unit_no == u && p_eve->kind == kind )
			{
				count += g_pxtn->evels->Record_Add_i( clock + p_eve->clock + _clock_width * t, (unsigned char)u, (unsigned char)kind, p_eve->value );
			}
		}
	}

	return count;
}


///////////////////////////
///////////////////////////

static bool _bUnit[ pxtnMAX_TUNEUNITSTRUCT ];

// 有効なユニットテーブルを準備。対象が変化する Paste では使えない
static void _ReadyEnableUnit()
{
	int32_t u;

	memset( _bUnit, false, sizeof(bool) * pxtnMAX_TUNEUNITSTRUCT );

	// １ユニット
	if( UnitFocus_IsFocusOnly() )
	{
		u = UnitFocus_Get();
		if( u != -1 ) _bUnit[ u ] = true;
	}
	// 複数ユニット
	else
	{
		for( u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
		{
			if( UnitFocus_IsFocusedOrOperated( u ) ) _bUnit[ u ] = true;
		}
	}
}

// 初期化
bool ScopeEvent_Initialize( int32_t max_event_num )
{
	_evels = new pxtnEvelist( NULL, NULL, NULL, NULL );
	if( !_evels->Allocate( max_event_num ) ) return false;
	return true;
}

void ScopeEvent_Release()
{
	SAFE_DELETE( _evels );
}

// コピー処理
int32_t ScopeEvent_Copy( int32_t clock1, int32_t clock2, bool* p_bEventKinds )
{
	int32_t  count = 0;
	int32_t  c;
	int32_t  v;
	bool bAdd;

	if( clock1 > clock2 ) _change( &clock1, &clock2 );

	// 作業領域をクリア
	_evels->Clear();

	_ReadyEnableUnit();

	const EVERECORD *p_eve = g_pxtn->evels->get_Records();

	for( ; p_eve; p_eve = p_eve->next )
	{
		if( p_eve->clock >= clock1 ) break;
	}

	for( ; p_eve; p_eve = p_eve->next )
	{
		if( p_eve->clock >= clock2 ) break;

		bAdd = false;
		if( _bUnit[ p_eve->unit_no ] )
		{
			if(     !p_bEventKinds                ) bAdd = true;
			else if( p_bEventKinds[ p_eve->kind ] ) bAdd = true;
		}
		if( bAdd )
		{
			c  = p_eve->clock - clock1;
			if( Evelist_Kind_IsTail( p_eve->kind ) && p_eve->value > clock2 - p_eve->clock ) v = clock2 - p_eve->clock;
			else                                                                             v =          p_eve->value;
			count += _evels->Record_Add_i( c, p_eve->unit_no, p_eve->kind, v );
		}

	}

	_clock_width     = clock2 - clock1;
	_bFocusOnly_copy = UnitFocus_IsFocusOnly();

	return count;
}


// 消去処理
int32_t ScopeEvent_Clear( int32_t clock1, int32_t clock2 )
{
	int32_t           count = 0;

	if( clock1 > clock2 ) _change( &clock1, &clock2 );

	// アンドゥ
	UndoEvent_SetOrderClock( clock1, clock2 );
	_ReadyEnableUnit();
	for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
	{
		if( _bUnit[ u ] ) UndoEvent_SetOrderUnit( u );
	}
	UndoEvent_Push( true );

	for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
	{
		if( _bUnit[ u ] ) count += g_pxtn->evels->Record_Delete( clock1, clock2, (unsigned char)u );
	}
	if( !count ) UndoEvent_Back();
	else         UndoEvent_ReleaseRedo();

	return count;
}

// 削除処理
int32_t ScopeEvent_Delete( int32_t clock1, int32_t clock2 )
{
	int32_t           count = 0;

	if( clock1 > clock2 ) _change( &clock1, &clock2 );

	// アンドゥ
	UndoEvent_SetOrderClock( clock1, g_pxtn->evels->get_Max_Clock() );
	_ReadyEnableUnit();
	for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
	{
		if( _bUnit[ u ] ) UndoEvent_SetOrderUnit( u );
	}
	UndoEvent_Push( true );

	for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
	{
		if( _bUnit[ u ] )
		{
			count += g_pxtn->evels->Record_Delete     ( clock1, clock2,                 (unsigned char)u );
			count += g_pxtn->evels->Record_Clock_Shift( clock2, (clock2 - clock1) * -1, (unsigned char)u );
		}
	}
	if( !count ) UndoEvent_Back();
	else         UndoEvent_ReleaseRedo();

	return count;
}

int32_t ScopeEvent_Cut( int32_t clock1, int32_t clock2 )
{
	ScopeEvent_Copy(          clock1, clock2, NULL );
	return ScopeEvent_Delete( clock1, clock2 );
}

// 変更処理
int32_t ScopeEvent_ChangeVolume( int32_t clock1, int32_t clock2, int32_t kind, int32_t value )
{
	int32_t count = 0;

	if( clock1 > clock2 ) _change( &clock1, &clock2 );

	// アンドゥ
	UndoEvent_SetOrderClock( clock1, clock2 );
	_ReadyEnableUnit();
	for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
	{
		if( _bUnit[ u ] ) UndoEvent_SetOrderUnit( u );
	}
	UndoEvent_Push( true );

	for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
	{
		if( _bUnit[ u ] ) count += g_pxtn->evels->Record_Value_Change( clock1, clock2, (unsigned char)u, (unsigned char)kind, value );
	}
	if( !count ) UndoEvent_Back();
	else         UndoEvent_ReleaseRedo();

	return count;
}


// ペースト処理
int32_t ScopeEvent_Paste( int32_t clock, int32_t times, bool* p_bEventKinds )
{
	int32_t count = 0;

	if( !times    ) return 0;

	// 複数ユニットの場合はユニットモードでなければ中止
	if( !_bFocusOnly_copy && if_ToolPanel_GetMode() != enum_ToolMode_U ) return 0;

	UndoEvent_SetOrderClock( clock, clock + _clock_width * times );

	// 単一ユニット
	if( _bFocusOnly_copy )
	{
		int32_t u = UnitFocus_Get();
		if( u >= 0 )
		{
			UndoEvent_SetOrderUnit( u );
			UndoEvent_Push( true );
			_evels->Record_UnitNo_Set( (unsigned char)u );

			for( int32_t kind = 0; kind < EVENTKIND_NUM; kind++ )
			{
				if( !p_bEventKinds || p_bEventKinds[ kind ] )
				{
					count+= _Paste( clock, u, kind, times );
				}
			}

			if( count )
			{
				UndoEvent_ReleaseRedo();
				g_pxtn->master->AdjustMeasNum( clock + _clock_width * times );
			}
			else
			{
				UndoEvent_Back();
			}
		}

	}
	// 複数ユニット
	else
	{

		for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
		{
			if( UnitFocus_IsFocusedOrOperated( u ) ) UndoEvent_SetOrderUnit( u );
		}
		UndoEvent_Push( true );

		for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
		{
			if( UnitFocus_IsFocusedOrOperated( u ) )
			{
				for( int32_t kind = 0; kind < EVENTKIND_NUM; kind++ )
				{
					if( !p_bEventKinds || p_bEventKinds[ kind ] )
					{
						count+= _Paste( clock, u, kind, times );
					}
				}
			}
		}

		if( count )
		{
			UndoEvent_ReleaseRedo();
			g_pxtn->master->AdjustMeasNum( clock + _clock_width * times );
		}
		else
		{
			UndoEvent_Back();
		}
	}

	return count;
}

// 挿入処理
int32_t ScopeEvent_Insert( int32_t clock, int32_t times )
{
	int32_t count = 0;

	// 複数ユニットの場合はユニットモードでなければ中止
	if( !_bFocusOnly_copy && if_ToolPanel_GetMode() != enum_ToolMode_U ) return 0;

	UndoEvent_SetOrderClock( clock, g_pxtn->evels->get_Max_Clock() + _clock_width * times );

	// 単一ユニット
	if( _bFocusOnly_copy )
	{
		int32_t u;
		if( ( u = UnitFocus_Get() ) >= 0 )
		{
			_evels->Record_UnitNo_Set( (unsigned char)u );
			UndoEvent_SetOrderUnit( u );
			UndoEvent_Push( true );

			count += g_pxtn->evels->Record_Clock_Shift( clock, _clock_width * times, (unsigned char)u );
			for( int32_t kind = 0; kind < EVENTKIND_NUM; kind++ ) count += _Paste( clock, u, kind, times );

			if( count )
			{
				UndoEvent_ReleaseRedo();
				g_pxtn->AdjustMeasNum();
			}
			else
			{
				UndoEvent_Back();
			}
		}
	}
	// 複数ユニット
	else
	{
		for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
		{
			if( UnitFocus_IsFocusedOrOperated( u ) ) UndoEvent_SetOrderUnit( u );
		}
		UndoEvent_Push( true );

		for( int32_t u = 0; u < pxtnMAX_TUNEUNITSTRUCT; u++ )
		{
			if( UnitFocus_IsFocusedOrOperated( u ) )
			{
				count += g_pxtn->evels->Record_Clock_Shift( clock, _clock_width * times, (unsigned char)u );
				for( int32_t kind = 0; kind < EVENTKIND_NUM; kind++ ) count += _Paste( clock, u, kind, times );
			}
		}

		if( count )
		{
			UndoEvent_ReleaseRedo();
			g_pxtn->AdjustMeasNum();
		}
		else
		{
			UndoEvent_Back();
		}
	}

	return count;
}


