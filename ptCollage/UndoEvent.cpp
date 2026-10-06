


#include <pxwAlteration.h>
extern pxwAlteration* g_alte;

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include "UndoEvent.h"


#define MAX_UNDOEVENT 100


// アンドゥ登録構造体
typedef struct
{
	bool* b_units;
	int32_t   clock1  ;
	int32_t   clock2  ;

}UNDOEVENTORDER;

typedef struct
{
	int32_t           clock  ;
	unsigned char unit_no;
	unsigned char kind   ;
	int32_t           value  ;

}UNDOEVENTDATA;

typedef struct
{

	bool          bValid;
	bool*         b_units;
	int32_t           clock1;
	int32_t           clock2;
	int32_t           data_num;
	UNDOEVENTDATA *pData;

}UNDOEVENTLIST;

UNDOEVENTORDER _UndoOrder;
UNDOEVENTLIST _UndoList[ MAX_UNDOEVENT ];
UNDOEVENTLIST _RedoList[ MAX_UNDOEVENT ];

static int32_t _max_unit = 0;

static int32_t           _undo_index    = 0;
static uint32_t           _undo_data_num = 0;
static bool          _bUndoZero     = true;

static int32_t           _redo_index    = 0;
static uint32_t           _redo_data_num = 0;
static bool          _bRedoZero     = true;



///////////////////////////////////////////////
// 以下の時にアンドゥはクリアされる
// ・プロジェクトが初期化されたりロードされた時
// ・ユニット・音源が削除された時
///////////////////////////////////////////////

// １ユニットの範囲内イベントを登録
static int32_t _AddUnitEventUndo( UNDOEVENTDATA *pUndoData, int32_t clock1, int32_t clock2, int32_t u, int32_t count )
{
	const EVERECORD* p = g_pxtn->evels->get_Records();

	for( ; p; p = p->next )
	{
		if( p->unit_no == u )
		{
			if( p->clock >= clock1 ) break;
			if( Evelist_Kind_IsTail( p->     kind ) && p->clock + p->     value > clock1 ) break;
		}
	}

	for( ; p; p = p->next )
	{
		if( p->clock != clock1 && p->clock >= clock2 ) break;
		if( p->unit_no == u )
		{
			pUndoData[ count ].clock   = p->clock  ;
			pUndoData[ count ].unit_no = (unsigned char)u;
			pUndoData[ count ].kind    = (unsigned char)p->kind ;
			pUndoData[ count ].value   = p->value;
			count++;
		}
	}

	return count;
}


///////////////////////////////////////
// 以下はグローバル ///////////////////
///////////////////////////////////////

// 初期化
bool UndoEvent_Initialize( int32_t max_unit )
{
	bool b_ret = false;
	int32_t  u;

	_max_unit = max_unit;

	memset( &_UndoOrder, 0, sizeof(UNDOEVENTORDER)                );
	memset( _UndoList,   0, sizeof(UNDOEVENTLIST) * MAX_UNDOEVENT );
	memset( _RedoList,   0, sizeof(UNDOEVENTLIST) * MAX_UNDOEVENT );

	// メモリの確保
	_UndoOrder.b_units = (bool *)malloc( sizeof(bool) * _max_unit );
	if( !_UndoOrder.b_units ) goto End;
	memset( _UndoOrder.b_units, false, sizeof(bool) * _max_unit );

	// アンドゥ
	for( u = 0; u < MAX_UNDOEVENT; u++ )
	{
		_UndoList[ u ].b_units = (bool *)malloc( sizeof(bool) * _max_unit );
		if( !_UndoList[ u ].b_units ) goto End;
		memset( _UndoList[ u ].b_units, false, sizeof(bool) * _max_unit );
	}
	_undo_index    = 0;
	_undo_data_num = 0;
	_bUndoZero     = true;

	// リドゥ
	for( u = 0; u < MAX_UNDOEVENT; u++ )
	{
		_RedoList[ u ].b_units = (bool *)malloc( sizeof(bool) * _max_unit );
		if( !_RedoList[ u ].b_units ) goto End;
		memset( _RedoList[ u ].b_units, false, sizeof(bool) * _max_unit );
	}
	_redo_index    = 0;
	_redo_data_num = 0;
	_bRedoZero     = true;

	b_ret = true;
End:
	return b_ret;
}


