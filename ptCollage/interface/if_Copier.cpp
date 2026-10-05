
#include <pxFile2.h>

#include <pxtnEvelist.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic//if_Generic.h"

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_Copier.h"

#define _COPIER_W  120
#define _COPIER_H  140

#define _BUTTON_CHECKBOX_X 16
#define _BUTTON_CHECKBOX_Y 24
#define _BUTTON_CHECKBOX_W 12
#define _BUTTON_CHECKBOX_H 12

#define _BUTTON_OPEN_X    104
#define _BUTTON_OPEN_Y      0
#define _BUTTON_OPEN_W     16
#define _BUTTON_OPEN_H     16

typedef struct
{
	float x, y;
	int32_t flags;
}
_COPIERSTRUCT;

static _COPIERSTRUCT  _copier      = {0};
static fRECT          _rcPanel     = {0};
static fRECT          _rc_view     = {0};
static const uxSS<14>    _status_name = "copier.status";
static const pxFile2* _ref_file_profile = NULL;


void if_Copier_AllCheck()
{
	for( int32_t i = 0; i < COPIERFLAG_EVENT_NUM; i++ ) _copier.flags |=  ( 0x01 << i );
}

bool if_Copier_SavePosition()
{
	pxDescriptor* desc = NULL;
	if( !_ref_file_profile->open_w( &desc, _status_name, NULL, NULL ) ) return false;
	if( !desc->w_asfile( &_copier, sizeof(_copier), 1 ) ){ SAFE_DELETE( desc ); return false; }
	SAFE_DELETE( desc );
	return true;
}

// 座標
void if_Copier_SetPosition( float x,   float y   )
{
	_copier.x = x;
	_copier.y = y;
	if( _copier.x <                          8 ) _copier.x =                          8;
	if( _copier.y <                          8 ) _copier.y =                          8;
	if( _copier.x > _rc_view.r - _COPIER_W - 8 ) _copier.x = _rc_view.r - _COPIER_W - 8;
	if( _copier.y > _rc_view.b - _COPIER_H - 8 ) _copier.y = _rc_view.b - _COPIER_H - 8;
}

void if_Copier_GetPosition( float *px, float *py ){ *px = _copier.x; *py = _copier.y; }

// 領域
void if_Copier_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	if( _copier.x <                          8 ) _copier.x =                         8;
	if( _copier.y <                          8 ) _copier.y =                         8;
	if( _copier.x > rc_view->r - _COPIER_W - 8 ) _copier.x =rc_view->r - _COPIER_W - 8;
	if( _copier.y > rc_view->b - _COPIER_H - 8 ) _copier.y =rc_view->b - _COPIER_H - 8;

	_rcPanel.l = _copier.x;
	_rcPanel.t = _copier.y;
	_rcPanel.r = _copier.x + _COPIER_W;
	_rcPanel.b = _copier.y + _COPIER_H;
}

fRECT* if_Copier_GetRect()
{
	return &_rcPanel;
}

bool if_Copier_CheckButton( float x, float y, bool* p_bON )
{
	// ボリュームチェック
	if( !( _copier.flags & COPIERFLAG_STATUS_OPEN ) ) return false;

	for( int32_t i = 0; i < COPIERFLAG_EVENT_NUM; i++ )
	{
		if( x >= _copier.x + _BUTTON_CHECKBOX_X &&
			x <  _copier.x + _BUTTON_CHECKBOX_X + _BUTTON_CHECKBOX_W &&
			y >= _copier.y + _BUTTON_CHECKBOX_Y + i * _BUTTON_CHECKBOX_H                      &&
			y <  _copier.y + _BUTTON_CHECKBOX_Y + i * _BUTTON_CHECKBOX_H + _BUTTON_CHECKBOX_H )
		{
			if( _copier.flags & ( 0x01 << i ) ){ _copier.flags &= ~( 0x01 << i ); *p_bON = false; }
			else                               { _copier.flags |=  ( 0x01 << i ); *p_bON = true ; }
			return true;
		}
	}
	return false;
}

bool if_Copier_CheckButton( float x, float y, bool bON )
{
	// ボリュームチェック
	if( !( _copier.flags & COPIERFLAG_STATUS_OPEN ) ) return false;

	for( int32_t i = 0; i < COPIERFLAG_EVENT_NUM; i++ )
	{
		if( x >= _copier.x + _BUTTON_CHECKBOX_X &&
			x <  _copier.x + _BUTTON_CHECKBOX_X + _BUTTON_CHECKBOX_W &&
			y >= _copier.y + _BUTTON_CHECKBOX_Y + i * _BUTTON_CHECKBOX_H                      &&
			y <  _copier.y + _BUTTON_CHECKBOX_Y + i * _BUTTON_CHECKBOX_H + _BUTTON_CHECKBOX_H )
		{
			if( !bON ) _copier.flags &= ~( 0x01 << i );
			else       _copier.flags |=  ( 0x01 << i );
			return true;
		}
	}
	return false;
}


