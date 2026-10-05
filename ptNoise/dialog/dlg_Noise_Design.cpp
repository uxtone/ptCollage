
#include <pxtnPulse_NoiseBuilder.h>
extern pxtnPulse_NoiseBuilder *g_noise_bldr;

#include <pxwWindowRect.h>
#include <pxwFilePath.h>

#include <pxwPathDialog.h>
extern pxwPathDialog*          g_path_dlg_ptn;
extern pxwPathDialog*          g_path_dlg_wav;

#include "../../Generic/Japanese.h"

#include "../../pxtoneTool/pxtoneTool.h"

#include "../resource.h"
#include "../PcmTable.h"
#include "../NoiseTable.h"

#include "./dlg_History.h"


extern HINSTANCE   g_hInst;
extern HWND        g_hWnd_Main;
extern HWND        g_hDlg_Design;
extern TCHAR       g_dir_module[];
extern TCHAR*      g_default_material_folder;


static bool             _b_init    = false;
static const TCHAR*     _rect_name = _T("noise_design.rect");

static pxtnPulse_Noise* _p_noise = NULL ;
static SAMPLINGQUALITY  _quality = { 0 };
static int32_t          _u       =   0  ;

static const pxFile2*   _ref_file_profile = NULL;


void dlg_NoiseDesign_init( const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;
}

INT_PTR CALLBACK
dlg_NoiseDesign_Quality( HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_NoiseDesign_Copy(    HWND hDlg, UINT msg, WPARAM w, LPARAM l );

// タイトルバー表記
static void _SetTitle( const TCHAR *path )
{
	TCHAR str[ MAX_PATH ];
	TCHAR *p_name;
	TCHAR *empty = _T("-");

	if( !path || path[0] == '\0' ) p_name = empty;
	else                           p_name = PathFindFileName( path );

#ifdef NDEBUG
	_stprintf_s( str, MAX_PATH, _T("Design Noise [%s]")      , p_name );
#else
	_stprintf_s( str, MAX_PATH, _T("Design Noise Debug [%s]"), p_name );
#endif
	SetWindowText( g_hDlg_Design, str );
}

#include <png.h>
#define WIDTHBYTES(i) (((i) + 31) / 32 * 4)

HBITMAP test_png_bmp_LoadPNG( HWND hwnd, const TCHAR *path_src )
{
	bool b_ret = false;

	FILE            *fp  = NULL;
	HBITMAP         hbm  =    0;
	png_structp     png  = NULL;
	png_infop       info;
	png_uint_32     w, h, rowbytes;
	int             color_type, depth, widthbytes;
	double          gamma;
	BITMAPINFO      bi;
	BYTE            *pbBits = NULL;
	png_bytepp      row_pointers = NULL;

	if( !(fp   = _tfopen( path_src, _T("rb") ) ) ) goto End;
	if( !(png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL) ) ) goto End;
	if( !(info = png_create_info_struct(png) ) ) goto End;

	if( setjmp( png_jmpbuf( png ) ) ) goto End;
	if( setjmp( png_jmpbuf( png ) ) ) goto End;

	png_init_io  ( png, fp   );
	png_read_info( png, info );

	png_get_IHDR          ( png, info, &w, &h, &depth, &color_type, NULL, NULL, NULL );
	png_set_strip_16      ( png );
	png_set_gray_to_rgb   ( png );
	png_set_palette_to_rgb( png );
	png_set_bgr           ( png );
	png_set_packing       ( png );
	if( png_get_gAMA(png, info, &gamma) ) png_set_gamma( png, 2.2, gamma   );
	else                                  png_set_gamma( png, 2.2, 0.45455 );
	png_read_update_info  ( png, info);
	png_get_IHDR          ( png, info, &w, &h, &depth, &color_type, NULL, NULL, NULL );

	rowbytes     = png_get_rowbytes( png, info );
	row_pointers = (png_bytepp)malloc( h * sizeof(png_bytep));
	for( unsigned int y = 0; y < h; y++ ) row_pointers[ y ] = (png_bytep)png_malloc( png, rowbytes );

	png_read_image( png, row_pointers );
	png_read_end  ( png, NULL );
	if( fp ) fclose( fp ); fp = NULL;

	memset( &bi.bmiHeader, 0, sizeof(BITMAPINFOHEADER) );
	bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth    = w;
	bi.bmiHeader.biHeight   = h;
	bi.bmiHeader.biPlanes   = 1;
	bi.bmiHeader.biBitCount = depth * png_get_channels(png, info);

	{
		HDC hdc = GetDC( hwnd );
		if( !( hbm = CreateDIBSection( hdc, &bi, DIB_RGB_COLORS, (VOID **)&pbBits, NULL, 0) ) ) goto End;
	}

	widthbytes = WIDTHBYTES( w * bi.bmiHeader.biBitCount );

	for(unsigned int y = 0; y < h; y++)
	{
		memcpy( pbBits + y * widthbytes, row_pointers[ h - 1 - y ], rowbytes );
	}

	b_ret = true;
End:
	if( row_pointers )
	{
		for( unsigned int y = 0; y < h; y++ ) free( row_pointers[ y ] );
		free( row_pointers );
	}
	if( png ) png_destroy_read_struct( &png, &info, NULL );
	if( fp  ) fclose( fp );
	return hbm;
}

