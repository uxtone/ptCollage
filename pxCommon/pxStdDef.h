// pxStdDef.h '15/12/13

#ifndef pxStdDef_H
#define pxStdDef_H

#ifndef _TCHAR_DEFINED
typedef char TCHAR;
#define _T(x) x
#define _tfopen fopen
#endif

#define pxINCLUDE_OGGVORBIS 1

#ifdef _WIN32
#define pxwDX09 // directX version 09 or 11
#include <cstdint>

#else
#include <stdint.h>
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if( p ){ delete( p ); p = NULL; } }
#endif

#define pxMOUSEBIT_L 0x01
#define pxMOUSEBIT_R 0x02

#include <stdio.h>

#ifdef pxSCE
#define pxFPOS_OFFSET(p) static_cast<int32_t>((p)._Off)
#elif defined(__GLIBC__)
#define pxFPOS_OFFSET(p) static_cast<int32_t>((p).__pos)
#else
#define pxFPOS_OFFSET(p) static_cast<int32_t>(p)
#endif


enum pxALIGN
{
	pxALIGN_Left  ,
	pxALIGN_Right ,
	pxALIGN_Center,
};

typedef intptr_t  sPtr;
typedef uintptr_t uPtr;


typedef struct
{
	float l;
	float t;
	float r;
	float b;
	float w()const{ return r - l; }
	float h()const{ return b - t; }
}
fRECT;

typedef struct
{
	int32_t l;
	int32_t t;
	int32_t r;
	int32_t b;
	int32_t w()const{ return r - l; }
	int32_t h()const{ return b - t; }
}
sRECT;

typedef struct
{
	uint32_t l;
	uint32_t t;
	uint32_t r;
	uint32_t b;
	uint32_t w()const{ return r - l; }
	uint32_t h()const{ return b - t; }
}
uRECT;


typedef struct
{
	float x;
	float y;
}
fPOINT;

typedef struct
{
    int32_t x;
    int32_t y;
}
sPOINT;

typedef struct
{
    int32_t w;
    int32_t h;
}
sSIZE;

typedef struct
{
    float w;
    float h;
}
fSIZE;

typedef struct
{
	float a;
	float r;
	float g;
	float b;
}
fCOLOR;

typedef bool ( *pxfunc_sample_pcm )( void *user, void *buf, int *p_res_size, int *p_req_size );
typedef bool ( *pxfunc_find_path  )( void *user, const TCHAR *path_find );

enum pxGLYPH_TYPE
{
	pxGLYPH_none = 0,
	pxGLYPH_mono,
	pxGLYPH_gray,
};


typedef struct
{
	uint32_t     grid_x         ;
	uint32_t     grid_y         ;
	pxGLYPH_TYPE type           ;
	TCHAR        font_name[ 64 ];
	int32_t      font_h         ;
	uint32_t     font_argb      ;
	bool         b_font_bold    ;
}
pxGLYPH_PARAM1;


#define pxBUFSIZE_RESOURCENAME 32
#define pxBUFSIZE_EXTENSION    32
#define pxBUFSIZE_RESOURCEFILE (pxBUFSIZE_RESOURCENAME+pxBUFSIZE_EXTENSION + 1)

#define pxBUFSIZE_PATH        260
#define pxBITPERSAMPLE16       16

#endif
