
#include <pxtnService.h>
extern pxtnService *g_pxtn;


#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_gen_Scroll.h"
#include "../../Generic/if_Generic.h"

#include "../UnitFocus.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_ToolPanel.h"
#include "if_VolumePanel.h"
#include "if_Player.h"

extern if_gen_Scroll  g_ScrlUnitV;

static fRECT _rcField = {0};
static fRECT _rc_view = {0};

void if_UnitTray_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	_rcField.l = rc_view->l;
	_rcField.r = rc_view->l + NAMETRAY_WIDTH;
	_rcField.t = rc_view->t + HEADER_H + TOOLPANEL_HEIGHT;

	switch( if_ToolPanel_GetMode() )
	{
	case enum_ToolMode_U    :

		if( if_VolumePanel_IsUse() ) _rcField.b  = rc_view->b - SCROLL_HEIGHT  - VOLUME_HEIGHT;
		else                         _rcField.b  = rc_view->b - SCROLL_HEIGHT;
		break;

	case enum_ToolMode_K:

		_rcField.b  = rc_view->t + HEADER_H + TOOLPANEL_HEIGHT + UNIT_HEIGHT;
		break;

	default: _rcField.b = _rcField.t; break;
	}

	g_ScrlUnitV.SetSize( (float)( UNIT_HEIGHT * g_pxtn->Unit_Num() ) );
}

fRECT* if_UnitTray_GetRect()
{
	return &_rcField;
}

static void _UnitTray_RedrawName( int32_t u )
{
	fRECT   rc   = {4,0,UNITNAME_WIDTH, UNITNAME_HEIGHT};
	int32_t surface;
	float   y;

	const pxtnUnit *p_unit = g_pxtn->Unit_Get( u ); if( !p_unit ) return;
	surface = SURF_UNITNAME + u / 10;
	y       = (float)( (u % 10) * UNITNAME_HEIGHT );
	rc.t    = y;
	rc.b    = rc.t + UNITNAME_HEIGHT;

	static const char *_default_name = "-PXTONE UNIT-";

	const char* p_name   = p_unit->get_name_buf( NULL );

	if( !p_name ) p_name = _default_name;
	g_dxdraw->tex_glyph_text ( surface, p_name, &rc, 1 );
}

void if_UnitTray_RedrawAllName( const uxDS& new_font_name )
{
	for( int32_t s = SURF_UNITNAME; s < SURF_UNITNAME+10; s++ )
	{
		if( new_font_name ) g_dxdraw->tex_glyph_another_font( s, new_font_name, 12, false );
		g_dxdraw->tex_glyph_clear( s );
		g_dxdraw->tex_Clear      ( s );
	}
	int32_t max_unit = g_pxtn->Unit_Num();
	for( int32_t u = 0; u < max_unit; u++ ) _UnitTray_RedrawName( u );
}


void if_UnitTray_LumpSelect( int32_t start_u, int32_t end_u )
{
	if( start_u > end_u )
	{
		int32_t   u = start_u;
		start_u = end_u  ;
		end_u   = u      ;
	}

	for( int32_t u = start_u; u <= end_u; u++ )
	{
		g_pxtn->Unit_Get_variable( u )->set_operated( true );
	}
}



// ユニットネームを表示
void if_UnitTray_Put()
{
	int32_t u;
	float   xpos;
	float   ypos;
	int32_t ani_no;
	float   offset;
	int32_t unit_num;
	int32_t focus;

	bool bPart;

	fRECT rcTile;
	fRECT rc_tray     = {368,  0,512,160};
	fRECT rc_play     = {368,208,384,224};
	fRECT rc_focus    = {384,224,512,240};
	fRECT rc_name[] =
	{
		{384,176,512,192},
		{384,192,512,208},
	};
	fRECT rc_part[] =
	{
		{368,176,384,192},
		{368,192,384,208},
	};

	fRECT   rcName = {0,0,UNITNAME_WIDTH,0};
	int32_t name_surf;

	// ネームトレイ
	rcTile   = _rcField;
	offset   = g_ScrlUnitV.GetOffset();
	if_gen_tile_v( &rcTile, &rc_tray, 1, (int32_t)offset, SURF_FIELDS );

	unit_num = g_pxtn->Unit_Num();
	xpos     = _rcField.l + 16;

	focus    = UnitFocus_Get();
	if( focus < 0 ) return;
	bPart = if_Player_flag_is_check_mute() ;

	for( u = 0; u < unit_num; u++ )
	{
		const pxtnUnit *p_unit = g_pxtn->Unit_Get( u );

		ypos = _rcField.t + u * UNIT_HEIGHT - offset;

		if( ypos > _rcField.t - UNIT_HEIGHT && ypos < _rcField.b)
		{
			// ネームボックス
			if( UnitFocus_IsFocusedOrOperated( u ) ) ani_no = 1;
			else                                     ani_no = 0;
			g_dxdraw->tex_Put_View( xpos, ypos, &rc_name[ ani_no ], SURF_FIELDS );
			if( bPart )
			{
				g_dxdraw->tex_Put_View( xpos, ypos, &rc_part[ ani_no ], SURF_FIELDS );
				if( p_unit->get_played() ) g_dxdraw->tex_Put_View( xpos, ypos, &rc_play, SURF_FIELDS );
			}

			// 有効チェック
			if( focus == u ) g_dxdraw->tex_Put_View( xpos, ypos, &rc_focus,  SURF_FIELDS );

			rcName.t  = (float)( (u %10)  * UNITNAME_HEIGHT   );
			rcName.b  = (float)( rcName.t + UNITNAME_HEIGHT-2 );
			name_surf = SURF_UNITNAME + u         /10;
			g_dxdraw->tex_Put_View( xpos + 16, ypos+2, &rcName, name_surf );
		}
	}
}

bool if_UnitTray_GetNamePosition( int32_t u, float *px, float *py )
{
	float offset;

	offset = g_ScrlUnitV.GetOffset();
	if( !g_pxtn->Unit_Get( u ) ) return false;
	*px    = _rcField.l + 16;
	*py    = _rcField.t + u * UNIT_HEIGHT - offset;
	return true;
}

// ユニットトレイの縦スクロール調整
void if_UnitTray_JustScroll()
{
	float scroll_y;
	int32_t u;

	if_UnitTray_SetRect( &_rc_view );

	u         = UnitFocus_Get(); if( u < 0 ) return;
	scroll_y  = (float)( UNIT_HEIGHT * u + UNIT_HEIGHT / 2 );
	scroll_y -= (_rcField.b - _rcField.t)     / 2;

	g_ScrlUnitV.SetOffset( scroll_y );
}