static void _InitDialog( HWND hDlg )
{
	_SetTitle( NULL );

	TCHAR *type_table[ pxWAVETYPE_num ] =
	{
		_T("None"), _T("Sine"), _T("Saw"), _T("Rect"), _T("Random"), _T("Saw2"), _T("Rect2"),
		_T("Triangle"), _T("random2"),
		_T("Rect-3"), _T("Rect-4"), _T("Rect-8"), _T("Rect-16"),
		_T("Saw-3"), _T("Saw-4"), _T("Saw-6"), _T("Saw-8"),
	};

	for( int i = 0; i < pxWAVETYPE_num; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_MAIN  , CB_ADDSTRING, 0, (LPARAM)type_table[ i ] );
	for( int i = 0; i < pxWAVETYPE_num; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_FREQ  , CB_ADDSTRING, 0, (LPARAM)type_table[ i ] );
	for( int i = 0; i < pxWAVETYPE_num; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_VOLUME, CB_ADDSTRING, 0, (LPARAM)type_table[ i ] );

	TC_ITEM item;
	TCHAR   name[ 32 ];
	HICON   hIcon;

	HIMAGELIST hImage;
	hImage = ImageList_Create( 16, 16, ILC_COLOR4 | ILC_MASK, 2, 1 );
	hIcon  = LoadIcon( g_hInst, _T("ICON_DISABLE") ); ImageList_AddIcon( hImage, hIcon ); DeleteObject( hIcon );
	hIcon  = LoadIcon( g_hInst, _T("ICON_ENABLE" ) ); ImageList_AddIcon( hImage, hIcon ); DeleteObject( hIcon );
	SendDlgItemMessage( hDlg, IDC_TAB, TCM_SETIMAGELIST, 0, (LPARAM)hImage );

	memset( &item, 0, sizeof(TC_ITEM) );
	item.cchTextMax = 10;
	item.pszText    = name;
	item.mask       = TCIF_TEXT | TCIF_IMAGE;

	for( int i = 0; i < _p_noise->get_unit_num(); i++ )
	{
		if( Japanese_Is() ) _stprintf_s( name, 32, _T("レイヤー %d"), i + 1 );
		else                _stprintf_s( name, 32, _T("layer %d"   ), i + 1 );
		item.iImage = _p_noise->get_unit( i )->bEnable ? 1 : 0;
		SendDlgItemMessage( hDlg, IDC_TAB, TCM_INSERTITEM, i, (LPARAM)&item );
	}

	if( Japanese_Is() ) SetWindowText( hDlg, _T("ピストンノイズ エディタ") );
}

