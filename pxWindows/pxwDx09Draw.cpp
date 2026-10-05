#include <pxStdDef.h>

#ifdef pxwDX09

#include <stdio.h>
#include <tchar.h>

#include <pxStr.h>
#include <pxStrT.h>

#include "./pxwFile.h"
#include "./pxwDx09Draw.h"

#define SAFE_RELEASE(p) { if(p){(p)->Release();(p)=NULL;} }

#define _MAX_TEXTURE              64

#define _TEXTUREFLAG_EXTERIOR          0x01
#define _TEXTUREFLAG_PLANE             0x02
#define _TEXTUREFLAG_IGNORE_SCREEN_MAG 0x04

#define _DELAY_FULLSCREENTEXT    180

#define _BUFSIZE_DRAWFONT (1024 * 10)

static const TCHAR*  _default_font_name = _T("MS UI Gothic");
static const int32_t _default_font_size =                 11;

int32_t pxwDx09Draw::SceneCount() const{ return _scene_count; }

typedef struct
{
	float x;
	float y;
	float z;
	float rhw;
	DWORD color;
}
_VERTEXBOX;

static bool _malloc_zero( void **pp, int size )
{
	*pp = malloc( size ); if( !( *pp ) ) return false;
	memset( *pp, 0, size );              return true ;
}

static void _free( void **pp )
{
	if( *pp ){ free( *pp ); *pp = NULL; }
}

pxwDx09Draw::pxwDx09Draw( const pxFile2* file_data )
{
	_ref_file_data   = file_data;

	_screen_mag      =     1;
	_b_init          = false;
	_b_fullscreen    = false;
	_b_present_error = false;
	_b_on_scene      = false;
	_b_on_sprite     = false;
	_delay_fullscreen_text = _DELAY_FULLSCREENTEXT;

	_client_rect_w =     0;
	_client_rect_h =     0;

	memset( &_size_view, 0, sizeof(_size_view) );

	_d3d            = NULL;
	_device         = NULL;
	_d3d_font       = NULL;
	_d3d_font_color = D3DCOLOR_ARGB(255,255,255,255);
	_sprt           = NULL;

	// dx3d.. ------------------------------
	memset( &_d3dprm           , 0, sizeof(_d3dprm           ) );
	memset( &_d3dprm_window    , 0, sizeof(_d3dprm_window    ) );
	memset( &_d3dprm_fullscreen, 0, sizeof(_d3dprm_fullscreen) );

	_texs = NULL;

	memset( &_view_clip, 0, sizeof(_view_clip) );
	_view_ofs_x = 0;
	_view_ofs_y = 0;

	_def_pal    = NULL;
}

pxwDx09Draw::~pxwDx09Draw()
{
	release();
}

bool pxwDx09Draw::d3d_font_color( float a, float r, float g, float b )
{
	_d3d_font_color = D3DCOLOR_ARGB( (DWORD)(255.f*a), (DWORD)(255.f*r), (DWORD)(255.f*g), (DWORD)(255.f*b));
	return true;
}

bool pxwDx09Draw::d3d_font_put( const char* text, float x, float y, const fRECT *p_rc_view )
{
	if( !_d3d_font ) return false;

	RECT rc_dst;
	rc_dst.left   = (LONG)x;
	rc_dst.top    = (LONG)y;
	rc_dst.right  = (LONG)p_rc_view->r;
	rc_dst.bottom = (LONG)p_rc_view->b;

	_d3d_font->DrawTextA( _sprt, text, -1, &rc_dst, DT_LEFT | DT_NOCLIP, _d3d_font_color );
	return true;
}

bool pxwDx09Draw::is_device_valid()
{
	if( !_device || _device->TestCooperativeLevel() != D3D_OK ) return false;
	return true;
}

bool pxwDx09Draw::_tex_reload()
{
	for( int t = 0; t < _MAX_TEXTURE; t++ )
	{
		const pxwDx09TEXTURE *pt = &_texs[ t ];

		if( pt->flags & _TEXTUREFLAG_PLANE )
		{
			if( pt->p_tex && tex_create( pt->origin_img_w, pt->origin_img_h, pt->color_ARGB, t ) != t ) return false;
		}
		else
		{
			bool b_ignore = false; if( pt->flags & _TEXTUREFLAG_IGNORE_SCREEN_MAG ) b_ignore = true;
			if( pt->p_tex && tex_load( pt->p_dir, pt->p_name, t, pt->stretch, b_ignore ) != t ) return false;
		}

		if( pt->glph_units )
		{
			const pxwDx09GLYPHUNIT* pu = pt->glph_units;

			if( !pt->glph->reset_magnify( _screen_mag ) ) return false;

			for( int32_t u = 0; u < pt->glph_unit_num; u++, pu++ )
			{
				if( pu->code ) tex_glyph_moji( t, pu->code, &pu->rc_dst, NULL );
			}
		}
	}
	return true;
}

