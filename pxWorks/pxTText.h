// '16/09/19 px-wTText_sjis.
// '16/09/19 px-wTText. (+UTF8 to T)
// '17/04/21 -> "pxTText".

#ifndef pxTText_H
#define pxTText_H

class pxTText
{
private:
const TCHAR* _p_text_t;

wchar_t*     _p_wide  ;
char   *     _p_sjis  ;

void _clear();

public:

pxTText();
~pxTText();

bool set_sjis_to_t    ( const char * text );
bool set_UTF8_to_t    ( const char * text );
bool set_TCHAR_to_sjis( const TCHAR* text );

const TCHAR* tchr() const;
const char*  sjis() const;
};

#endif

