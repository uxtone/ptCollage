// '15/12/13 pxError.h
// '16/09/21 + pxerr_t().

#ifndef pxError_H
#define pxError_H

#include <pxStdDef.h>

bool        pxError_init       ();
void        pxError_reset      ();
void        pxError_release    ();
bool        pxError_is_error   ();
const char* pxError_get_message();
const char* pxError_get_date   ();

bool pxerr  (); // fatal error.
bool pxerr  ( const char *fmt, ... );
bool pxerr_t( const char* text1, const TCHAR* text_t );

#endif