bool pxwDx09Draw::_device_reset  ( bool b_reload_tex )
{
	SAFE_RELEASE( _sprt     );
	SAFE_RELEASE( _d3d_font );

	if( FAILED( _device->Reset( &_d3dprm ) ) ) return false;

	if( FAILED(D3DXCreateSprite( _device, &_sprt ) )             ) return false;
	if( !d3d_font_init( _default_font_name, _default_font_size ) ) return false;

	if( b_reload_tex )
	{
		if( !_tex_reload() ) return false;
	}
	_b_present_error = false;

	return true;
}

void pxwDx09Draw::FullScreenDelay()
{
	_delay_fullscreen_text = _DELAY_FULLSCREENTEXT;
}

bool pxwDx09Draw::Fullscreen_mag( HWND hwnd, float mag )
{
	if( !_b_init      ) return false;
	if( _b_fullscreen ) return false;

	_b_fullscreen = true;

	_d3dprm_fullscreen.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
	_d3dprm_fullscreen.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);

	_d3dprm                = _d3dprm_fullscreen   ;
	_delay_fullscreen_text = _DELAY_FULLSCREENTEXT;
	if( mag ) _screen_mag = mag;
	return _device_reset( mag ? true : false );
}

bool pxwDx09Draw::WindowMode_mag( HWND hwnd, float mag, float client_rect_w, float client_rect_h )
{
	if( !_b_init ) return false;
	_b_fullscreen = false         ;
	_d3dprm       = _d3dprm_window;
	if( mag ) _screen_mag    = mag;
	_d3dprm.BackBufferWidth  = (int32_t)client_rect_w;
	_d3dprm.BackBufferHeight = (int32_t)client_rect_h;
	return _device_reset( mag ? true : false );
}

int32_t pxwDx09Draw_system_mag()
{
	int32_t dpi = 96;
	HDC     hdc = GetDC( NULL );
	if( hdc ){ dpi = GetDeviceCaps( hdc, LOGPIXELSX ); ReleaseDC( NULL, hdc ); }
	int32_t mag = ( dpi + 95 ) / 96; // round up to the next integer; never fractional.
	return mag < 1 ? 1 : mag;
}

float pxwDx09Draw::get_screen_mag() const
{
	return _screen_mag;
}

bool pxwDx09Draw::restore( bool b_reload_texture )
{
	if( !_b_init ) return false;
	return _device_reset( b_reload_texture );
}


bool pxwDx09Draw::init( HWND hWnd )
{
	if( !(  _texs = (pxwDx09TEXTURE*)malloc( sizeof(pxwDx09TEXTURE) * _MAX_TEXTURE ) ) ) return false;
	memset( _texs, 0,                        sizeof(pxwDx09TEXTURE) * _MAX_TEXTURE );

	if( !(_d3d = Direct3DCreate9( D3D_SDK_VERSION ) ) ) return false;

	_screen_mag = (float)pxwDx09Draw_system_mag();

	ZeroMemory( &_d3dprm_window, sizeof(_d3dprm_window) );

	D3DDISPLAYMODE d;
	_d3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d );
	d.RefreshRate;

	_d3dprm_window    .BackBufferWidth            = 0;
	_d3dprm_window    .BackBufferHeight           = 0;
	_d3dprm_window    .BackBufferFormat           = D3DFMT_UNKNOWN;
//	_d3dprm_window    .BackBufferFormat           = D3DFMT_A8R8G8B8;//D3DFMT_UNKNOWN;
	_d3dprm_window    .BackBufferCount            = 1;
	_d3dprm_window    .MultiSampleType            = D3DMULTISAMPLE_NONE;
	_d3dprm_window    .MultiSampleQuality         = 0;
	_d3dprm_window    .SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	_d3dprm_window    .hDeviceWindow              = hWnd;
	_d3dprm_window    .Windowed                   = TRUE;
	_d3dprm_window    .EnableAutoDepthStencil     = FALSE;
	_d3dprm_window    .AutoDepthStencilFormat     = D3DFMT_D16;
	_d3dprm_window    .Flags                      = 0;
	_d3dprm_window    .FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if( !(d.RefreshRate%60) ) _d3dprm_window.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT  ;
	else                      _d3dprm_window.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	ZeroMemory( &_d3dprm_fullscreen, sizeof(_d3dprm_fullscreen));

	_d3dprm_fullscreen.BackBufferWidth            = GetSystemMetrics(SM_CXSCREEN);
	_d3dprm_fullscreen.BackBufferHeight           = GetSystemMetrics(SM_CYSCREEN);
	_d3dprm_fullscreen.BackBufferFormat           = D3DFMT_X8R8G8B8;
	_d3dprm_fullscreen.BackBufferCount            = 1;
	_d3dprm_fullscreen.MultiSampleType            = D3DMULTISAMPLE_NONE  ;
	_d3dprm_fullscreen.MultiSampleQuality         = 0;
	_d3dprm_fullscreen.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	_d3dprm_fullscreen.hDeviceWindow              = hWnd ;
	_d3dprm_fullscreen.Windowed                   = FALSE;
	_d3dprm_fullscreen.EnableAutoDepthStencil     = FALSE;
	_d3dprm_fullscreen.AutoDepthStencilFormat     = D3DFMT_D16;
	_d3dprm_fullscreen.Flags                      = 0;
	_d3dprm_fullscreen.FullScreen_RefreshRateInHz = 0;
	if( !(d.RefreshRate%60) ) _d3dprm_fullscreen.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT  ;
	else                      _d3dprm_fullscreen.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	_d3dprm = _b_fullscreen ? _d3dprm_fullscreen : _d3dprm_window;

	if( FAILED( _d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,D3DCREATE_HARDWARE_VERTEXPROCESSING, &_d3dprm, &_device ) ) &&
		FAILED( _d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING, &_d3dprm, &_device ) ) )
	{
		return false;
	}

	if( FAILED(D3DXCreateSprite( _device, &_sprt ) ) ) return false;

	if( !d3d_font_init( _default_font_name, _default_font_size ) ) return false;

	_b_present_error = false;
	_b_init          = true ;

	return true;
}

