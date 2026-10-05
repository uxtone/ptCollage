
#include <pxtnService.h>
#include <uxStr.h>
extern pxtnService* g_pxtn;

#include <pxwPathDialog.h>
extern pxwPathDialog* g_path_dlg_tune;

#include "../Generic/Menu_History.h"
#include "../Generic/Japanese.h"

#include "interface/if_Player.h"
#include "interface/Interface.h"

extern uxSS<32> g_app_name;

bool         _b_valid_tune = false;

void MainWindow_SetTitle( const uxDS& path );
bool InquireOperation();


bool Tune_LoadAndPlay( HWND hwnd, const uxDS& path )
{
	if( !InquireOperation() ) return false;

	bool    b_ret     = false       ;
	pxtnERR res       = pxtnERR_VOID;
	int     event_num =            0;

	SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	_b_valid_tune = false;
	if_Player_RedrawName( NULL );
	if_Player_ZeroSampleOffset();
	MainWindow_SetTitle( NULL );

	FILE* fp = ux_fopen( path, "rb" ); if( !fp ) goto term;
	res = g_pxtn->read( fp );
	if( res != pxtnOK ) goto term;
	fclose( fp ); fp = NULL;

	res = g_pxtn->tones_ready();
	if( res != pxtnOK ) goto term;

	if_Player_StartPlay();
	MainWindow_SetTitle( path );
	Menu_History_Add(   path );

	g_path_dlg_tune->set_loaded_path( path );
	g_path_dlg_tune->save_lasts();

	_b_valid_tune = true;
	b_ret         = true;
term:
	if( fp ) fclose( fp );

	if( !b_ret )
	{
		_b_valid_tune = false;
		g_path_dlg_tune->last_filename_clear();
		MessageBoxA( hwnd, pxtnError_get_string( res ), "pxtone Player", MB_OK );
	}
	SetCursor( LoadCursor( NULL, IDC_ARROW ) );

	return b_ret;
}

bool Tune_is_valid_data()
{
	return _b_valid_tune;
}

bool Tune_SelectAndPlay( HWND hwnd )
{
	uxDS path_get;
	if( !g_path_dlg_tune->dialog_load( hwnd, path_get ) ) return false;
	if( !Tune_LoadAndPlay            ( hwnd, path_get ) ) return false;
	Interface_Process( hwnd, true );
	return true;
}

bool Tune_IsComment()
{
	if( g_pxtn->text->is_comment_buf() ) return true;
	return false;
}



#include <process.h>

typedef struct
{
	const char *title  ;
	char *comment;
	HWND hwnd;
}
_COMMENTBOX;

static unsigned __stdcall _thrd_Comment( void *p_ )
{
	_COMMENTBOX *p = (_COMMENTBOX*)p_;
	MessageBoxA( p->hwnd, p->comment, p->title, MB_OK );

	free( p->comment );
	free( p );
	_endthreadex( 0 );

	return 1;
}

void Tune_PutComment( HWND hWnd )
{
	const char* cmnt = g_pxtn->text->get_comment_buf( NULL );
	if( !cmnt ) return;
	int len = strlen( cmnt ) + 1;

	if( len == 1 ) return;

	_COMMENTBOX *p_cmntbox = (_COMMENTBOX*)malloc( sizeof(_COMMENTBOX) );

	if( !(  p_cmntbox->comment = (char*)malloc( len ) ) ){ free( p_cmntbox ); return; }

	strcpy( p_cmntbox->comment, cmnt );

	p_cmntbox->title = Japanese_Is() ? "コメント" : "Comment";
	p_cmntbox->hwnd  = hWnd;

	unsigned int id;
	_beginthreadex( NULL, 0, _thrd_Comment, (LPVOID)p_cmntbox, 0, &id );
}
