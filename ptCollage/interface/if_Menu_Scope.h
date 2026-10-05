#include <pxStdDef.h>

enum enum_ScopeMenu
{
	enum_ScopeMenu_None =  0,
	enum_ScopeMenu_Copy     ,
	enum_ScopeMenu_Cut      ,
	enum_ScopeMenu_Clear    ,
	enum_ScopeMenu_Delete   ,
	enum_ScopeMenu_Paste    ,
	enum_ScopeMenu_Insert   ,
	enum_ScopeMenu_Transpose,
	enum_ScopeMenu_Scope    ,
};


void   if_Menu_Scope_Initialize ();
void   if_Menu_Scope_SetPosition( float x, float y, bool bCenter );
void   if_Menu_Scope_GetPosition( float *px, float *py );
void   if_Menu_Scope_SetRect    ( const fRECT *rc_view );
void   if_Menu_Scope_SetActive  ( bool bActive );
fRECT* if_Menu_Scope_GetRect  ();
bool   if_Menu_Scope_IsPanel(        float x, float y );
enum_ScopeMenu if_Menu_Scope_GetMenu( float x, float y );
void   if_Menu_Scope_Put();
