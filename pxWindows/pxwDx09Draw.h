// '16/03/09 pxwDx09Draw.
// '17/01/20 Begin( + "bool* pb_tex_reloaded" ).

#include <pxStdDef.h>

#ifdef pxwDX09

#ifndef pxDx09Draw_H
#define pxDx09Draw_H

#include <pxStdDef.h>

#include <pxSurface.h>
#include <pxPalette.h>
#include <pxFile2.h>

#include "../DirectX9/d3dx9.h"

#include "./pxwGlyph1.h"

typedef struct
{
	fRECT    rc_dst;
	uint32_t code  ;
}
pxwDx09GLYPHUNIT;

typedef struct
{
	uint32_t           flags         ;
	uxDS              p_name        ;
	uxDS              p_dir         ;
	LPDIRECT3DTEXTURE9 p_tex         ;
	int32_t            origin_img_w  ;
	int32_t            origin_img_h  ;
	float              stretch       ;
	uint32_t           color_ARGB    ;

	pxwGlyph1*         glph          ;
	pxwDx09GLYPHUNIT*  glph_units    ;
	int32_t            glph_unit_num ;
	bool               glph_b_UTF16LE;
}
pxwDx09TEXTURE;

// integer screen magnification: system DPI / 96, rounded up. usable before init().
int32_t pxwDx09Draw_system_mag();

class pxwDx09Draw
{
private:

enum
{
	_MAX_DXWTEXT      = 1024,
	_BUFSIZE_FONTNAME =  128,
};

const pxFile2* _ref_file_data;

float    _screen_mag           ;
bool     _b_init               ;
bool     _b_fullscreen         ;
bool     _b_present_error      ;

int32_t  _delay_fullscreen_text;

float    _client_rect_w        ;
float    _client_rect_h        ;

fSIZE    _size_view            ;


    // dx3d.. ------------------------------
LPDIRECT3D9           _d3d    ;
LPDIRECT3DDEVICE9     _device ;

bool                  _b_on_scene    ;
bool                  _b_on_sprite   ;

LPD3DXFONT            _d3d_font      ;
D3DCOLOR              _d3d_font_color;

LPD3DXSPRITE          _sprt  ;
D3DPRESENT_PARAMETERS _d3dprm;
D3DPRESENT_PARAMETERS _d3dprm_window;
D3DPRESENT_PARAMETERS _d3dprm_fullscreen;

pxwDx09TEXTURE*       _texs;

fRECT                 _view_clip ;
float                 _view_ofs_x;
float                 _view_ofs_y;

const pxPalette*      _def_pal   ;

bool _device_reset( bool b_reload_tex );
bool _tex_reload  ();

int32_t  _scene_count;

void _sprite_begin();
void _sprite_end  ();

public:

pxwDx09Draw( const pxFile2* app_file_data );
~pxwDx09Draw();

bool init           ( HWND hWnd );
void release        ();

void  SetViewport    ( float l, float t, float r, float b, float ofs_x, float ofs_y         );
void  GetViewOffset  ( float *px, float *py );
void  GetViewClip    ( fRECT *p_rc_clip );
bool  WindowMode_mag ( HWND hwnd, float mag, float client_rect_w, float client_rect_h );
bool  Fullscreen_mag ( HWND hwnd, float mag );
float get_screen_mag() const;

void  FullScreenDelay();


bool  Begin        ( bool* pb_tex_reloaded );
bool  End          ( bool b_instant_flip   );

void  Fill         (                    float a, float r, float g, float b );
void  FillView     (                    float a, float r, float g, float b );

bool  FillRect     ( const fRECT *p_rc, float a, float r, float g, float b );
bool  FillRect_clip( const fRECT *p_rc, float a, float r, float g, float b, const fRECT *rc_clip );
bool  FillRect_clip( const fRECT *p_rc, uint32_t aBGR,                      const fRECT *rc_clip );
bool  FillRect_view( const fRECT *p_rc, float a, float r, float g, float b );
bool  FillRect_view( const fRECT *p_rc, uint32_t aBGR );

uint32_t tex_load  ( const uxDS& dir, const uxDS& name, int32_t tx_idx, float stretch, bool b_ignore_screen_mag );
uint32_t tex_load  ( const uxDS& dir, const uxDS& name, int32_t tx_idx, float stretch );
uint32_t tex_load  ( const uxDS& dir, const uxDS& name, int32_t tx_idx );
void     tex_release( int32_t t );
void     tex_release();
uint32_t tex_create           ( int32_t w, int32_t h, uint32_t argb, int32_t tx_idx );
bool     tex_GetSize_original ( float *p_w, float *p_h, int32_t tx_idx );
bool     tex_GetSize_stretched( float *p_w, float *p_h, int32_t tx_idx );

void     tex_Put_Clip   ( float x, float y, const fRECT *p_rc_src, int32_t t, const fRECT *p_rc_clip );
void     tex_Put_View   ( float x, float y, const fRECT *p_rc    , int32_t t );

bool     tex_blt        ( int32_t t, const uint32_t *p_src, int32_t src_w, int32_t src_h );

bool     tex_glyph_init ( int32_t t, const pxGLYPH_PARAM1 *p_prm, int32_t unit_num, bool b_UTF16LE );
bool     tex_glyph_moji ( int32_t t, uint32_t moji   , const fRECT *p_rc_dst, fSIZE *p_size );
bool     tex_glyph_text ( int32_t t, const char *text, const fRECT *p_rc_dst, int32_t gap_pix );
bool     tex_glyph_another_font( int32_t t, const uxDS& font_name, int32_t font_h, bool b_bold );
void     tex_glyph_clear( int32_t t );
bool     tex_glyph_color( int32_t t, uint32_t argb );

    // old(dx09).
bool     tex_GetOriginalSize( int32_t t, float *p_w, float *p_h );
bool     tex_Clear          ( int32_t t );

void     test_SetBitmap( float x, float y, fRECT *p_rc );
bool     test_CreateBitmap();

int32_t SceneCount() const;

    // directx 9..
bool d3d_font_init   ( const uxDS& font_name, int32_t font_size );
bool d3d_font_color  ( float a, float r, float g, float b );
bool d3d_font_put    ( const char *text, float x, float y, const fRECT *p_rc_view );
void font_draw_on_tex( int32_t t, const fRECT *p_rc_dst, const char *text );

bool restore         ( bool b_reload_texture );
bool is_device_valid ();

bool default_palette_set( const pxPalette* def_pal );
};

#endif
#endif