void pxwDx09Draw::release()
{
	SAFE_RELEASE( _sprt     );

	tex_release();

	SAFE_RELEASE( _d3d_font );
	SAFE_RELEASE( _device   );
	SAFE_RELEASE( _d3d      );

	if( _texs         ) free( _texs                ); _texs         = NULL;
}

void pxwDx09Draw::GetViewOffset( float *px, float *py )
{
	if( px ) *px = _view_ofs_x;
	if( py ) *py = _view_ofs_y;
}

void pxwDx09Draw::GetViewClip( fRECT *p_rc_clip )
{
	*p_rc_clip = _view_clip;
}

void pxwDx09Draw::SetViewport( float l, float t, float r, float b, float ofs_x, float ofs_y )
{
	// center..
	if( ofs_x < 0 )
	{
		_view_ofs_x = ( (_d3dprm.BackBufferWidth /_screen_mag) - (r - l) ) / 2;
		_view_ofs_y = ( (_d3dprm.BackBufferHeight/_screen_mag) - (b - t) ) / 2;
	}
	else
	{
		_view_ofs_x = ofs_x;
		_view_ofs_y = ofs_y;
	}

	_view_clip.l = l + _view_ofs_x;
	_view_clip.t = t + _view_ofs_y;
	_view_clip.r = r + _view_ofs_x;
	_view_clip.b = b + _view_ofs_y;

	_size_view.w = r - l;
	_size_view.h = b - t;
}

bool pxwDx09Draw::Begin( bool* pb_tex_reloaded )
{
	_b_on_scene = false;
	if( pb_tex_reloaded ) *pb_tex_reloaded = false;

	if( !_device ) return false;

	if( _b_present_error )
	{
		if( !_device_reset( true ) ) return false;
		if( pb_tex_reloaded ) *pb_tex_reloaded = true;
	}

	if( FAILED( _device->BeginScene() ) ) return false;

	_b_on_scene = true ;

	if( _b_fullscreen )
	{
		float w = (float)GetSystemMetrics( SM_CXSCREEN );
		float h = (float)GetSystemMetrics( SM_CYSCREEN );

		unsigned char r = 0x20;
		unsigned char g = 0x20;
		unsigned char b = 0x20;

		const _VERTEXBOX vtxs[4] =
		{
			{0, 0, 0.f, 1.f, D3DCOLOR_ARGB(0xff,r,g,b)},
			{w, 0, 0.f, 1.f, D3DCOLOR_ARGB(0xff,r,g,b)},
			{0, h, 0.f, 1.f, D3DCOLOR_ARGB(0xff,r,g,b)},
			{w, h, 0.f, 1.f, D3DCOLOR_ARGB(0xff,r,g,b)}
		};

		_device->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
		_device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vtxs, sizeof(vtxs[0]) );
	}
	_scene_count++;
	return true;
}

void pxwDx09Draw::_sprite_begin()
{
	if( !_b_on_scene || _b_on_sprite ) return;
	_sprt->Begin(D3DXSPRITE_ALPHABLEND);
	_b_on_sprite = true;
}

void pxwDx09Draw::_sprite_end()
{
	if( !_b_on_scene || !_b_on_sprite ) return;
	_sprt->End();
	_b_on_sprite = false;
}

bool pxwDx09Draw::End( bool b_instant_flip )
{
	if( _b_fullscreen && _delay_fullscreen_text )
	{
		_delay_fullscreen_text--;
		fRECT rc_dst;
		rc_dst.l = 0;
		rc_dst.t = 0;
		rc_dst.r = _client_rect_w;
		rc_dst.b = _client_rect_h;
		d3d_font_put( "[ Alt + Enter ] Window Mode.", 4, 4, &rc_dst );
	}

	_sprite_end();
	if( !_b_on_scene ) return true;

	if( FAILED( _device->EndScene() ) )
	{
	}

	if( FAILED( _device->Present( NULL, NULL, NULL, NULL ) ) )
	{
		_b_present_error = true;
	}

	_b_on_scene = false;
	return true;
}

