
//#include "./DebugLog.h"

#include <pxDebugLog.h>

#include "./pxMidiIn.h"

pxMidiIn::pxMidiIn()
{
	_h      = NULL;
	_dev_id =    0;
}

pxMidiIn::~pxMidiIn()
{
	pxMidiIn::Close();
}

/*

case MIM_OPEN:
case MIM_CLOSE:
case MIM_DATA     : prm1:midi-message / prm2:time-stamp
case MIM_LONGDATA : prm1:lp-midi-Hdr  / prm2:time-stamp
case MIM_ERROR    : prm1:midi-message / prm2:time-stamp
case MIM_LONGERROR: prm1:lp-midi-Hdr  / prm2:time-stamp
case MIM_MOREDATA : prm1:midi-message / prm2:time-stamp

*/
/*
MIM_DATA: wMsg=000003C3, p1=00004390, p2=000051E1
MIM_DATA: wMsg=000003C3, p1=00334390, p2=000052BE
MIM_DATA: wMsg=000003C3, p1=00004390, p2=00005374
MIM_DATA: wMsg=000003C3, p1=00073C90, p2=0000577A
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00005863
MIM_DATA: wMsg=000003C3, p1=00533C90, p2=00005C1B
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00005CE8
MIM_DATA: wMsg=000003C3, p1=005C3C90, p2=00005E6C
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00005F32
MIM_DATA: wMsg=000003C3, p1=00593C90, p2=000060CA
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=0000618C
MIM_DATA: wMsg=000003C3, p1=00523C90, p2=00006286
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00006CA8
MIM_DATA: wMsg=000003C3, p1=00553C90, p2=00006FF6
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=000072FE
MIM_DATA: wMsg=000003C3, p1=004F3C90, p2=000075ED
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00007AA4
MIM_DATA: wMsg=000003C3, p1=003B3C90, p2=000082A7
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00008396
MIM_DATA: wMsg=000003C3, p1=00463C90, p2=00008734
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00008823
MIM_DATA: wMsg=000003C3, p1=00333C90, p2=00008D73
MIM_DATA: wMsg=000003C3, p1=00443E90, p2=000096DE
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=00009D74
MIM_DATA: wMsg=000003C3, p1=00003E90, p2=00009D7D
MIM_DATA: wMsg=000003C3, p1=00453C90, p2=0000AD77
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=0000B198
MIM_DATA: wMsg=000003C3, p1=004F3C90, p2=0000B441
MIM_DATA: wMsg=000003C3, p1=00003C90, p2=0000B951
MIM_DATA: wMsg=000003C3, p1=00493C90, p2=0000C455*/

static void CALLBACK _testFunc( HMIDIIN h, UINT msg, DWORD_PTR inst, DWORD_PTR prm1, DWORD_PTR prm2 )
{
	MIDIHDR *hdr;
//	char str[ 256 ];

	switch( msg )
	{
	case MIM_OPEN :
		dlog_c( "MIDI device is opened." ); break;
	case MIM_CLOSE:
		dlog_c( "MIDI device is closed." ); break;
	case MIM_DATA :
		if( (prm1 & 0x000000ff) == 0xfe ) break;
		dlog_c( "MIM_DATA: wMsg=%08X, p1=%08X, p2=%08X", msg, prm1, prm2 );
		{
			unsigned char *buf = (unsigned char*)&prm1;
			dlog_c( "[0]:%02X, [1]:%02X, [2]:%02X, [3]:%02X", buf[ 0 ], buf[ 1 ], buf[ 2 ], buf[ 3 ] );
		}
		break;

	case MIM_LONGDATA:

		hdr = (MIDIHDR*)prm1;
		dlog_c( "MIM_LONGDATA: [%d bytes] *", hdr->dwBytesRecorded );

		for ( unsigned int i = 0; i<hdr->dwBytesRecorded; i++ )
		{
//			dlog( "%02X ", (unsigned char)((hdr->lpData)[i]) );
		}

		midiInPrepareHeader( h, hdr, sizeof(MIDIHDR) );
		midiInAddBuffer    ( h, hdr, sizeof(MIDIHDR) );
		break;

	case MIM_ERROR    :
	case MIM_LONGERROR:
	case MIM_MOREDATA :
	default:
	{
		dlog_c( "MidiInProc: wMsg=%08X, p1=%08X, p2=%08X\n", msg, prm1, prm2 );
		break;
	}
	}
}

