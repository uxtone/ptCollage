

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_Generic.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_Panel_Scale.h"
#include "if_BaseField.h"

#define _BUTTON_ZOOM_X  80
#define _BUTTON_ZOOM_Y   4
#define _BUTTON_DIVI_X 192
#define _BUTTON_DIVI_Y   4
#define _BUTTON_W       16
#define _BUTTON_H       16

static const TCHAR* _file_name = _T("scales.bin");
static fRECT        _rcPanel   = {0};

#define _DIVI_NUM 10
#define _ZOOM_NUM  4

static const int _divi_table[ _DIVI_NUM ] = {1,2,3,4,6,8,12,16,24,48};
static const int _zoom_table[ _ZOOM_NUM ] =
{
	DEFAULT_BEATWIDTH/2,
	DEFAULT_BEATWIDTH*1,
	DEFAULT_BEATWIDTH*2,
	DEFAULT_BEATWIDTH*4,
};

static       int      _divi_index = 3;
static       int      _zoom_index = 1;
static const pxFile2* _ref_file_profile = NULL;


void    if_Panel_init( const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;
}

void if_Panel_Scale_Load()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_r( &desc, _file_name, NULL, NULL ) ) goto term;
	if( !desc->r( &_divi_index, sizeof(int), 1 )  ) goto term;
	if( !desc->r( &_zoom_index, sizeof(int), 1 )  ) goto term;

term:
	SAFE_DELETE( desc );

	if( _divi_index < 0 || _divi_index >= _DIVI_NUM ) _divi_index = 3;
	if( _zoom_index < 0 || _zoom_index >= _ZOOM_NUM ) _zoom_index = 1;

	if_BaseField_SetParameter( _zoom_table[ _zoom_index ], _divi_table[ _divi_index ] );
}

bool if_Panel_Scale_Save()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	// was "if( open_w(..) )" with no braces: only the first write was guarded (see if_Player_SavePosition).
	if( !_ref_file_profile->open_w( &desc, _file_name, NULL, NULL ) ) goto term;
	if( !desc->w_asfile( &_divi_index, sizeof(int), 1 )  ) goto term;
	if( !desc->w_asfile( &_zoom_index, sizeof(int), 1 )  ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );

	return b_ret;
}

void if_Panel_Scale_SetRect( const fRECT *rc_view )
{
	float x = rc_view->r - PANEL_RACK_W;
	if( x < 0 ) x = 0;
	_rcPanel.l = x          + PANEL_COMMENT_W;
	_rcPanel.t = rc_view->t + PANEL_RACK_H   ;
	_rcPanel.r = _rcPanel.l + PANEL_SCALE_W  ;
	_rcPanel.b = _rcPanel.t + PANEL_SCALE_H  ;
}

fRECT* if_Panel_Scale_GetRect()
{
	return &_rcPanel;
}

void if_Panel_Scale_Put()
{
	fRECT rc_zoom_tbl = {400, 48,512, 72};
	fRECT rc_divi_tbl = {288, 48,400, 72};
	fRECT rc_zoom     = {408, 96,432,104};
	fRECT rc_divi     = {296, 96,320,104};

	g_dxdraw->tex_Put_View( _rcPanel.l                  , _rcPanel.t, &rc_zoom_tbl, SURF_TABLES );
	g_dxdraw->tex_Put_View( _rcPanel.l + rc_zoom_tbl.w(), _rcPanel.t, &rc_divi_tbl, SURF_TABLES );

	rc_zoom.t += 16 * _zoom_index;
	rc_zoom.b  = rc_zoom.t + 8 ;
	g_dxdraw->tex_Put_View( _rcPanel.l +  48, _rcPanel.t + 8, &rc_zoom, SURF_TABLES );

	rc_divi.t += 16 * _divi_index;
	rc_divi.b  = rc_divi.t + 8 ;
	g_dxdraw->tex_Put_View( _rcPanel.l + 160, _rcPanel.t + 8, &rc_divi, SURF_TABLES );
}

bool if_Panel_Scale_IsZoomButton( float x, float y )
{
	if( x >= _rcPanel.l + _BUTTON_ZOOM_X             &&
		x <  _rcPanel.l + _BUTTON_ZOOM_X + _BUTTON_W &&
		y >= _rcPanel.t + _BUTTON_ZOOM_Y             &&
		y <  _rcPanel.t + _BUTTON_ZOOM_Y + _BUTTON_H ) return true;
	return false;
}

bool if_Panel_Scale_IsDiviButton( float x, float y )
{
	if( x >= _rcPanel.l + _BUTTON_DIVI_X             &&
		x <  _rcPanel.l + _BUTTON_DIVI_X + _BUTTON_W &&
		y >= _rcPanel.t + _BUTTON_DIVI_Y             &&
		y <  _rcPanel.t + _BUTTON_DIVI_Y + _BUTTON_H ) return true;
	return false;
}


int32_t  if_Panel_Scale_GetDivision()
{
	return	_divi_table[ _divi_index ];
}

void if_Panel_Scale_SetDivision( int32_t divi_index )
{
	if( divi_index < 0 ) return;
	_divi_index = divi_index;
	if_BaseField_SetParameter( NULL, _divi_table[ _divi_index ] );
}


int32_t  if_Panel_Scale_GetZoom()
{
	return	_zoom_table[ _zoom_index ];
}

void if_Panel_Scale_SetZoom( int32_t zoom_index )
{
	if( zoom_index < 0 ) return;
	_zoom_index = zoom_index;
	if_BaseField_SetParameter( _zoom_table[ _zoom_index ], NULL );
}

void if_Panel_Scale_ShiftZoom( bool b )
{
	if( b )
	{
		if( _zoom_index < _ZOOM_NUM - 1 ) _zoom_index++;
	}
	else
	{
		if( _zoom_index > 0             ) _zoom_index--;
	}
	if_BaseField_SetParameter(_zoom_table[ _zoom_index ], NULL );
}