static void _UpdateSec( HWND hDlg )
{
	int32_t smp_num;
	float   sec;
	TCHAR   str[ 10 ];

	smp_num = GetDlgItemInt( hDlg, IDC_SAMPLENUM, NULL, true );

	sec = (float)smp_num / _quality.sps;
	_stprintf_s( str, 10, _T("%0.2f"), sec );
	SetDlgItemText( hDlg, IDC_SEC, str );
}

static void _UpdateUnitEnable( HWND hDlg )
{
	TC_ITEM item;
	int32_t     u;
	memset( &item, 0, sizeof(TC_ITEM) );
	item.mask       = TCIF_IMAGE;

	for( u = 0; u < _p_noise->get_unit_num(); u++ )
	{
		item.iImage     = _p_noise->get_unit( u )->bEnable ? 1 : 0;
		SendDlgItemMessage( hDlg, IDC_TAB, TCM_SETITEM, u, (LPARAM)&item );
	}
}

static void _SetParameter( HWND hDlg, int32_t u )
{
	TCHAR str[ 128 ];

	const pxNOISEDESIGN_UNIT *p_unit = _p_noise->get_unit( u );
	if( !p_unit ) return;

	// envelope
	_stprintf_s( str, 128, _T("%0.3f"), (float)p_unit->enves[ 0 ].x / 1000 ); SetDlgItemText( hDlg, IDC_ENVELOPE_SEC_1, str );
	_stprintf_s( str, 128, _T("%0.3f"), (float)p_unit->enves[ 1 ].x / 1000 ); SetDlgItemText( hDlg, IDC_ENVELOPE_SEC_2, str );
	_stprintf_s( str, 128, _T("%0.3f"), (float)p_unit->enves[ 2 ].x / 1000 ); SetDlgItemText( hDlg, IDC_ENVELOPE_SEC_3, str );
	SetDlgItemInt( hDlg, IDC_ENVELOPE_VOL_1, p_unit->enves[ 0 ].y, true );
	SetDlgItemInt( hDlg, IDC_ENVELOPE_VOL_2, p_unit->enves[ 1 ].y, true );
	SetDlgItemInt( hDlg, IDC_ENVELOPE_VOL_3, p_unit->enves[ 2 ].y, true );

	// main
	SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_MAIN  , CB_SETCURSEL, p_unit->main.type, 0 );
	CheckDlgButton( hDlg, IDC_CHECK_REVERSE_MAIN, p_unit->main.b_rev );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->main.freq     ); SetDlgItemText( hDlg, IDC_FREQUENCY_MAIN  , str );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->main.volume   ); SetDlgItemText( hDlg, IDC_VOLUME_MAIN     , str );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->main.offset   ); SetDlgItemText( hDlg, IDC_OFFSET_MAIN     , str );

	// freq
	SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_FREQ  , CB_SETCURSEL, p_unit->freq.type, 0 );
	CheckDlgButton( hDlg, IDC_CHECK_REVERSE_FREQ, p_unit->freq.b_rev );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->freq.freq     ); SetDlgItemText( hDlg, IDC_FREQUENCY_FREQ  , str );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->freq.volume   ); SetDlgItemText( hDlg, IDC_VOLUME_FREQ     , str );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->freq.offset   ); SetDlgItemText( hDlg, IDC_OFFSET_FREQ     , str );

	// volume
	SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_VOLUME, CB_SETCURSEL, p_unit->volu.type, 0 );
	CheckDlgButton( hDlg, IDC_CHECK_REVERSE_VOLUME, p_unit->volu.b_rev );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->volu.freq   ); SetDlgItemText( hDlg, IDC_FREQUENCY_VOLUME, str );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->volu.volume ); SetDlgItemText( hDlg, IDC_VOLUME_VOLUME   , str );
	_stprintf_s( str, 128, _T("%0.1f"), p_unit->volu.offset ); SetDlgItemText( hDlg, IDC_OFFSET_VOLUME   , str );

	// sec
	SetDlgItemInt( hDlg, IDC_SAMPLENUM, _p_noise->get_smp_num_44k(), true );
	_UpdateSec( hDlg );

	_stprintf_s( str, 128, _T("%d"), p_unit->pan   ); SetDlgItemText( hDlg, IDC_PAN, str );
	_stprintf_s( str, 128, _T("%dch / %dHz / %dbit"), _quality.ch, _quality.sps, _quality.bps );
	SetDlgItemText( hDlg, IDC_QUALITY, str );

	CheckDlgButton( hDlg, IDC_CHECK_ENABLE, _p_noise->get_unit( _u )->bEnable );
}