// 開放
void UndoEvent_ReleaseUndoRedo()
{
	int32_t i;
	UNDOEVENTLIST *p_undo;

	// アンドゥ
	for( i = 0; i < MAX_UNDOEVENT; i++ )
	{
		p_undo = &_UndoList[ i ];
		if( p_undo->bValid && p_undo->pData )
		{
			free( p_undo->pData );
			p_undo->pData = NULL;
		}
		memset( p_undo->b_units, false, sizeof(bool) * _max_unit );
		p_undo->bValid = false;
	}
	_undo_index    = 0;
	_undo_data_num = 0;
	_bUndoZero     = true;

	// リドゥ
	for( i = 0; i < MAX_UNDOEVENT; i++ )
	{
		p_undo = &_RedoList[i];
		if( p_undo->bValid && p_undo->pData )
		{
			free( p_undo->pData );
			p_undo->pData = NULL;
		}
		memset( p_undo->b_units, false, sizeof(bool) * _max_unit );
		p_undo->bValid = false;
	}
	_redo_index    = 0;
	_redo_data_num = 0;
	_bRedoZero     = true;

}

void UndoEvent_ReleaseRedo()
{
	int32_t i;
	UNDOEVENTLIST *p_undo;

	for( i = 0; i < MAX_UNDOEVENT; i++ )
	{
		p_undo = &_RedoList[i];
		if( p_undo->bValid && p_undo->pData )
		{
			free( p_undo->pData );
			p_undo->pData = NULL;
		}
		memset( p_undo->b_units, false, sizeof(bool) * _max_unit );
		p_undo->bValid = false;
	}

	_redo_index    = 0;
	_redo_data_num = 0;
	_bRedoZero     = true;
}

void UndoEvent_Release()
{
	UndoEvent_ReleaseUndoRedo();

	if( _UndoOrder.b_units )
	{
		free( _UndoOrder.b_units );
		_UndoOrder.b_units = NULL;
	}
}

void UndoEvent_SetOrderUnit( int32_t u )
{
	if( u >= 0 && u < _max_unit ) _UndoOrder.b_units[ u ] = true;
	else MessageBox( NULL, uxT( "217394080" ), uxT( "error" ), MB_OK );
}

void UndoEvent_SetOrderClock( int32_t clock1, int32_t clock2 )
{
	memset( _UndoOrder.b_units, false, sizeof(bool) * _max_unit );
	_UndoOrder.clock1 = clock1;
	_UndoOrder.clock2 = clock2;
}

// アンドゥプッシュ
bool UndoEvent_Push( bool bUndo )
{
	uint32_t event_num;
	int32_t un       ;

	UNDOEVENTLIST  *pUndoList;

	if( bUndo )
	{
		un = _undo_index + 1;
		if( un >= MAX_UNDOEVENT ) un = 0;
		pUndoList = &_UndoList[ un ];
	}
	else
	{
		un = _redo_index + 1;
		if( un >= MAX_UNDOEVENT ) un = 0;
		pUndoList = &_RedoList[ un ];
	}

	// 使用中なら開放 ------------------
	if( pUndoList->bValid ){

		_undo_data_num -= pUndoList->data_num;
		if( pUndoList->pData ) free( pUndoList->pData ); pUndoList->pData = NULL;
		memset( pUndoList->b_units, false, sizeof(bool) * _max_unit );
	}


	// 該当イベントの数を数える --------
	event_num = 0;
	for( int32_t u = 0; u < _max_unit; u++ )
	{
		if( g_pxtn->Unit_Get( u ) && _UndoOrder.b_units[ u ] )
		{
			event_num += g_pxtn->evels->get_Count( _UndoOrder.clock1, _UndoOrder.clock2, u );
		}
	}

	// 領域を確保 ----------------------
	if( event_num )
	{
		pUndoList->pData = (UNDOEVENTDATA *)malloc( sizeof(UNDOEVENTDATA) * event_num );
		if( !pUndoList->pData ) return false;

		// 該当イベントを記録 --------------
		int32_t count = 0;
		for( int32_t u = 0; u < _max_unit; u++ )
		{
			if( g_pxtn->Unit_Get( u ) && _UndoOrder.b_units[ u ] )
			{
				count = _AddUnitEventUndo( pUndoList->pData, _UndoOrder.clock1, _UndoOrder.clock2, u, count );
			}
		}
	}

	pUndoList->bValid   = true;
	pUndoList->clock1   = _UndoOrder.clock1;
	pUndoList->clock2   = _UndoOrder.clock2;
	pUndoList->data_num = event_num;
	for( int32_t u = 0; u < _max_unit; u++ )
	{
		if( g_pxtn->Unit_Get( u ) && _UndoOrder.b_units[ u ] ) pUndoList->b_units[ u ] = true;
	}

	if( bUndo )
	{
		_undo_data_num += event_num;
		_undo_index    =  un;
		_bUndoZero     =  false;
	}
	else
	{
		_redo_data_num += event_num;
		_redo_index    =  un;
		_bRedoZero     =  false;
	}

	g_alte->set();

	return true;
}


