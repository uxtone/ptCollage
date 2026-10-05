
#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_Generic.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_Panel_Scale.h"
#include "if_PcmTable.h"

#define _BUTTON_ZOOM_X  80
#define _BUTTON_ZOOM_Y   4
#define _BUTTON_W       16
#define _BUTTON_H       16

static fRECT _rcPanel = {0};

#define _ZOOM_NUM 6

static const int32_t _zoom_table[ _ZOOM_NUM ] =
{
	80,
	40,
	20,
	10,
	5,
	1,
};

#define _DEFAULT_ZOOM_INDEX 4
static int32_t        _zoom_index    = _DEFAULT_ZOOM_INDEX;
static const TCHAR*   _filename_zoom = _T("zoom.bin");
static const pxFile2* _ref_file_profile = NULL;

static bool _zoom_save()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _filename_zoom, NULL, NULL ) ) goto term;
	if( !desc->w_asfile( &_zoom_index, sizeof(_zoom_index), 1 )         ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );

	return b_ret;
}

static bool _zoom_load()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_r( &desc, _filename_zoom, NULL, NULL ) ) goto term;
	if( !desc->r( &_zoom_index, sizeof(_zoom_index), 1 ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	if( !b_ret ) _zoom_index = _DEFAULT_ZOOM_INDEX;
	return b_ret;
}

void if_Panel_Scale_init( const pxFile2* ref_file_profile )
{
	_ref_file_profile = ref_file_profile;
	_zoom_load();
	if_PcmTable_SetParameter( _zoom_table[ _zoom_index ] );
}

void if_Panel_Scale_SetRect( const fRECT *rc_view )
{
	float x = rc_view->r - PANEL_SCALE_W - PANEL_LOGO_W;
	if( x < 0 ) x = 0;
	_rcPanel.l = x              ;
	_rcPanel.t = rc_view->t                  ;
	_rcPanel.r = _rcPanel.l + PANEL_SCALE_W  ;
	_rcPanel.b = _rcPanel.t + PANEL_SCALE_H  ;
}

fRECT*if_Panel_Scale_GetRect()
{
	return &_rcPanel;
}

void if_Panel_Scale_Put()
{
	fRECT rcPanel = {400, 24,512, 48};
	fRECT rc_zoom = {408, 72,432, 80};

	g_dxdraw->tex_Put_View( _rcPanel.l, _rcPanel.t, &rcPanel, SURF_PARTS );

	rc_zoom.t += 16 * _zoom_index;
	rc_zoom.b  = rc_zoom.t + 8 ;
	g_dxdraw->tex_Put_View( _rcPanel.l +  48, _rcPanel.t + 8, &rc_zoom, SURF_PARTS );
}

bool if_Panel_Scale_IsZoomButton( float x, float y )
{
	if( x >= _rcPanel.l + _BUTTON_ZOOM_X             &&
		x <  _rcPanel.l + _BUTTON_ZOOM_X + _BUTTON_W &&
		y >= _rcPanel.t + _BUTTON_ZOOM_Y             &&
		y <  _rcPanel.t + _BUTTON_ZOOM_Y + _BUTTON_H ) return true;
	return false;
}

int32_t  if_Panel_Scale_GetZoom()
{
	return	_zoom_table[ _zoom_index ];
}

void if_Panel_Scale_SetZoom( int32_t zoom_index )
{
	if(  zoom_index < 0          ) return;
	if(  zoom_index >= _ZOOM_NUM ) return;
	_zoom_index = zoom_index;
	_zoom_save();
	if_PcmTable_SetParameter( _zoom_table[ _zoom_index ] );
}

void if_Panel_Scale_ShiftZoom( bool b )
{
	if( b ) _zoom_index++;
	else    _zoom_index--;

	if( _zoom_index >= _ZOOM_NUM ) _zoom_index = _ZOOM_NUM - 1;
	if( _zoom_index <          0 ) _zoom_index =             0;
	_zoom_save();
	if_PcmTable_SetParameter( _zoom_table[ _zoom_index ] );
}