bool if_Copier_ShutButton( float x, float y )
{
	if( !( _copier.flags & COPIERFLAG_STATUS_OPEN ) ) return false;

	// オープンクローズ
	if( x >= _copier.x + _BUTTON_OPEN_X                  &&
		x <  _copier.x + _BUTTON_OPEN_X + _BUTTON_OPEN_W &&
		y >= _copier.y + _BUTTON_OPEN_Y                  &&
		y <  _copier.y + _BUTTON_OPEN_Y + _BUTTON_OPEN_H )
	{
		_copier.flags &= ~COPIERFLAG_STATUS_OPEN;
		return true;
	}
	return false;
}



int32_t if_Copier_GetFlags()
{
	return _copier.flags;
}

void if_Copier_SetFlags( int32_t flags )
{
	_copier.flags = flags;
}

bool if_Copier_IsOpen()
{
	if( _copier.flags & COPIERFLAG_STATUS_OPEN ) return true;
	return false;
}

void if_Copier_Put()
{
	if( !( _copier.flags & COPIERFLAG_STATUS_OPEN ) ) return;

	fRECT rc_frame = {256,256,376,396};
	fRECT rcCheck  = {376,256,376+12,256+12};

	g_dxdraw->tex_Put_View( _copier.x, _copier.y, &rc_frame, SURF_DIALOGS );

	for( int32_t i = 0; i < COPIERFLAG_EVENT_NUM; i++ )
	{
		if( _copier.flags & ( 0x01 << i ) )
		{
			g_dxdraw->tex_Put_View(
				_copier.x + _BUTTON_CHECKBOX_X,
				_copier.y + _BUTTON_CHECKBOX_Y + i * _BUTTON_CHECKBOX_H,
				&rcCheck, SURF_DIALOGS );
		}
	}

#ifdef _DEBUG
	if_gen_num6( _copier.x, _copier.y    , (int32_t)_copier.x, 3 );
	if_gen_num6( _copier.x, _copier.y + 8, (int32_t)_copier.y, 3 );
#endif

}

static bool _EventKinds[ EVENTKIND_NUM ];

bool *if_Copier_GetEventKinds()
{
	memset( _EventKinds, 0, sizeof(bool) * EVENTKIND_NUM );

	if( _copier.flags & COPIERFLAG_EVENT_ON        )
	{
		_EventKinds[ EVENTKIND_ON       ] = true;
		_EventKinds[ EVENTKIND_VELOCITY ] = true;
	}
	if( _copier.flags & COPIERFLAG_EVENT_KEY        ) _EventKinds[ EVENTKIND_KEY        ] = true;
	if( _copier.flags & COPIERFLAG_EVENT_PAN_TIME   ) _EventKinds[ EVENTKIND_PAN_TIME   ] = true;
	if( _copier.flags & COPIERFLAG_EVENT_PAN_VOLUME ) _EventKinds[ EVENTKIND_PAN_VOLUME ] = true;
	if( _copier.flags & COPIERFLAG_EVENT_VOLUME     ) _EventKinds[ EVENTKIND_VOLUME     ] = true;
	if( _copier.flags & COPIERFLAG_EVENT_PORTAMENT  ) _EventKinds[ EVENTKIND_PORTAMENT  ] = true;
	if( _copier.flags & COPIERFLAG_EVENT_VOICENO    ) _EventKinds[ EVENTKIND_VOICENO    ] = true;
	if( _copier.flags & COPIERFLAG_EVENT_GROUPNO    ) _EventKinds[ EVENTKIND_GROUPNO    ] = true;
	if( _copier.flags & COPIERFLAG_EVENT_TUNING     ) _EventKinds[ EVENTKIND_TUNING     ] = true;
	return _EventKinds;
}



#include "if_SubWindows.h"

static SUBWINDOWSTRUCT* _handle = NULL;
bool if_Cursor_Action_Copier( float cur_x, float cur_y );

void if_Copier_init( HWND hWnd, const pxFile2* file_profile )
{
	pxDescriptor* desc = NULL;
	_ref_file_profile = file_profile;

	if( !_ref_file_profile->open_r( &desc, _status_name, NULL, NULL ) ||
		!desc->r( &_copier, sizeof(_copier), 1 ) )
	{
		_copier.x  =   496;
		_copier.y  =   240;
	}
	SAFE_DELETE( desc );
	if_Copier_AllCheck();

	_handle = if_SubWindows_Create( if_Copier_Put, if_Cursor_Action_Copier );
}


void if_Copier_SetFocus()
{
	if_SubWindows_SetFocus( _handle );
}

void if_Copier_SetOpen()
{
	if( _copier.flags & COPIERFLAG_STATUS_OPEN ){ _copier.flags &= ~COPIERFLAG_STATUS_OPEN;                       }
	else                                        { _copier.flags |=  COPIERFLAG_STATUS_OPEN; if_Copier_SetFocus(); }
}
