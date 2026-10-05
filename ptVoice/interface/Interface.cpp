
#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw*   g_dxdraw;

#include <pxwAlteration.h>
extern pxwAlteration* g_alte  ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*  g_strm_xa2;

#include "../../Generic/KeyControl.h"
#include "../../Generic/MessageBox.h"
#include "../../Generic/if_Generic.h"

#include "../MidiInput.h"

#include "../CursorKeyCtrl.h"
extern CursorKeyCtrl* g_curkey;

#include "./Interface.h"
#include "./ptuiLayout.h"

#include "../WoiceUnit.h"
extern WoiceUnit*     g_vunit ;

void Interface::_release()
{
	_b_init = false;

	if( _organkey && g_vunit ) _organkey->key_reset( g_vunit );

	SAFE_DELETE( _cursor );

	pxMem_free( (void**)&_ptuis );

	SAFE_DELETE( _header    );
	SAFE_DELETE( _worktemp  );
	SAFE_DELETE( _rack_0    );
	SAFE_DELETE( _rack_0_wa );
	SAFE_DELETE( _rack_0_en );
	SAFE_DELETE( _rack_1    );
	SAFE_DELETE( _rack_1_wa );
	SAFE_DELETE( _rack_1_en );

	SAFE_DELETE( _wavetbl   );
	SAFE_DELETE( _wavehead  );
	SAFE_DELETE( _sines     );
	SAFE_DELETE( _wavepre   );

	SAFE_DELETE( _envehead  );
	SAFE_DELETE( _envetbl   );
	SAFE_DELETE( _rlstime   );

	SAFE_DELETE( _organkey  );
	SAFE_DELETE( _velocity  );
}

Interface:: Interface()
{
	_b_init   = false;
	_cursor   = NULL ;
	_ptuis    = NULL ;
	_ptui_num =     0;
}

Interface::~Interface()
{
	_release();
}