// アンドゥ１回取り消し
void UndoEvent_Back()
{

	if( _bUndoZero                     ) return;
	if( !_UndoList[_undo_index].bValid ) return;

	// リリース
	_undo_data_num -= _UndoList[ _undo_index ].data_num;
	if( _UndoList[ _undo_index ].pData ) free( _UndoList[_undo_index].pData );
	_UndoList[ _undo_index ].pData = NULL;
	memset( _UndoList[ _undo_index ].b_units, false, sizeof(bool) * _max_unit );

	// インデックス移動
	_undo_index -= 1;
	if( _undo_index < 0 ) _undo_index = MAX_UNDOEVENT - 1;

	// undo の残量
	if( !_UndoList[_undo_index].bValid ) _bUndoZero = true;
}


// アンドゥポップ
bool UndoEvent_Pop( bool bUndo )
{
	UNDOEVENTLIST  *pUndoList;

	if( bUndo )
	{
		if( _bUndoZero ) return true;
		pUndoList = &_UndoList[ _undo_index ];
	}
	else
	{
		if( _bRedoZero ) return true;
		pUndoList = &_RedoList[ _redo_index ];
	}

	if( !pUndoList->bValid ) return false;


	// アンドゥのアンドゥ ====================================================
	UndoEvent_SetOrderClock( pUndoList->clock1, pUndoList->clock2 );

	for( int u = 0; u < _max_unit; u++ )
	{
		if( g_pxtn->Unit_Get( u ) && pUndoList->b_units[ u ] )
			UndoEvent_SetOrderUnit( u );
	}
	// アンドゥの場合リドゥを追加
	if( bUndo )
	{
		if( !UndoEvent_Push( false ) ) return false;
	}
	// リドゥの場合アンドゥを追加
	else
	{
		if( !UndoEvent_Push( true ) ) return false;
	}
	// =======================================================================

	for( int32_t u = 0; u < _max_unit; u++ )
	{
		if( g_pxtn->Unit_Get( u ) && pUndoList->b_units[ u ] )
			g_pxtn->evels->Record_Delete( pUndoList->clock1, pUndoList->clock2, u );
	}

	// 範囲のイベントを配置
	int32_t  clock  ;
	int32_t  unit_no;
	int32_t  kind   ;
	int32_t  value  ;
	int32_t  d;

	if( pUndoList->pData )
	{
		for( d = 0; d < pUndoList->data_num; d++ )
		{
			clock      = pUndoList->pData[ d ].clock  ;
			unit_no    = pUndoList->pData[ d ].unit_no;
			kind       = pUndoList->pData[ d ].kind   ;
			value      = pUndoList->pData[ d ].value  ;

			if( !g_pxtn->Unit_Get( unit_no ) ) return false;
			g_pxtn->evels->Record_Add_i( clock, (unsigned char)unit_no, (unsigned char)kind, value );
		}
	}

	// リリース
	if( bUndo ) _undo_data_num -= pUndoList->data_num;
	else        _redo_data_num -= pUndoList->data_num;
	if( pUndoList->pData ) free( pUndoList->pData );
	pUndoList->pData = NULL;
	memset( pUndoList->b_units, false, sizeof(bool) * _max_unit );
	pUndoList->bValid = false;

	// インデックス移動

	// undo の残量
	if( bUndo )
	{
		_undo_index -= 1;
		if( _undo_index < 0                ) _undo_index = MAX_UNDOEVENT - 1;
		if( !_UndoList[_undo_index].bValid ) _bUndoZero = true;
	}
	else
	{
		_redo_index -= 1;
		if( _redo_index < 0                ) _redo_index = MAX_UNDOEVENT - 1;
		if( !_RedoList[_redo_index].bValid ) _bRedoZero = true;
	}

	g_alte->set();

	return true;
}






