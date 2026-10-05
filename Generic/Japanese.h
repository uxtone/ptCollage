#include <pxStdDef.h>
#pragma once

typedef struct
{
	int          idc     ;
	const char* p_text_j;
	int32_t      wid     ;

}JAPANESETEXTSTRUCT_DLGITEM;

typedef struct
{
	const char* p_text_e;
	const char* p_text_j;

}JAPANESETEXTSTRUCT_TEXTSET;

void Japanese_Release();

bool Japanese_DialogItem_SetTable( JAPANESETEXTSTRUCT_DLGITEM* table, int num );
bool Japanese_MenuItem_SetTable  ( JAPANESETEXTSTRUCT_TEXTSET* table, int num );
bool Japanese_Message_SetTable   ( JAPANESETEXTSTRUCT_TEXTSET* table, int num );
void Japanese_DialogItem_Change  ( HWND  hWnd  );
void Japanese_MenuItem_Change    ( HMENU hMenu );
void Japanese_MessageBox         ( HWND hWnd, const uxDS& message, const uxDS& title, UINT uType );
bool Japanese_Is                 ();
void Japanese_Set                ( bool b );
