#include <pxMem.h>

#include "./ptuiMap.h"

ptuiMap::ptuiMap()
{
	_b_map_init   = false;

	_p_map_view   = NULL ;
	_p_map_posi   = NULL ;
	_p_map_nega   = NULL ;
	_b_map_redraw = false;
}

void ptuiMap::_map_release()
{
	_b_map_init = false;
	pxMem_free( (void**)&_p_map_view );
	pxMem_free( (void**)&_p_map_posi );
	pxMem_free( (void**)&_p_map_nega );
}

ptuiMap::~ptuiMap()
{
	_map_release();
}

void ptuiMap::redraw_set   ()
{
	_b_map_redraw = true;
}

void ptuiMap::redraw_clear ()
{
	_b_map_redraw = false;
}

bool ptuiMap::_map_init( int32_t map_w, int32_t map_h, int32_t surf_screen )
{
	if( _b_map_init ) return false;

	int32_t work_byte = sizeof(uint32_t) * map_w * map_h;

	if( !work_byte ) goto term;

	if( !pxMem_zero_alloc( (void**)&_p_map_view, work_byte ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&_p_map_posi, work_byte ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&_p_map_nega, work_byte ) ) goto term;

	_surf_screen = surf_screen;
	_b_map_init  = true       ;
term:
	if( !_b_map_init ) _map_release();
	return _b_map_init;
}

bool ptuiMap::try_update()
{
	if( !_b_show ) return true;
	return _try_update();
}

bool ptuiMap::keyctrl_move( int32_t idx, int32_t mv_x, int32_t mv_y )
{
	if( !_b_show ) return true;
	return _keyctrl_move( idx, mv_x, mv_y );
}

bool ptuiMap::keyctrl_tab ( bool b_back                             )
{
	if( !_b_show ) return true;
	return _keyctrl_tab( b_back );
}

bool ptuiMap::_keyctrl_move( int32_t idx, int32_t mv_x, int32_t mv_y )
{
	return false;
}

bool ptuiMap::_keyctrl_tab ( bool b_back                             )
{
	return false;
}
