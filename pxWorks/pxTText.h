// '16/09/19 px-wTText_sjis.
// '16/09/19 px-wTText. (+UTF8 to T)
// '17/04/21 -> "pxTText".

#ifndef pxTText_H
#define pxTText_H


#include <pxStdDef.h>

class pxTText
{
private:
const uxDS  _p_text_t;

wchar_t*     _p_wide  ;
char   *     _p_sjis  ;

void _clear();

public:

pxTText();
~pxTText();

bool set_sjis_to_t    ( const char * text );
bool set_UTF8_to_t    ( const char * text );
bool set_TCHAR_to_sjis( const uxDS& text );

const uxDS  str() const;
const char*  sjis() const;
};

#endif