bool pxwDx09Draw::tex_GetOriginalSize( int32_t t, float *p_w, float *p_h )
{
	if( t >= _MAX_TEXTURE || t < 0 ) return 0;
	pxwDx09TEXTURE *pt = &_texs[ t ];
	if( !pt->p_tex ) return false;
	if( p_w ) *p_w = (float)pt->origin_img_w;
	if( p_h ) *p_h = (float)pt->origin_img_h;
	return true;
}

bool pxwDx09Draw::tex_Clear( int32_t t )
{
	if( t >= _MAX_TEXTURE || t < 0 ) return false;
	pxwDx09TEXTURE *pt = &_texs[ t ];
	if( !pt->p_tex ) return false;

	D3DLOCKED_RECT rc;

	if( FAILED(pt->p_tex->LockRect( 0, &rc, NULL, 0 ) ) ) return false;
	DWORD* p     = (DWORD*)rc.pBits;
	float  mag   = (pt->flags & _TEXTUREFLAG_IGNORE_SCREEN_MAG) ? 1 : _screen_mag;
	float  strch = pt->stretch; if( !strch ) strch = 1;

	int w = (int32_t)( pt->origin_img_w * strch * mag );
	int h = (int32_t)( pt->origin_img_h * strch * mag );

	for( int y = 0; y < h; y++ )
	{
		DWORD* p_row = (DWORD*)( (uint8_t*)p + rc.Pitch * y ); // the pitch is not always the width.
		for( int x = 0; x < w; x++ ) p_row[ x ] = 0;
	}
	pt->p_tex->UnlockRect( 0 );
	return true;
}

uint32_t  pxwDx09Draw::tex_create ( int32_t w, int32_t h, uint32_t argb, int t )
{
	pxwDx09TEXTURE *pt;

	if( t < 0 )
	{
		t = 1;
		for( t; t <  _MAX_TEXTURE; t++ ){ if( !_texs[ t ].p_tex ) break; }
	}
	if( t >= _MAX_TEXTURE ) return 0;

	pt = &_texs[ t ]; if( pt->p_tex ) pt->p_tex->Release(); pt->p_tex = NULL;

	bool b_ret     = false;
	int  dst_w     = (int)( w * _screen_mag );
	int  dst_h     = (int)( h * _screen_mag );

	if( FAILED(D3DXCreateTexture( _device, dst_w, dst_h, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pt->p_tex ) ) ) goto End;

	D3DLOCKED_RECT rc; pt->p_tex->LockRect( 0, &rc, NULL, 0 );

	{
		DWORD    dst_pitch = rc.Pitch / sizeof(DWORD);
		int      src_pitch = dst_w;
		LPDWORD  p_dst     = (LPDWORD)rc.pBits + dst_pitch * (dst_h-1);

		for ( int src_y = 0; src_y < dst_h ; src_y++ )
		{
			int  dst_x   =     0;
			for ( int src_x = 0; src_x < dst_w; src_x++ )
			{
				p_dst[ dst_x ] = argb;
				dst_x++;
			}
			p_dst -= dst_pitch;    //次の行
		}
	}

	pt->p_tex->UnlockRect( 0 );

	_free( (void**)&pt->p_name );
	_free( (void**)&pt->p_dir  );

	pt->color_ARGB   = argb;
	pt->origin_img_w = w;
	pt->origin_img_h = h;
	pt->flags        = _TEXTUREFLAG_EXTERIOR|_TEXTUREFLAG_PLANE;
	pt->stretch      = 1;

	b_ret = true;
End:
	if( !b_ret ) tex_release( t );

	return b_ret ? t : 0;
}


bool pxwDx09Draw::d3d_font_init( const TCHAR *font_name, int32_t font_size )
{
	HRESULT hr = D3DXCreateFont(
		_device,
		(int32_t)( font_size * _screen_mag ), //height
		0,                                    // width
		FW_NORMAL,               // font style
		1,                       // mip map level
		false,                   // italic
		SHIFTJIS_CHARSET   ,     // char set
		OUT_TT_ONLY_PRECIS ,     // 出力精度
		ANTIALIASED_QUALITY,     // 出力品質
		FF_DONTCARE,             // フォントピッチとファミリ
		font_name,               // フォント名
		&_d3d_font               // フォントポインタ
		);

	if( FAILED(hr) ) return false;
	return true;
}


bool pxwDx09Draw::tex_glyph_init( int32_t t, const pxGLYPH_PARAM1 *p_prm, int32_t unit_num, bool b_UTF16LE )
{
	if( !_texs            ) return false;
	if( t >= _MAX_TEXTURE ) return false;

	pxwDx09TEXTURE *pt = &_texs[ t ];

	pt->glph = new pxwGlyph1();
	if( !pt->glph->init( p_prm, _screen_mag ) ) return false;

	if( !(  pt->glph_units = (pxwDx09GLYPHUNIT*)malloc( sizeof(pxwDx09GLYPHUNIT) * unit_num ) ) ) return false;
	memset( pt->glph_units, 0,                          sizeof(pxwDx09GLYPHUNIT) * unit_num );
	pt->glph_unit_num  = unit_num ;
	pt->glph_b_UTF16LE = b_UTF16LE;
	return true;
}

