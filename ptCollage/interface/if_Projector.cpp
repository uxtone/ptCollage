
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_Generic.h"

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_Projector.h"

#define PROJECTORFLAG_STATUS_OPEN     0x1000

#define _PROJECTOR_W    208
#define _PROJECTOR_H    116

#define _BUTTON_CLOSE_X 192
#define _BUTTON_CLOSE_Y   0
#define _BUTTON_CLOSE_W  16
#define _BUTTON_CLOSE_H  16

#define _BUTTON_EDIT_X  176
#define _BUTTON_EDIT_Y   88
#define _BUTTON_EDIT_W   16
#define _BUTTON_EDIT_H   16

#define _PROJECTNAME_X   74
#define _PROJECTNAME_Y   24
#define _BEATNUM_X       68
#define _BEATNUM_Y       52
#define _TEMPO_X         68
#define _TEMPO_Y         72
#define _MEASNUM_X       68
#define _MEASNUM_Y       92

typedef struct
{
	float   x, y;
	int32_t flags;
}
_PROJECTORSTRUCT;

static _PROJECTORSTRUCT _projector = {0};
static fRECT            _rcPanel   = {0};
static fRECT            _rc_view   = {0};

static const uxSS<17>      _status_name  = "projector.status";
static const char *     _default_name =    "-PXTONE COLLAGE-" ;
static const pxFile2*   _ref_file_profile = NULL;

void if_Projector_RedrawName( const uxDS& new_font_name )
{
	const char* p_name    = g_pxtn->text->get_name_buf( NULL );

	if( !p_name ) p_name  = _default_name;

	if( new_font_name ) g_dxdraw->tex_glyph_another_font( SURF_PROJECTNAME, new_font_name, 12, false );

	{
		fRECT rc = {3,3,PROJECTNAME_WIDTH, PROJECTNAME_HEIGHT};
		g_dxdraw->tex_glyph_clear( SURF_PROJECTNAME );
		g_dxdraw->tex_Clear      ( SURF_PROJECTNAME );
		g_dxdraw->tex_glyph_text ( SURF_PROJECTNAME, p_name, &rc, 1 );
	}
}


bool if_Projector_SavePosition()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _status_name, NULL, NULL ) ) goto term;
	if( !desc->w_asfile( &_projector     , sizeof(_projector),    1 ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}

void if_Projector_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	if( _projector.x < 8 ) _projector.x = 8;
	if( _projector.y < 8 ) _projector.y = 8;
	if( _projector.x > rc_view->r - _PROJECTOR_W - 8 ) _projector.x = rc_view->r - _PROJECTOR_W - 8;
	if( _projector.y > rc_view->b - _PROJECTOR_H - 8 ) _projector.y = rc_view->b - _PROJECTOR_H - 8;

	_rcPanel.l = _projector.x;
	_rcPanel.t = _projector.y;
	_rcPanel.r = _projector.x + _PROJECTOR_W;
	_rcPanel.b = _projector.y + _PROJECTOR_H;
}

// 座標
void if_Projector_SetPosition( float x, float y )
{
	_projector.x = x;
	_projector.y = y;
	if( _projector.x < 8 ) _projector.x = 8;
	if( _projector.y < 8 ) _projector.y = 8;
	if( _projector.x > _rc_view.r - _PROJECTOR_W - 8 ) _projector.x = _rc_view.r - _PROJECTOR_W - 8;
	if( _projector.y > _rc_view.b - _PROJECTOR_H - 8 ) _projector.y = _rc_view.b - _PROJECTOR_H - 8;
}

void if_Projector_GetPosition( float *px, float *py ){ *px = _projector.x; *py = _projector.y; }

fRECT *if_Projector_GetRect()
{
	return &_rcPanel;
}

bool if_Projector_ShutButton( float x, float y )
{
	if( !( _projector.flags & PROJECTORFLAG_STATUS_OPEN ) ) return false;

	// オープンクローズ
	if( x >= _projector.x + _BUTTON_CLOSE_X                   &&
		x <  _projector.x + _BUTTON_CLOSE_X + _BUTTON_CLOSE_W &&
		y >= _projector.y + _BUTTON_CLOSE_Y                   &&
		y <  _projector.y + _BUTTON_CLOSE_Y + _BUTTON_CLOSE_H )
	{
		_projector.flags &= ~PROJECTORFLAG_STATUS_OPEN;
		return true;
	}
	return false;
}