bool pxMidiIn::Open( const TCHAR *device_name, HWND hwnd, pxMIDIIN_CALLBACK func )
{
	pxMidiIn::Close();

	if( !hwnd && !func ) func = reinterpret_cast<pxMIDIIN_CALLBACK>(_testFunc);

	int num = midiOutGetNumDevs();
	MIDIINCAPS caps;
	int  device_id;
	for( device_id = 0; device_id < num; device_id++ )
	{
		if( midiInGetDevCaps( device_id, &caps, sizeof(caps)) == MMSYSERR_NOERROR &&
			!_tcscmp( caps.szPname, device_name ) ) break;
	}
	if( device_id == num ) return false;

	int res;

	if( hwnd ) res = midiInOpen( &_h, device_id, (DWORD_PTR)hwnd, 0, CALLBACK_WINDOW   );
	else       res = midiInOpen( &_h, device_id, (DWORD_PTR)func, 0, CALLBACK_FUNCTION );

	if( res != MMSYSERR_NOERROR )
	{
		TCHAR errmsg[MAXERRORLENGTH];
		midiInGetErrorText(res, errmsg, MAXERRORLENGTH);
		_h = NULL;
		return false;
	}

	return true;
}

void pxMidiIn::Close()
{
	if( !_h ) return;
	midiInClose( _h );
	_h = NULL;
}



bool pxMidiIn::Input_Start()
{
	if( !_h ) return false;
	if( midiInStart( _h ) != MMSYSERR_NOERROR ) return false;
	return true;
}
bool pxMidiIn::Input_Stop ()
{
	if( !_h ) return false;
	if( midiInStop( _h ) != MMSYSERR_NOERROR ) return false;
	return true;
}
bool pxMidiIn::Input_Reset()
{
	if( !_h ) return false;
	if( midiInReset( _h ) != MMSYSERR_NOERROR ) return false;
	return true;
}












static bool          _b_init = false;
static unsigned char _velocities[ pxMidiIn_KEY_NUM ];


unsigned char pxMidiIn_get_velo( unsigned char key )
{
	if( !_b_init ) return 0;
	return _velocities[ key ];
}

void pxMidiIn_ClearVelos()
{
	memset( _velocities, 0, sizeof(_velocities ) );
}

bool CALLBACK pxMidiIn_Callback( HMIDIIN h, UINT msg, DWORD inst, LPARAM l, WPARAM w )
{
	MIDIHDR *hdr;

	switch( msg )
	{
	case MIM_OPEN : memset( _velocities, 0, sizeof(_velocities ) ); _b_init = true; dlog_c( "MIDI device is opened2." ); break;
	case MIM_CLOSE: memset( _velocities, 0, sizeof(_velocities ) );                 dlog_c( "MIDI device is closed2." ); break;

	case MIM_DATA :
		if( (l & 0x000000ff) == 0xfe ) break;
		dlog_c( "MIM_DATA: wMsg=%08X, p1=%08X, p2=%08X", msg, l, w );
		{
			unsigned char *buf = (unsigned char*)&l;
			dlog_c( "[0]:%02X, [1]:%02X, [2]:%02X, [3]:%02X", buf[ 0 ], buf[ 1 ], buf[ 2 ], buf[ 3 ] );;
			if( buf[ 0 ] == 0x90 ) _velocities[ buf[ 1 ] ] = buf[ 2 ];
			if( buf[ 0 ] == 0x80 ) _velocities[ buf[ 1 ] ] =      0  ;
		}
		break;

	case MIM_LONGDATA:

		hdr = (MIDIHDR*)l;
		dlog_c( "MIM_LONGDATA: [%d bytes] ** ", hdr->dwBytesRecorded );

		for ( unsigned int i = 0; i<hdr->dwBytesRecorded; i++ );// dlog( str, "%02X ", (unsigned char)((hdr->lpData)[i]) );

		midiInPrepareHeader( h, hdr, sizeof(MIDIHDR) );
		midiInAddBuffer    ( h, hdr, sizeof(MIDIHDR) );
		break;

	case MIM_ERROR    :
	case MIM_LONGERROR:
	case MIM_MOREDATA :
	default:
	{
		dlog_c( "MidiInProc: wMsg=%08X, p1=%08X, p2=%08X", msg, l, w );
		return false;
	}
	}
	return true;
}