bool pxwDx09Draw::tex_glyph_another_font( int32_t t, const TCHAR* font_name, int32_t font_h, bool b_bold )
{
	if( !_texs            ) return false;
	if( t >= _MAX_TEXTURE ) return false;
	pxwDx09TEXTURE *pt = &_texs[ t ];
	if( !pt || !pt->glph  ) return false;

	return pt->glph->another_font( font_name, font_h, b_bold );
}

bool pxwDx09Draw::tex_glyph_color( int32_t t, uint32_t argb )
{
	if( !_texs            ) return false;
	if( t >= _MAX_TEXTURE ) return false;

	pxwDx09TEXTURE *pt = &_texs[ t ];
	if( !pt->glph ) return false;

	return pt->glph->color( argb );
}


void pxwDx09Draw::tex_glyph_clear( int32_t t )
{
	if( !_texs            ) return;
	if( t >= _MAX_TEXTURE ) return;
	pxwDx09TEXTURE *pt = &_texs[ t ];
	if( pt->glph_units ) memset( pt->glph_units, 0, sizeof(pxwDx09GLYPHUNIT) * pt->glph_unit_num );
}

bool pxwDx09Draw::tex_glyph_moji( int32_t t, uint32_t moji, const fRECT *p_rc_dst, fSIZE *p_size )
{
	bool b_ret = false;

	if( !_texs            ) return false;
	if( t >= _MAX_TEXTURE ) return false;
	if( !_texs[ t ].p_tex ) return false;

	pxwDx09TEXTURE*  pt     = &_texs[ t ];
	const pxSurface* p_surf = NULL;
	sRECT            rc_ofs = {0};

	if( !pt->glph->generate( moji, pt->glph_b_UTF16LE, &p_surf, &rc_ofs ) ) return false;

	{
		D3DLOCKED_RECT rc_locked; memset( &rc_locked, 0, sizeof(rc_locked) );
		RECT rc_dst;
		rc_dst.left   = (LONG)( p_rc_dst->l * _screen_mag + rc_ofs.l );
		rc_dst.right  = (LONG)( p_rc_dst->l * _screen_mag + rc_ofs.r );
		rc_dst.top    = (LONG)( p_rc_dst->t * _screen_mag + rc_ofs.t );
		rc_dst.bottom = (LONG)( p_rc_dst->t * _screen_mag + rc_ofs.b );

		HRESULT hr = pt->p_tex->LockRect( 0, &rc_locked, &rc_dst, 0 );

		uint32_t*       p_dst = (uint32_t*)rc_locked.pBits;
		const uint32_t* p_src = (uint32_t*)p_surf->get_buf_const();
		int32_t  dst_pitch = rc_locked.Pitch     / sizeof(uint32_t);
		int32_t  src_pitch = p_surf->get_pitch() / sizeof(uint32_t);
		int32_t  w         = rc_dst.right  - rc_dst.left;
		int32_t  h         = rc_dst.bottom - rc_dst.top ;
		uint32_t color_src, a, r, g, b;

		for( int32_t y = 0; y < h; y++ )
		{
			for( int32_t x = 0; x < w; x++ )
			{
				color_src = *(p_src + x + y * src_pitch );
				a = (color_src >> 24) & 0xff;
				b = (color_src >> 16) & 0xff;
				g = (color_src >>  8) & 0xff;
				r = (color_src >>  0) & 0xff;
				*(p_dst + x + y * dst_pitch) = (a<<24)|(b<<16)|(g<<8)|(r<<0);
			}
		}

		pt->p_tex->UnlockRect( 0 );
	}

	if( p_size )
	{
		p_size->w = (float)rc_ofs.r;
		p_size->h = (float)rc_ofs.b;
	}

	{
		pxwDx09GLYPHUNIT *pu = NULL;
		for( int32_t u = 0; u < pt->glph_unit_num; u++ )
		{
			if( !pt->glph_units[ u ].code ){ pu = &pt->glph_units[ u ]; break; }
		}
		if( !pu ) goto End;
		{
			pu->code = moji;
		}
	}

	b_ret = true;
End:

	return b_ret;
}

