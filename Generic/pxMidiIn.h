#include <pxStdDef.h>
#pragma once


typedef bool (* pxMIDIIN_CALLBACK )( HMIDIIN h, UINT msg, DWORD inst, LPARAM prm1, WPARAM prm2 );

class pxMidiIn
{
private:

HMIDIIN _h;
int     _dev_id;


public:
pxMidiIn();
~pxMidiIn();

bool Open( const uxDS& device_name, HWND hwnd, pxMIDIIN_CALLBACK func );
void Close();

bool Input_Start();
bool Input_Stop ();
bool Input_Reset();

};

// Number of MIDI input devices. Wine's winmm loads its MIDI driver on the first MIDI call, and on a host
// where the ALSA sequencer cannot be opened that call never returns. The first call therefore runs on a
// worker thread with a timeout; if the driver does not answer, MIDI is reported as absent (0 devices) for
// the rest of the session and every caller carries on without it.
int pxMidiIn_get_device_num();

#define pxMidiIn_KEY_NUM 0x100

unsigned char pxMidiIn_get_velo( unsigned char key );
void          pxMidiIn_ClearVelos();
bool CALLBACK pxMidiIn_Callback( HMIDIIN h, UINT msg, DWORD inst, LPARAM prm1, WPARAM prm2 );
