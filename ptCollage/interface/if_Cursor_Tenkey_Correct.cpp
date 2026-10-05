
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include "../../Generic/KeyControl.h"

#include "../UndoEvent.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_Tenkey_Tuning.h"


extern HWND g_hWnd_Main;

static bool _Action_Free( float cur_x, float cur_y )
{
	fRECT *prcFrame;

	if( !if_Tenkey_Tuning_IsFrame( cur_x, cur_y ) ){ if_Tenkey_Tuning_Close(); return false; }

	prcFrame = if_Tenkey_Tuning_GetRect();

	// 左クリック
	if( KeyControl_IsClickLeftTrigger() ){
		if( if_Tenkey_Tuning_ButtonDown( cur_x, cur_y ) )
		{
			g_cursor.action = 1;
			g_cursor.focus  = ifCurFocus_Tenkey_Tuning;
		}
	}

	return true;
}

static bool _Action_Drag( float cur_x, float cur_y )
{

	bool bDrag = false;

	g_cursor.focus  = ifCurFocus_Tenkey_Tuning;

	//ドラッグキャンセル
	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() )
	{
		if_Tenkey_Tuning_ButtonCancel();
		g_cursor.action = 0;
		return true;
	}

	if( !KeyControl_IsClickLeft() )
	{
		if( if_Tenkey_Tuning_ButtonUp() )
		{
			float f;
			int32_t   clock;
			int32_t   u;

			if( if_Tenkey_Tuning_GetResult( &f, &u, &clock ) )
			{
				int32_t count= 0;
				if( u < g_pxtn->Unit_Num() )
				{
					// アンドゥ
					UndoEvent_SetOrderClock( clock, clock );
					UndoEvent_SetOrderUnit( u       );
					UndoEvent_Push( true );

					count += g_pxtn->evels->Record_Add_f( clock, (unsigned char)u, EVENTKIND_TUNING, f );

					if( count < 1 ) UndoEvent_Back();
					else            UndoEvent_ReleaseRedo();
				}

			}
			if_Tenkey_Tuning_Close();
		}
		g_cursor.action = 0;
		return true;
	}

	return true;
}

// 音符編集カーソル
bool if_Cursor_Action_Tenkey_Tuning( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action ){
	case 0: b_ret = _Action_Free( cur_x, cur_y ); break;
	case 1: b_ret = _Action_Drag( cur_x, cur_y ); break;
	}

	return b_ret;
}