bool pxwDx09Draw::tex_glyph_text( int32_t t, const char *txt, const fRECT *p_rc_dst, int32_t gap_pix )
{
	float   x      = p_rc_dst->l;
	float   w      = 0;
	int32_t len    = strlen( txt );
	int32_t a      = 0;
	fRECT   rc_dst = *p_rc_dst;
	fSIZE   res_size;

	uint32_t code;

	while( a < len )
	{
		code = 0;
		if( pxStr_sjis_is_2byte( txt[ a ] ) )
		{
			if( a + 1 >= len ) return false;
			uint8_t* p_buf = (uint8_t*)(&txt[ a ]);
			code = (p_buf[ 0 ]<<8)|(p_buf[ 1 ]);
		}
		else
		{
			code = txt[ a ];
		}

		if( rc_dst.l >= rc_dst.r ) return false;
		if( !tex_glyph_moji( t, code, &rc_dst, &res_size ) ) return false;
		rc_dst.l += res_size.w / _screen_mag + gap_pix; // res_size is in magnified pixels, rc_dst is not

		if( pxStr_sjis_is_2byte( txt[ a ] ) ) a += 2;
		else                                  a += 1;
	}
	return true;
}


uint32_t  pxwDx09Draw::tex_load   ( const TCHAR *dir, const TCHAR *name, int32_t tx_idx, float stretch )
{
	return tex_load( dir, name, tx_idx, stretch, false );
}

uint32_t  pxwDx09Draw::tex_load   ( const TCHAR *dir, const TCHAR *name, int32_t tx_idx )
{
	return tex_load( dir, name, tx_idx,       1, false );
}

bool pxwDx09Draw::default_palette_set( const pxPalette* p_def_pal )
{
	if( !p_def_pal || !p_def_pal->get_color( 0, NULL ) ) return false;
	_def_pal = p_def_pal;
	return true;
}

uint32_t  pxwDx09Draw::tex_load( const TCHAR *dir, const TCHAR *name, int32_t tx_idx, float stretch, bool b_ignore_screen_mag )
{
	pxwDx09TEXTURE* pt   = NULL;
	pxDescriptor*   desc = NULL;

	if( tx_idx < 0 )
	{
		tx_idx = 1;
		for( tx_idx; tx_idx <  _MAX_TEXTURE; tx_idx++ ){ if( !_texs[ tx_idx ].p_tex ) break; }
	}
	if( tx_idx >= _MAX_TEXTURE ) return 0;

	pt = &_texs[ tx_idx ];

	if( pt->p_tex ) pt->p_tex->Release(); pt->p_tex = NULL;

	bool      b_ret = false;
	int       src_w, src_h, dst_w, dst_h;
	pxSurface surf;
	float     mag = (stretch ? stretch : 1) * (b_ignore_screen_mag ? 1 : _screen_mag);

	if( !_ref_file_data->open_r( &desc, dir, name, NULL ) ) goto term;

	if( !surf.png_read( desc, 0, 0, _def_pal ) ) goto term;

	surf.get_size( &src_w, &src_h );
	dst_w = (int32_t)( src_w * mag );
	dst_h = (int32_t)( src_h * mag );

	if( FAILED(D3DXCreateTexture( _device, dst_w, dst_h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pt->p_tex ) ) )
	{
		goto term;
	}

	{
		D3DLOCKED_RECT rc;
		pt->p_tex->LockRect( 0, &rc, NULL, 0 );

		DWORD    dst_pitch = rc.Pitch / sizeof(DWORD);
		int      src_pitch = src_w;

		const uint32_t* p_src_row = (uint32_t*)surf.get_buf_const();
		uint32_t*       p_dst_row = (uint32_t*)rc.pBits;

		if( mag >= 1.0f )
		{
			int32_t mag1 = (int32_t)mag;

			for( int src_y = 0; src_y < src_h; src_y++ )
			{
				for( int32_t str_y = 0; str_y < mag1; str_y++ )
				{
					const uint32_t* ps = p_src_row;
					uint32_t*       pd = p_dst_row;
					for( int src_x = 0; src_x < src_w; src_x++, ps++ )
					{
						for( int32_t str_x = 0; str_x < mag1; str_x++, pd++ )
						{
							const uint8_t* p = (const uint8_t*)ps; // rgba -> argb
							*pd = (*(p + 3)<<24) | (*(p + 0)<<16) | (*(p + 1)<<8) | (*(p + 2)<<0);
						}
					}
					p_dst_row += src_w * mag1;
				}
				p_src_row += src_w;
			}
		}
		else
		{
			goto term;
		}

		pt->p_tex->UnlockRect( 0 );
	}

	{
		TCHAR* p_name = pt->p_name;
		TCHAR* p_dir  = pt->p_dir ;
		if( name && !pxStrT_copy_allocate( &pt->p_name, name ) ) goto term;
		if( dir  && !pxStrT_copy_allocate( &pt->p_dir , dir  ) ) goto term;
		pxStrT_free( &p_name );
		pxStrT_free( &p_dir  );
	}

	pt->origin_img_w = src_w;
	pt->origin_img_h = src_h;
	pt->flags        = _TEXTUREFLAG_EXTERIOR; if( b_ignore_screen_mag ) pt->flags |= _TEXTUREFLAG_IGNORE_SCREEN_MAG;
	pt->stretch      = stretch;

	b_ret = true;

term:
	if( !b_ret ) tex_release( tx_idx );
	SAFE_DELETE( desc );

	return b_ret ? tx_idx : -1;
}


