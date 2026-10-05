#include <pxStdDef.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw*   g_dxdraw;

#include "./ptVoice.h"

#define _KETA 4

void ptv_put_value( float x, float y, int32_t value, const fRECT* rc_clip )
{
	fRECT   rcMinus = {72,488,76,496};
	int32_t tbl0[]  = {1,10,100,1000};
	int32_t tbl9[]  = {9,99,999,9999};

	fRECT rc        = {0};
	fRECT rcNum     = { 32,488, 36,496};
	float num_w     = rcNum.w();

	int32_t a       =     0;
	bool    bPut    = false;
	bool    bMinus  = false;
	int32_t offset  =     0;
	float   xpos    =     0;

	if( value > tbl9[_KETA-1] )
		value = tbl9[_KETA-1];

	if( value < 0 ){ value *= -1; bMinus = true; }

	xpos = x;

	for( offset = _KETA-1; offset >= 0; offset-- )
	{
		a     = value/tbl0[offset];
		value = value%tbl0[offset];
		rc    = rcNum;
		rc.l  += num_w * a; rc.r = rc.l + num_w;
		if( bMinus && a && !bPut ) g_dxdraw->tex_Put_Clip( xpos - 4, y, &rcMinus,  SURF_PARTS, rc_clip );
		if( bPut || a || !offset ) g_dxdraw->tex_Put_Clip( xpos,     y, &rc     , SURF_PARTS, rc_clip );
		if(          a          ) bPut = true;
		xpos += 4;
	}
}

void ptv_put_value( float x, float y, int32_t value )
{
	ptv_put_value( x, y, value, NULL );
}

void ptv_put_envesec( float x, float y, int32_t value, const fRECT* rc_clip )
{
	if( value <  0 ) value =  0;
	if( value > 99 ) value = 99;

	fRECT   rc_line  = {44,112,48,120};
	fRECT   rc_point = {40,112,44,120};

	fRECT   rc     = {0};
	fRECT   rc_num = {0,112,4,120};
	int32_t num_w  = rc_num.w();

	g_dxdraw->tex_Put_Clip( x, y, &rc_line, SURF_PARTS, rc_clip );
	x+= 4;

	rc    = rc_num;
	rc.l  += num_w * (value/10); rc.r = rc.l + num_w;
	g_dxdraw->tex_Put_Clip( x, y, &rc,  SURF_PARTS, rc_clip );
	x+= 4;

	g_dxdraw->tex_Put_Clip( x, y, &rc_point, SURF_PARTS, rc_clip );
	x+= 4;

	rc    = rc_num;
	rc.l  += num_w * (value%10); rc.r = rc.l + num_w;
	g_dxdraw->tex_Put_Clip( x, y, &rc,  SURF_PARTS, rc_clip );
/*

	fRECT   rcMinus = {72,488,76,496};
	int32_t tbl0[]  = {1,10,100,1000};
	int32_t tbl9[]  = {9,99,999,9999};

	fRECT rc        = {0};
	fRECT rcNum     = { 32,488, 36,496};
	float num_w     = rcNum.w();

	int32_t a       =     0;
	bool    bPut    = false;
	bool    bMinus  = false;
	int32_t offset  =     0;
	float   xpos    =     0;

	if( value > tbl9[_KETA-1] )
		value = tbl9[_KETA-1];

	if( value < 0 ){ value *= -1; bMinus = true; }

	xpos = x;

	for( offset = _KETA-1; offset >= 0; offset-- )
	{
		a     = value/tbl0[offset];
		value = value%tbl0[offset];
		rc    = rcNum;
		rc.l  += num_w * a; rc.r = rc.l + num_w;
		if( bMinus && a && !bPut ) g_dxdraw->tex_Put_Clip( xpos - 4, y, &rcMinus,  SURF_PARTS, rc_clip );
		if( bPut || a || !offset ) g_dxdraw->tex_Put_Clip( xpos,     y, &rc     , SURF_PARTS, rc_clip );
		if(          a          ) bPut = true;
		xpos += 4;
	}
	*/
}