void pxtoneProject_IDM_EDITPROJECT();

bool if_Projector_HitButton( float cur_x, float cur_y )
{
	if( !( _projector.flags & PROJECTORFLAG_STATUS_OPEN ) ) return false;

	// 表示倍率変更
	if( cur_x > _rcPanel.l + _BUTTON_EDIT_X && cur_x < _rcPanel.l + _BUTTON_EDIT_X + _BUTTON_EDIT_W &&
		cur_y > _rcPanel.t + _BUTTON_EDIT_Y && cur_y < _rcPanel.t + _BUTTON_EDIT_Y + _BUTTON_EDIT_H )
	{
		pxtoneProject_IDM_EDITPROJECT();
		KeyControl_Clear();
		return true;
	}
	return false;
}


int32_t if_Projector_GetFlags()
{
	return _projector.flags;
}

void if_Projector_SetFlags( int32_t flags )
{
	_projector.flags = flags;
}

bool if_Projector_IsOpen()
{
	if( _projector.flags & PROJECTORFLAG_STATUS_OPEN ) return true;
	return false;
}


void if_Projector_Put()
{
	if( !( _projector.flags & PROJECTORFLAG_STATUS_OPEN ) ) return;

	fRECT rcFrame = {  0,384,208,500};

	g_dxdraw->tex_Put_View( _projector.x, _projector.y, &rcFrame, SURF_DIALOGS );

	int     beat_num  ;
	float   beat_tempo;
	int32_t meas_num  ;
	fRECT  rcName = {0,0,PROJECTNAME_WIDTH, PROJECTNAME_HEIGHT};

	g_pxtn->master->Get( &beat_num, &beat_tempo, NULL, &meas_num );

	g_dxdraw->tex_Put_View( _rcPanel.l + _PROJECTNAME_X, _rcPanel.t + _PROJECTNAME_Y, &rcName, SURF_PROJECTNAME );

	if_gen_num6_clip( _rcPanel.l + _TEMPO_X,       _rcPanel.t + _TEMPO_Y,   (int32_t)beat_tempo, 4, &_rcPanel );
	if_gen_num6_clip( _rcPanel.l + _BEATNUM_X,     _rcPanel.t + _BEATNUM_Y, beat_num,        4, &_rcPanel );
	if_gen_num6_clip( _rcPanel.l + _MEASNUM_X,     _rcPanel.t + _MEASNUM_Y, meas_num,        4, &_rcPanel );

#ifdef _DEBUG
	if_gen_num6( _projector.x, _projector.y    , (int32_t)_projector.x, 3 );
	if_gen_num6( _projector.x, _projector.y + 8, (int32_t)_projector.y, 3 );
#endif

}

void if_Projector_SetOpen()
{
	if( _projector.flags & PROJECTORFLAG_STATUS_OPEN ){ _projector.flags &= ~PROJECTORFLAG_STATUS_OPEN;                          }
	else                                              { _projector.flags |=  PROJECTORFLAG_STATUS_OPEN; if_Projector_SetFocus(); }
}


#include "if_SubWindows.h"

static SUBWINDOWSTRUCT* _handle = NULL;
bool if_Cursor_Action_Projector( float cur_x, float cur_y );

void if_Projector_init( HWND hWnd, const pxFile2* file_profile )
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	_ref_file_profile = file_profile;

	if( !_ref_file_profile->open_r( &desc, _status_name, NULL, NULL ) ||
		!desc->r( &_projector, sizeof(_projector), 1 ) )
	{
		_projector.x = 252;
		_projector.y = 240;
	}
	SAFE_DELETE( desc );

	_handle = if_SubWindows_Create( if_Projector_Put, if_Cursor_Action_Projector );
}

void if_Projector_SetFocus()
{
	if_SubWindows_SetFocus( _handle );
}