static bool _GetInputParameter( HWND hDlg )
{
	TCHAR str[ 10 ];
	pxNOISEDESIGN_UNIT *p_unit;
	float f;

	if( !( p_unit = _p_noise->get_unit( _u ) ) ) return false;

	GetDlgItemText( hDlg, IDC_ENVELOPE_SEC_1, str, 10 ); f = (float)_ttof( str ); p_unit->enves[ 0 ].x = (int32_t)( f * 1000 + 0.5 );
	GetDlgItemText( hDlg, IDC_ENVELOPE_SEC_2, str, 10 ); f = (float)_ttof( str ); p_unit->enves[ 1 ].x = (int32_t)( f * 1000 + 0.5 );
	GetDlgItemText( hDlg, IDC_ENVELOPE_SEC_3, str, 10 ); f = (float)_ttof( str ); p_unit->enves[ 2 ].x = (int32_t)( f * 1000 + 0.5 );

	p_unit->enves[ 0 ].y = GetDlgItemInt( hDlg, IDC_ENVELOPE_VOL_1, NULL, true );
	p_unit->enves[ 1 ].y = GetDlgItemInt( hDlg, IDC_ENVELOPE_VOL_2, NULL, true );
	p_unit->enves[ 2 ].y = GetDlgItemInt( hDlg, IDC_ENVELOPE_VOL_3, NULL, true );

	// main
	p_unit->main.type     = (pxWAVETYPE)SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_MAIN    , CB_GETCURSEL, 0, 0 );
	p_unit->main.b_rev = IsDlgButtonChecked( hDlg, IDC_CHECK_REVERSE_MAIN ) ? true : false;
	GetDlgItemText( hDlg, IDC_FREQUENCY_MAIN  , str, 10 ); p_unit->main.freq   = (float)_ttof( str );
	GetDlgItemText( hDlg, IDC_VOLUME_MAIN     , str, 10 ); p_unit->main.volume = (float)_ttof( str );
	GetDlgItemText( hDlg, IDC_OFFSET_MAIN     , str, 10 ); p_unit->main.offset = (float)_ttof( str );

	// freq
	p_unit->freq.type     = (pxWAVETYPE)SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_FREQ    , CB_GETCURSEL, 0, 0 );
	p_unit->freq.b_rev = IsDlgButtonChecked( hDlg, IDC_CHECK_REVERSE_FREQ ) ? true : false;
	GetDlgItemText( hDlg, IDC_FREQUENCY_FREQ  , str, 10 ); p_unit->freq.freq   = (float)_ttof( str );
	GetDlgItemText( hDlg, IDC_VOLUME_FREQ     , str, 10 ); p_unit->freq.volume = (float)_ttof( str );
	GetDlgItemText( hDlg, IDC_OFFSET_FREQ     , str, 10 ); p_unit->freq.offset = (float)_ttof( str );

	// volume
	p_unit->volu.type     = (pxWAVETYPE)SendDlgItemMessage( hDlg, IDC_COMBO_TYPE_VOLUME, CB_GETCURSEL, 0, 0 );
	p_unit->volu.b_rev = IsDlgButtonChecked( hDlg, IDC_CHECK_REVERSE_VOLUME ) ? true : false;
	GetDlgItemText( hDlg, IDC_FREQUENCY_VOLUME, str, 10 ); p_unit->volu.freq   = (float)_ttof( str );
	GetDlgItemText( hDlg, IDC_VOLUME_VOLUME   , str, 10 ); p_unit->volu.volume = (float)_ttof( str );
	GetDlgItemText( hDlg, IDC_OFFSET_VOLUME   , str, 10 ); p_unit->volu.offset = (float)_ttof( str );

	_p_noise->set_smp_num_44k( GetDlgItemInt( hDlg, IDC_SAMPLENUM , NULL, true ) );
	p_unit->bEnable     = IsDlgButtonChecked( hDlg, IDC_CHECK_ENABLE ) ? true : false;

	GetDlgItemText( hDlg, IDC_PAN             , str, 10 ); p_unit->pan         = _ttoi( str );
	return true;
}