static void _Replace_UnitNo( UNDOEVENTLIST* list, int32_t old_pos, int32_t new_pos )
{
	UNDOEVENTLIST* p_undo = list;
	UNDOEVENTDATA* p_data;

	for( int32_t u = 0; u < MAX_UNDOEVENT; u++, p_undo++ )
	{
		if( p_undo->bValid )
		{
			// b_units
			bool b_old = p_undo->b_units[ old_pos ];
			if( old_pos < new_pos )
			{
				for( int32_t b = old_pos; b < new_pos; b++ ) p_undo->b_units[ b ] = p_undo->b_units[ b + 1 ];
			}
			else
			{
				for( int32_t b = old_pos; b > new_pos; b-- ) p_undo->b_units[ b ] = p_undo->b_units[ b - 1 ];
			}
			p_undo->b_units[ new_pos ] = b_old;

			// unit_no
			if( old_pos < new_pos )
			{
				p_data = p_undo->pData;
				for( int32_t d = 0; d < p_undo->data_num; d++, p_data++ )
				{
					if(      p_data->unit_no == old_pos                               ) p_data->unit_no = (unsigned char)new_pos;
					else if( p_data->unit_no >  old_pos && p_data->unit_no <= new_pos ) p_data->unit_no--;
				}
			}
			else
			{
				p_data = p_undo->pData;
				for( int32_t d = 0; d < p_undo->data_num; d++, p_data++ )
				{
					if(      p_data->unit_no == old_pos                               ) p_data->unit_no = (unsigned char)new_pos;
					else if( p_data->unit_no <  old_pos && p_data->unit_no >= new_pos ) p_data->unit_no++;
				}
			}
		}
	}
}

static void _Replace_VoiceNo( UNDOEVENTLIST* list, int32_t old_pos, int32_t new_pos )
{
	UNDOEVENTLIST* p_undo = list;
	UNDOEVENTDATA* p_data;

	for( int32_t u = 0; u < MAX_UNDOEVENT; u++, p_undo++ )
	{
		if( p_undo->bValid )
		{
			if( old_pos < new_pos )
			{
				p_data = p_undo->pData;
				for( int32_t d = 0; d < p_undo->data_num; d++, p_data++ )
				{
					if( p_data->kind == EVENTKIND_VOICENO )
					{
						if(      p_data->value == old_pos                             ) p_data->value = new_pos;
						else if( p_data->value >  old_pos && p_data->value <= new_pos ) p_data->value--;
					}
				}
			}
			else
			{
				p_data = p_undo->pData;
				for( int32_t d = 0; d < p_undo->data_num; d++, p_data++ )
				{
					if( p_data->kind == EVENTKIND_VOICENO )
					{
						if(      p_data->value == old_pos                             ) p_data->value = new_pos;
						else if( p_data->value <  old_pos && p_data->value >= new_pos ) p_data->value++;
					}
				}
			}
		}
	}
}

void UndoEvent_Replace_UnitNo( int32_t old_pos, int32_t new_pos )
{
	if( old_pos == new_pos ) return;
	_Replace_UnitNo( _UndoList, old_pos, new_pos );
	_Replace_UnitNo( _RedoList, old_pos, new_pos );
}

void UndoEvent_Replace_VoiceNo( int32_t old_pos, int32_t new_pos )
{
	if( old_pos == new_pos ) return;
	_Replace_VoiceNo( _UndoList, old_pos, new_pos );
	_Replace_VoiceNo( _RedoList, old_pos, new_pos );
}

// 待機アンドゥはあるか？
bool UndoEvent_IsUndoZero(   void ){ return _bUndoZero;  }
bool UndoEvent_IsRedoZero(   void ){ return _bRedoZero;  }

// アンドゥ位置
int32_t UndoEvent_GetUndoIndex(){ return _undo_index; }
int32_t UndoEvent_GetRedoIndex(){ return _redo_index; }