bool pxwDx09Draw::tex_blt   ( int32_t t, const uint32_t *p_src, int32_t src_w, int32_t src_h )
{
	if( t >= _MAX_TEXTURE || t < 0 ) return false;
	if( !_texs[ t ].p_tex          ) return false;

	pxwDx09TEXTURE *pt = &_texs[ t ];
	float strch = (pt->stretch?pt->stretch : 1);
	const int32_t src_pitch = src_w;
	int32_t       mag1      = (pt->flags & _TEXTUREFLAG_IGNORE_SCREEN_MAG) ? 1 : (int32_t)_screen_mag; if( mag1 < 1 ) mag1 = 1;

	if( src_w > (int32_t)( pt->origin_img_w * strch ) ) src_w = (int32_t)( pt->origin_img_w * strch );
	if( src_h > (int32_t)( pt->origin_img_h * strch ) ) src_h = (int32_t)( pt->origin_img_h * strch );

	D3DLOCKED_RECT rc;
	HRESULT hr = pt->p_tex->LockRect( 0, &rc, NULL, 0 );
	if( FAILED( hr ) ) return false;

	uint8_t *p_dst_line = (uint8_t*)rc.pBits;
	int32_t src, a, r, g, b;

	// the texture is held in magnified pixels: repeat each source pixel mag1 x mag1 times.
	for( int y = 0; y < src_h; y++ )
	{
		for( int mag_y = 0; mag_y < mag1; mag_y++ )
		{
			uint32_t *p_dst = (uint32_t*)p_dst_line;
			for( int x = 0; x < src_w; x++ )
			{
				src = p_src[ x + y * src_pitch ];
				a   = (src>>24) & 0xff;
				b   = (src>>16) & 0xff;
				g   = (src>> 8) & 0xff;
				r   = (src>> 0) & 0xff;
				for( int mag_x = 0; mag_x < mag1; mag_x++, p_dst++ ) *p_dst = (a<<24)|(r<<16)|(g<<8)|(b<<0);
			}
			p_dst_line += rc.Pitch;
		}
	}
	pt->p_tex->UnlockRect( 0 );
	return true;
}

void pxwDx09Draw::tex_release( int32_t t )
{
	if( !_texs || t >= _MAX_TEXTURE ) return;
	pxwDx09TEXTURE *pt = &_texs[ t ];

	if( !pt ) return;

	if( pt->p_tex ) pt->p_tex->Release(); pt->p_tex = NULL;
	_free( (void**)&pt->p_dir  );
	_free( (void**)&pt->p_name );

	if( pt->glph_units ) free( pt->glph_units ); pt->glph_units = NULL;
	SAFE_DELETE( pt->glph );
}

void pxwDx09Draw::tex_release()
{
	for( int i = 0; i < _MAX_TEXTURE; i++ ) tex_release( i );
}

void pxwDx09Draw::tex_Put_Clip( float x, float y, const fRECT *p_rc_src, int t, const fRECT *p_rc_clip )
{
	if( !_b_on_scene               ) return;
	if( t >= _MAX_TEXTURE || t < 0 ) return;
	if( !_texs[ t ].p_tex          ) return;

	fRECT rc_src ;
	fRECT rc_clip;
	float src_w  ;
	float src_h  ;

	rc_src  = *p_rc_src ;

	if( p_rc_clip ) rc_clip = *p_rc_clip;
	else            rc_clip = _view_clip;

	{
		float mag = (_texs[ t ].flags & _TEXTUREFLAG_IGNORE_SCREEN_MAG) ? 1 : _screen_mag;

		x         *= mag;
		y         *= mag;
		rc_src .l *= mag;
		rc_src .t *= mag;
		rc_src .r *= mag;
		rc_src .b *= mag;

		rc_clip.l *= _screen_mag;
		rc_clip.t *= _screen_mag;
		rc_clip.r *= _screen_mag;
		rc_clip.b *= _screen_mag;

		src_w = rc_src.w();
		src_h = rc_src.h();
	}

	if( x + src_w > rc_clip.r ){ src_w    -= x + src_w - rc_clip.r; rc_src.r = rc_src.l + src_w; } // R
	if( x < rc_clip.l         ){ rc_src.l += rc_clip.l - x; x = rc_clip.l;                       } // L
	if( y + src_h > rc_clip.b ){ src_h    -= y + src_h - rc_clip.b; rc_src.b = rc_src.t + src_h; } // B
	if( y < rc_clip.t         ){ rc_src.t += rc_clip.t - y; y = rc_clip.t ;                      } // T

	fRECT rc_dst;
	rc_dst.l = x;
	rc_dst.t = y;
	rc_dst.r = x + rc_src.r  - rc_src.l;
	rc_dst.b = y + rc_src.b - rc_src.t ;

	if( rc_src.r - rc_src.l <= 0 ) return;
	if( rc_src.b - rc_src.t <= 0 ) return;

	_sprite_begin();

	{
		D3DXVECTOR3 center(0,0,0);
		D3DXVECTOR3 pos( rc_dst.l, rc_dst.t, 0 );
		RECT rc;
		rc.left   = (LONG)rc_src.l;
		rc.top    = (LONG)rc_src.t;
		rc.right  = (LONG)rc_src.r;
		rc.bottom = (LONG)rc_src.b;
		_sprt->Draw( _texs[ t ].p_tex, &rc, &center, &pos, 0xffffffff);
	}
}