static void _IDM_QUALITY_DIALOG( HWND hDlg )
{
	TCHAR str[ 128 ];
	if( DialogBoxParam( g_hInst, _T("DLG_NOISE_QUALITY"), hDlg, dlg_NoiseDesign_Quality, (LPARAM)&_quality ) )
	{
		_stprintf_s( str, 128, _T("%dch / %dHz / %dbit"), _quality.ch, _quality.sps, _quality.bps );
		SetDlgItemText( hDlg, IDC_QUALITY, str );
		_UpdateSec( hDlg );
	}
}
static void _IDM_COPY( HWND hDlg )
{
	DialogBoxParam( g_hInst, _T("DLG_NOISE_COPY"), hDlg, dlg_NoiseDesign_Copy, (LPARAM)_u );
}

static bool _IDM_INITIALIZE( HWND hDlg )
{
	if( IDOK != MessageBox( hDlg, _T("initialize design parameters"), _T("Initialize"), MB_OKCANCEL ) ){
		return false;
	}

	_p_noise->Release();

	if( !_p_noise->Allocate( MAX_NOISETABLEUNIT, MAX_NOISETABLEENVELOPE ) )
	{
		MessageBox( hDlg, _T("allocate design"), _T("error"), MB_OK );
		return false;
	}
	_u = 0;
	SendDlgItemMessage( hDlg, IDC_TAB, TCM_SETCURFOCUS, _u, 0 );

	NoiseTable_SetDefault( _p_noise );

	_SetTitle( NULL );
	_SetParameter(     hDlg, _u );
	_UpdateUnitEnable( hDlg     );
	return true;
}



