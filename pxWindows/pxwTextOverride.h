// '16/01/04 pxwTextOverride.

#ifndef pxwTextOverride_H
#define pxwTextOverride_H


#include <pxStdDef.h>

#include <pxCSV2.h>

// #include <pxTText.h>

class pxwTextOverride
{
private:
void operator = (const pxwTextOverride& src){}
pxwTextOverride (const pxwTextOverride& src){}

uxDS   _inv_name;
pxCSV2* _csv     ;

void _release();

bool                 _update_title     ( HWND hwnd   );
bool                 _update_combo_box ( HWND h_ctrl );
bool                 _override_callback( HWND h_ctrl );
static BOOL CALLBACK _override_sttc    ( HWND h_ctrl, LPARAM lp );

bool _find_original_to_tt( uxDS& tt_ovr, const uxDS& t_src );
bool _override_menu      ( HMENU hmenu );

public:
pxwTextOverride();
~pxwTextOverride();

bool override_dialog( HWND  hdlg , pxDescriptor* desc, bool b_UTF8 );
bool override_menu  ( HMENU hmenu, pxDescriptor* desc, bool b_UTF8 );

const uxDS& get_inv_name() const;
};

#endif
