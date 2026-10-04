

#include "ptConfig_Font.h"

                        //0123456789abcdef0123456789abcdef
static char *_code     = "ptConfig_Font_20120321          ";
static int32_t  _code_size = 32;

void ptConfig_Font::set_default()
{
	memset( name, 0, sizeof(name) );
}

ptConfig_Font::ptConfig_Font()
{
	ptConfig_Font::set_default();
}

bool ptConfig_Font::write( pxDescriptor* desc ) const
{
	bool b_ret = false;

	if( !desc->w_asfile( _code, sizeof(char), _code_size      ) ) goto End;
	if( !desc->w_asfile( name , sizeof(TCHAR), BUFSIZE_FONTNAME) ) goto End;

	b_ret = true;
End:
	return b_ret;
}

bool ptConfig_Font::read( pxDescriptor* desc )
{
	char code[64] = {};
	bool b_ret = false;

	if( !desc->r( code, sizeof(char), _code_size ) ) goto End;
	if( memcmp( code, _code,          _code_size ) ) goto End;
	if( !desc->r( name, sizeof(TCHAR), BUFSIZE_FONTNAME )      ) goto End;

	b_ret = true;
End:
	if( !b_ret ) set_default();

	return b_ret;
}