bool Interface::init()
{
	if( _b_init ) return false;

	_cursor  = new ptuiCursor     (); if( !_cursor     ->init() ) goto term;

	if( !pxMem_zero_alloc( (void**)&_ptuis, sizeof(ptui*) * PTUI_num ) ) goto term;

	_header    = new ptuiHeader     (); if( !_header   ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _header   ; _ptui_num++;
	_worktemp  = new ptuiWorkTemp   (); if( !_worktemp ->init(            ) ) goto term; _ptuis[ _ptui_num ] = _worktemp ; _ptui_num++;

	_organkey  = new ptuiOrganKey   (); if( !_organkey ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _organkey ; _ptui_num++;
	_velocity  = new ptuiVelocity   (); if( !_velocity ->init(            ) ) goto term; _ptuis[ _ptui_num ] = _velocity ; _ptui_num++;

	_rack_0    = new ptuiRackFull   (); if( !_rack_0   ->init( g_vunit, 0 ) ) goto term; _ptuis[ _ptui_num ] = _rack_0   ; _ptui_num++;
	_rack_0_wa = new ptuiWave_mini  (); if( !_rack_0_wa->init( g_vunit, 0 ) ) goto term; _ptuis[ _ptui_num ] = _rack_0_wa; _ptui_num++;
	_rack_0_en = new ptuiEnve_mini  (); if( !_rack_0_en->init( g_vunit, 0 ) ) goto term; _ptuis[ _ptui_num ] = _rack_0_en; _ptui_num++;
	_rack_1    = new ptuiRackFull   (); if( !_rack_1   ->init( g_vunit, 1 ) ) goto term; _ptuis[ _ptui_num ] = _rack_1   ; _ptui_num++;
	_rack_1_wa = new ptuiWave_mini  (); if( !_rack_1_wa->init( g_vunit, 1 ) ) goto term; _ptuis[ _ptui_num ] = _rack_1_wa; _ptui_num++;
	_rack_1_en = new ptuiEnve_mini  (); if( !_rack_1_en->init( g_vunit, 1 ) ) goto term; _ptuis[ _ptui_num ] = _rack_1_en; _ptui_num++;
	_rack_half = new ptuiRackHalf   (); if( !_rack_half->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _rack_half; _ptui_num++;
	_wavetbl   = new ptuiWave       (); if( !_wavetbl  ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _wavetbl  ; _ptui_num++;
	_wavehead  = new ptuiWaveHead   (); if( !_wavehead ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _wavehead ; _ptui_num++;
	_sines     = new ptuiSines      (); if( !_sines    ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _sines    ; _ptui_num++;
	_wavepre   = new ptuiWavePreset (); if( !_wavepre  ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _wavepre  ; _ptui_num++;
	_envetbl   = new ptuiEnvelope   (); if( !_envetbl  ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _envetbl  ; _ptui_num++;
	_envehead  = new ptuiEnveHead   (); if( !_envehead ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _envehead ; _ptui_num++;
	_rlstime   = new ptuiReleaseTime(); if( !_rlstime  ->init( g_vunit    ) ) goto term; _ptuis[ _ptui_num ] = _rlstime  ; _ptui_num++;

	_b_init = true;
term:
	if( !_b_init ) _release();
	return _b_init;
}

bool Interface::organkey_reset()
{
	if( !_b_init ) return false;
	_organkey->key_reset( g_vunit );
	return true;
}

bool Interface::map_redraw_set()
{
	if( !_b_init ) return false;
	_wavetbl  ->redraw_set();
	_envetbl  ->redraw_set();
	_rack_0_wa->redraw_set();
	_rack_0_en->redraw_set();
	_rack_1_wa->redraw_set();
	_rack_1_en->redraw_set();
	return true;
}


static void _GetMousePointer( HWND hWnd, float *x, float *y )
{
	POINT pt;
	GetCursorPos  ( &pt );
	ScreenToClient( hWnd, &pt );
	*x = (float)pt.x / g_dxdraw->get_screen_mag();
	*y = (float)pt.y / g_dxdraw->get_screen_mag();
}

static int32_t _CountFramePerSecond()
{
	static bool     bFirst    = true;
	static uint32_t wait      = 0;
	static uint32_t count     = 0;
	static int32_t  max_count = 0;

	if( bFirst )
	{
		wait   = GetTickCount();
		bFirst = false;
	}

	count++;
	if( wait + 1000 <= GetTickCount() ){
		wait     += 1000;
		max_count = count;
		count     = 0;
	}

	return max_count;
}


bool Interface::_key_control() //return b_draw;
{
	bool b_draw = false;

	KeyControl_UpdateTrigger();

	if( KeyControl_IsKey( VK_CONTROL ) )
	{
		if( KeyControl_IsKeyTrigger( VK_UP   ) ){ _organkey->key_octave_shift( false ); return true; }
		if( KeyControl_IsKeyTrigger( VK_DOWN ) ){ _organkey->key_octave_shift( true  ); return true; }
	}

	int32_t mv  = 1;
	int32_t idx = 0;

	if( KeyControl_IsKey( VK_SHIFT ) ) mv = 4;

	switch( g_curkey->get( &idx ) )
	{
	case CursorKey_wave:

		if( KeyControl_IsKey( VK_UP    ) ){ _wavetbl->keyctrl_move( idx, 0, mv *  1 ); b_draw = true; }
		if( KeyControl_IsKey( VK_DOWN  ) ){ _wavetbl->keyctrl_move( idx, 0, mv * -1 ); b_draw = true; }
		if( KeyControl_IsKey( VK_LEFT  ) ){ _wavetbl->keyctrl_move( idx, mv * -1, 0 ); b_draw = true; }
		if( KeyControl_IsKey( VK_RIGHT ) ){ _wavetbl->keyctrl_move( idx, mv *  1, 0 ); b_draw = true; }

		if( KeyControl_IsKeyTrigger( VK_TAB ) )
		{
			if( _wavetbl->keyctrl_tab( KeyControl_IsKey( VK_SHIFT ) ) ) b_draw = true;
		}
		break;

	case CursorKey_envelope:

		if( KeyControl_IsKey( VK_UP    ) ){ _envetbl->keyctrl_move( idx, 0, mv *  1 ); b_draw = true; }
		if( KeyControl_IsKey( VK_DOWN  ) ){ _envetbl->keyctrl_move( idx, 0, mv * -1 ); b_draw = true; }
		if( KeyControl_IsKey( VK_LEFT  ) ){ _envetbl->keyctrl_move( idx, mv * -1, 0 ); b_draw = true; }
		if( KeyControl_IsKey( VK_RIGHT ) ){ _envetbl->keyctrl_move( idx, mv *  1, 0 ); b_draw = true; }

		if( KeyControl_IsKeyTrigger( VK_TAB ) )
		{
			if( _envetbl->keyctrl_tab( KeyControl_IsKey( VK_SHIFT ) ) ) b_draw = true;
		}
		break;
	}

	return b_draw;
}


bool Interface::_mode_update()
{
	for( int i = 0; i < _ptui_num; i++ ){ if( _ptuis[ i ] ) _ptuis[ i ]->show_set( false ); }

	_header  ->show_set( true );
	_worktemp->show_set( true );
	_velocity->show_set( true );
	_organkey->show_set( true );

	switch( g_vunit->mode_get() )
	{
	case WOICEUNIT_MODE_rack     :

		_rack_0   ->show_set( true );
		_rack_0_wa->show_set( true );
		_rack_0_en->show_set( true );
		_rack_1   ->show_set( true );

		if( g_vunit->get_voice( 1 )->volume )
		{
			_rack_1_wa->show_set( true );
			_rack_1_en->show_set( true );
		}

		_rack_0->mute_button_update();
		_rack_1->mute_button_update();

		break;

	case WOICEUNIT_MODE_wave    :

		_rack_half->show_set( true );
		_wavehead ->show_set( true );
		_wavepre  ->show_set( true );
		_wavetbl  ->show_set( true );

		if( g_vunit->get_voice()->type == pxtnVOICE_Overtone ) _sines->show_set( true );
		break;

	case WOICEUNIT_MODE_envelope:

		_rack_half->show_set( true );
		_envehead ->show_set( true );
		_envetbl  ->show_set( true );
		_rlstime  ->show_set( true );
		break;

	default:
		return false;
	}
	return true;
}


#define _FLIPWAIT 20


bool Interface::proc( const fRECT* rc_view, const pxMouse* mouse, bool b_draw )
{
	if( !_b_init ) return false;

	g_dxdraw->SetViewport( rc_view->l, rc_view->t, rc_view->r, rc_view->b, 0, 0 );

	_mode_update();

	g_vunit->frame_alte_clear();

	// Make Rect ====================================================

	for( int i = 0; i < _ptui_num; i++ ){ if( _ptuis[ i ] ) _ptuis[ i ]->proc_begin( rc_view ); }

	// Action ========================================================
	if( _key_control() ) b_draw = true;

	MidiInput_Procedure();

	if( _organkey->key_proc( g_vunit ) ) b_draw = true;

	_cursor->set_mouse_status( mouse );

	for( int i = 0; i < _ptui_num; i++ ){ if( _ptuis[ i ] && _ptuis[ i ]->check_on_map( _cursor ) ) break; }

	if( !_cursor->get_action() )
	{
		bool b_proc = false;
		for( int i = _ptui_num - 1; !b_proc && i >= 0 ; i-- )
		{
			if( _ptuis[ i ] ) if( _cursor->action_free_try( _ptuis[ i ] ) ) b_proc = true;
		}
		if     ( b_proc ) b_draw = true;
		else if( _cursor->is_trigger_left() ){ g_curkey->clear(); b_draw = true; }
	}
	else
	{
		_cursor->action_busy();
	}

	_mode_update();

	// frame alte
	{
		uint32_t flags = g_vunit->frame_alte_get();
		if( flags & WOICEUNIT_ALTE_channel  )
		{
			_wavetbl->redraw_set();
			_envetbl->redraw_set();
		}

		if( flags & WOICEUNIT_ALTE_wave_screen ){ _wavetbl->redraw_set(); }
		if( flags & WOICEUNIT_ALTE_wave_voice  ){ g_vunit->strm_ready_sample  (); g_alte->set(); }
		if( flags & WOICEUNIT_ALTE_enve_screen ){ _envetbl->redraw_set(); }
		if( flags & WOICEUNIT_ALTE_enve_voice  ){ g_vunit->strm_ready_envelope(); g_alte->set(); }
		if( flags & WOICEUNIT_ALTE_etc         ){ g_curkey->clear();              g_alte->set(); }
		if( flags & (WOICEUNIT_ALTE_wave_voice|WOICEUNIT_ALTE_enve_voice|WOICEUNIT_ALTE_etc) ) _worktemp->last_unit_clear();

		if( flags & WOICEUNIT_ALTE_wave_mini0  ){ _rack_0_wa->redraw_set(); }
		if( flags & WOICEUNIT_ALTE_wave_mini1  ){ _rack_1_wa->redraw_set(); }
		if( flags & WOICEUNIT_ALTE_enve_mini0  ){ _rack_0_en->redraw_set(); }
		if( flags & WOICEUNIT_ALTE_enve_mini1  )
		{ _rack_1_en->redraw_set(); }
	}


	// 表示 ==========================================================
	if( b_draw )
	{
		bool b_tex_reloaded = false;

		g_dxdraw->Begin( &b_tex_reloaded );

		g_dxdraw->Fill( 1, 0, 0.144f,0.187f );

		if( b_tex_reloaded )
		{
			_wavetbl  ->redraw_set();
			_envetbl  ->redraw_set();
			_rack_0_wa->redraw_set();
			_rack_1_wa->redraw_set();
			_rack_0_en->redraw_set();
			_rack_1_en->redraw_set();
		}

		// map update..
		_wavetbl  ->try_update();
		_envetbl  ->try_update();
		_rack_0_wa->try_update();
		_rack_1_wa->try_update();
		_rack_0_en->try_update();
		_rack_1_en->try_update();

		for( int i = 0; i < _ptui_num; i++ ){ if( _ptuis[ i ] ) _ptuis[ i ]->put( _cursor ); }


		// フレーム数
		uint32_t fps   = _CountFramePerSecond();
		fRECT    rcFps = {112, 96,136,104};
		g_dxdraw->tex_Put_View( rc_view->r - 40 - 4,  2, &rcFps, SURF_PARTS );
		if_gen_num6( rc_view->r - 16 - 4,  2, fps, 2 );
	}

	{
		static uint32_t wait = 0;
		static uint32_t count;

		while( 1 )
		{
			Sleep( 1 );
			count = GetTickCount();
			if( count >= wait + _FLIPWAIT )
			{
				if( count >= wait + _FLIPWAIT * 5 ) wait  = count;
				else                                wait += _FLIPWAIT;
				break;
			}
		}
		if( b_draw ) g_dxdraw->End( true );
	}
	return true;
}
