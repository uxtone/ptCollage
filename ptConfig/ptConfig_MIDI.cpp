
#include "./ptConfig_MIDI.h"

                        //0123456789abcdef0123456789abcdef
static char *_code     = "ptConfig_MIDI_20170326a         "; // + key_tuning
static char *_code_old = "ptConfig_MIDI_20120329          ";
static int32_t  _code_size = 32;

ptConfig_MIDI::ptConfig_MIDI()
{
	ptConfig_MIDI::set_default();
}

void ptConfig_MIDI::set_default()
{
	memset( name, 0, sizeof(name) );
	b_velo     = true;
	key_tuning = 1.0f;
}

bool ptConfig_MIDI::write( pxDescriptor *desc ) const
{
	bool b_ret = false;

	if( !desc->w_asfile( _code      , sizeof(char),  _code_size ) ) goto End;
	if( !desc->w_asfile( name       , sizeof(char), BUFSIZE_MIDIDEVICENAME ) ) goto End;
	if( !desc->w_asfile( &b_velo    , sizeof(b_velo    ), 1 ) ) goto End;
	if( !desc->w_asfile( &key_tuning, sizeof(key_tuning), 1 ) ) goto End;

	b_ret = true;
End:
	return b_ret;
}

bool ptConfig_MIDI::read( pxDescriptor *desc )
{
	bool b_ret    = false;
	bool b_old    = false;
	char code[64] = { 0 };

	if( !desc->r(  code, sizeof(char), _code_size ) ) goto End;
	if( memcmp( code, _code,            _code_size     ) )
	{
		if( memcmp( code, _code_old, _code_size ) ) goto End;
		b_old = true;
	}

	if( !desc->r( name, sizeof(char), BUFSIZE_MIDIDEVICENAME ) ) goto End;
	if( !desc->r( &b_velo, sizeof(b_velo), 1      ) ) goto End;
	if( b_old ) key_tuning = 0;
	else if( !desc->r( &key_tuning, sizeof(key_tuning), 1 ) ) goto End;

	b_ret = true;
End:
	if( !b_ret ) ptConfig_MIDI::set_default();
	return b_ret;
}