void pxwDx09Draw::tex_Put_View( float  x, float  y, const fRECT *p_rc  , int  t )
{
	x += _view_ofs_x;
	y += _view_ofs_y;

	tex_Put_Clip( x, y, p_rc, t, &_view_clip );
}


bool pxwDx09Draw::FillRect( const fRECT *p_rc, float a, float r, float g, float b )
{
	if( !_b_on_scene ) return false;

	_sprite_end();

	fRECT rc = *p_rc;

	rc.l += _view_ofs_x;
	rc.r += _view_ofs_x;
	rc.t += _view_ofs_y;
	rc.b += _view_ofs_y;

	DWORD clr =
		((DWORD)(255.f*a)) << 24 |
				((DWORD)(255.f*r)) << 16 |
				((DWORD)(255.f*g)) <<  8 |
				((DWORD)(255.f*b)) <<  0;

	const _VERTEXBOX vtxs[4] =
	{
		{ rc.l * _screen_mag, rc.t * _screen_mag, 0.f, 1.f, clr },
		{ rc.r * _screen_mag, rc.t * _screen_mag, 0.f, 1.f, clr },
		{ rc.l * _screen_mag, rc.b * _screen_mag, 0.f, 1.f, clr },
		{ rc.r * _screen_mag, rc.b * _screen_mag, 0.f, 1.f, clr }
	};
	_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE           ); //アルファブレンディングの有効化
	_device->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA    ); //SRCの設定
	_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ); //DESTの設定

	_device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vtxs, sizeof(vtxs[0]) );
	return true;
}

void pxwDx09Draw::FillView( float a, float r, float g, float b )
{
	fRECT rc;

	rc.l =            0;
	rc.r = _size_view.w;
	rc.t =            0;
	rc.b = _size_view.h;

	FillRect( &rc, a, r, g, b );
}

void pxwDx09Draw::Fill( float a, float r, float g, float b )
{
	FillRect( &_view_clip, a, r, g, b );
}

bool pxwDx09Draw::FillRect_clip( const fRECT *p_rc, float a, float r, float g, float b, const fRECT *rc_clip )
{
	fRECT rc = *p_rc;
	rc.l += _view_ofs_x;
	rc.r += _view_ofs_x;
	rc.t += _view_ofs_y;
	rc.b += _view_ofs_y;

	if( rc.l < rc_clip->l ) rc.l = rc_clip->l;
	if( rc.t < rc_clip->t ) rc.t = rc_clip->t;
	if( rc.r > rc_clip->r ) rc.r = rc_clip->r;
	if( rc.b > rc_clip->b ) rc.b = rc_clip->b;

	return FillRect( &rc, a, r, g, b );
}

bool pxwDx09Draw::FillRect_clip( const fRECT *p_rc, uint32_t aBGR, const fRECT *rc_clip )
{
	float a = (float)( (aBGR >> 24) & 0xff ) / 0xff;
	float b = (float)( (aBGR >> 16) & 0xff ) / 0xff;
	float g = (float)( (aBGR >>  8) & 0xff ) / 0xff;
	float r = (float)( (aBGR >>  0) & 0xff ) / 0xff;
	return FillRect_clip( p_rc, a, r, g, b,rc_clip );
}

bool pxwDx09Draw::FillRect_view( const fRECT *p_rc, float a, float r, float g, float b )
{
	return FillRect_clip( p_rc, a, r, g, b, &_view_clip );
}

bool pxwDx09Draw::FillRect_view( const fRECT *p_rc, uint32_t aBGR )
{
	return FillRect_clip( p_rc, aBGR, &_view_clip );
}

bool pxwDx09Draw::tex_GetSize_original( float *p_w, float *p_h, int32_t tx_idx )
{
	if( tx_idx >= _MAX_TEXTURE || tx_idx < 0 ) return 0;
	pxwDx09TEXTURE *pt = &_texs[ tx_idx ];
	if( !pt->p_tex ) return false;
	if( p_w ) *p_w = (float)pt->origin_img_w;
	if( p_h ) *p_h = (float)pt->origin_img_h;
	return true;
}

bool pxwDx09Draw::tex_GetSize_stretched( float *p_w, float *p_h, int32_t tx_idx )
{
	if( tx_idx >= _MAX_TEXTURE || tx_idx < 0 ) return 0;
	pxwDx09TEXTURE *pt = &_texs[ tx_idx ];
	if( !pt->p_tex ) return false;
	float strch = (pt->stretch?pt->stretch:1);
	if( p_w ) *p_w = (float)pt->origin_img_w * strch;
	if( p_h ) *p_h = (float)pt->origin_img_h * strch;
	return true;
}

#endif