// I/O..
static bool _io_read( void* user, void* p_dst, int32_t size, int32_t num )
{
	if( fread( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}
static bool _io_write( void* user, const void* p_dst, int32_t size, int32_t num )
{
	if( fwrite( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}
static  bool _io_seek( void* user,       int   mode , int32_t size              )
{
	if( fseek( (FILE*)user, size, mode ) ) return false; return true;
}
static bool _io_pos( void* user, int32_t* p_pos )
{
	fpos_t sz{};
	if( fgetpos( (FILE*)user, &sz ) ) return false;
	*p_pos  = pxFPOS_OFFSET(sz);
	return true;
}


static bool _Load_Design( const TCHAR* path )
{
	PcmTable_Woice_Stop( true );

	_u = 0; SendDlgItemMessage( g_hDlg_Design, IDC_TAB, TCM_SETCURFOCUS, 0, 0 );

	pxtnPulse_Noise *p_ptn = new pxtnPulse_Noise( _io_read, _io_write, _io_seek, _io_pos );

	if( !_p_noise->Allocate( MAX_NOISETABLEUNIT, MAX_NOISETABLEENVELOPE ) )
	{
		MessageBox( g_hDlg_Design, _T("allocate noise"), _T("error"), MB_OK );
		return false;
	}

	FILE*fp = _tfopen( path, _T("rb") );
	if( !fp )
	{
		MessageBox( g_hDlg_Design, _T("can't open"), _T("error"), MB_OK );
		return false;
	}

	pxtnERR res = pxtnERR_VOID;
	if( (res = p_ptn->read( fp )) != pxtnOK )
	{
		fclose( fp );
		MessageBox( g_hDlg_Design, _T("load ptnoise error"), _T("error"), MB_OK );
		NoiseTable_SetDefault( _p_noise );
		_SetTitle( NULL );
		return false;
	}
	fclose( fp );

	for( int i = 0; i < p_ptn->get_unit_num(); i++ )
	{
		NoiseTable_CopyUnit( _p_noise->get_unit( i ), p_ptn->get_unit( i ) );
	}
	_p_noise->set_smp_num_44k( p_ptn->get_smp_num_44k() );
	_SetTitle( path );
	_SetParameter(     g_hDlg_Design, _u );
	_UpdateUnitEnable( g_hDlg_Design     );

	return true;
}

static bool _IDM_LOAD( HWND hDlg )
{
	static HEARSELECTDIALOGSTRUCT hear = {0};

	hear.visible_flags = HEARSELECTVISIBLE_PTN;

	if( g_path_dlg_ptn->get_last_path( hear.dir_default, MAX_PATH ) ) PathRemoveFileSpec( hear.dir_default );
	else                                                              _stprintf_s( hear.dir_default, MAX_PATH, _T("%s\\%s"), g_dir_module, g_default_material_folder );

	if( !pxtoneTool_HearSelect_Dialog( hDlg, &hear ) ) return true;
	g_path_dlg_ptn->set_loaded_path( hear.path_selected );
	_Load_Design( hear.path_selected );
	return true;
}

static bool _IDM_SAVE   ( HWND hDlg, bool b_as )
{
	bool  b_ret = false;
	TCHAR path_dst    [ MAX_PATH ] = {0};

	PcmTable_Woice_Stop( true );
	_GetInputParameter( hDlg );

	if( !g_path_dlg_ptn->entrust_save_path( hDlg, b_as, path_dst, _T("no name") ) ) return true;

	_p_noise->Fix();

	{
		FILE* fp = _tfopen( path_dst, _T("wb") ); if( !fp ) return false;
		if( !_p_noise->write( fp, NULL ) )
		{
			fclose( fp );
			MessageBox( hDlg, _T("save ptnoise error"), _T("error"), MB_OK );
			return false;
		}
		fclose( fp );
	}
	_SetTitle(                path_dst );

	return true;
}

void dlg_Noise_Design_CopyUnit( int32_t from, int32_t to )
{
	if( from < 0 || from >= MAX_NOISETABLEUNIT ||
		to   < 0 || to   >= MAX_NOISETABLEUNIT ) return;

	NoiseTable_CopyUnit( _p_noise->get_unit( to ), _p_noise->get_unit( from ) );

	SendDlgItemMessage( g_hDlg_Design, IDC_TAB, TCM_SETCURFOCUS, to, 0 );
	_SetParameter(      g_hDlg_Design, to );
	_UpdateUnitEnable(  g_hDlg_Design     );
	_u = to;
}

// ファイルドロップ(ウインドウ)
static bool _Function_WM_DROPFILES( HWND hWnd, WPARAM wParam )
{
	TCHAR path[ MAX_PATH ];
	HDROP hDrop;

	hDrop = (HDROP)wParam;

	if( DragQueryFile( hDrop, -1, NULL, 0 ) != 0 )
	{
		DragQueryFile( hDrop,  0, path, MAX_PATH );

		if( !_tcsicmp( PathFindExtension( path ), _T(".ptnoise") ) )
		{
			_Load_Design( path );
		}
	}

	DragFinish( hDrop );
	return true;
}

void dlg_Noise_Design_Push( pxtnPulse_Noise *p_ptn )
{
	for( int i = 0; i < p_ptn->get_unit_num(); i++ )
	{
		NoiseTable_CopyUnit( _p_noise->get_unit( i ), p_ptn->get_unit( i ) );
	}
	_p_noise->set_smp_num_44k( p_ptn->get_smp_num_44k() );
	_SetParameter(     g_hDlg_Design, _u );
	_UpdateUnitEnable( g_hDlg_Design     );
	PcmTable_BuildAndPlay( _p_noise, &_quality );
}

void dlg_Noise_Design_GetQuality( SAMPLINGQUALITY *p_q )
{
	*p_q = _quality;
}

static bool _init_window( HWND hdlg )
{
	if( _b_init ) return false;
	{
		HBITMAP hb = (HBITMAP)LoadImage( g_hInst, _T("IMG_PATTERN"), IMAGE_BITMAP, 256, 200, LR_DEFAULTCOLOR );
		HRESULT hr =  SendDlgItemMessage ( hdlg, IDC_BITMAP1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hb );
		hr = hr;
	}

	_p_noise = new pxtnPulse_Noise( _io_read, _io_write, _io_seek, _io_pos );

	NoiseTable_SetDefaultQuality( &_quality );

	if( !_p_noise->Allocate( MAX_NOISETABLEUNIT, MAX_NOISETABLEENVELOPE ) )
	{
		MessageBox ( hdlg, _T("allocate design"), _T("error"), MB_OK );
		PostMessage( hdlg, WM_COMMAND, IDCANCEL, 0 );
		goto term;
	}
	NoiseTable_SetDefault( _p_noise );
	_u = 0;


	_InitDialog(   hdlg );
	Japanese_DialogItem_Change(        hdlg   );
	Japanese_MenuItem_Change( GetMenu( hdlg ) );
	DrawMenuBar( hdlg );
	_SetParameter( hdlg, _u );

	DragAcceptFiles( hdlg, true );
	pxwWindowRect_load( hdlg, _rect_name, false, true );
	_b_init = true;
term:

	return _b_init;
}

//コールバック
INT_PTR CALLBACK
dlg_NoiseDesign_Design( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	case WM_INITDIALOG:

		if( !_init_window( hDlg ) ) MessageBox( hDlg, _T("initialize eror"), _T("pxtone Noise"), MB_ICONERROR );
		break;

	case WM_DESTROY:

		SAFE_DELETE( _p_noise );
		break;

	//ボタンクリック
	case WM_COMMAND:

		switch(LOWORD(w))
		{
		case IDC_BUILD:
		{
			if( !_GetInputParameter( hDlg ) ) break;
			_p_noise->Fix();
			dlg_History_Push( _p_noise );
			_SetParameter( hDlg, _u );
			PcmTable_BuildAndPlay( _p_noise, &_quality );
		}
		break;

		case IDCANCEL:
			_b_init       = false;
			g_hDlg_Design = NULL ;
			EndDialog( hDlg, false );
			break;

		case IDC_SAMPLENUM:
			if( _b_init && HIWORD( w ) == EN_CHANGE ) _UpdateSec( hDlg );
			break;

		case IDC_CHECK_ENABLE:
		{
			TC_ITEM item;
			memset( &item, 0, sizeof(TC_ITEM) );
			item.mask       = TCIF_IMAGE;
			item.iImage     = IsDlgButtonChecked( hDlg, IDC_CHECK_ENABLE ) ? 1 : 0;
			SendDlgItemMessage( hDlg, IDC_TAB, TCM_SETITEM, _u, (LPARAM)&item );
		}

		break;
		case IDM_QUALITY_DIALOG: _IDM_QUALITY_DIALOG( hDlg        ); break;
		case IDM_LOAD          : _IDM_LOAD(           hDlg        ); break;
		case IDM_SAVE          : _IDM_SAVE(           hDlg, false ); break;
		case IDM_SAVE_AS       : _IDM_SAVE(           hDlg, true  ); break;
		case IDM_INITIALIZE    : _IDM_INITIALIZE(     hDlg        ); break;
		case IDM_COPY          : _IDM_COPY(           hDlg        ); break;
		}
		break;

	case WM_NOTIFY:
		if( ( (NMHDR*)l )->idFrom == IDC_TAB && ( (NMHDR*)l )->code == TCN_SELCHANGE )
		{
			_GetInputParameter(      hDlg                                 );
			_u = SendDlgItemMessage( hDlg, IDC_TAB, TCM_GETCURFOCUS, 0, 0 );
			_SetParameter(           hDlg, _u                             );
		}
		break;

	case WM_DROPFILES:

		_Function_WM_DROPFILES( hDlg, w    );
		break;

	case WM_MOVE:
		pxwWindowRect_save( hDlg, _rect_name );
		break;

	default: return false;

	}
	return true;
}
