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

bool Open( const TCHAR *device_name, HWND hwnd, pxMIDIIN_CALLBACK func );
void Close();

bool Input_Start();
bool Input_Stop ();
bool Input_Reset();

};

#define pxMidiIn_KEY_NUM 0x100

unsigned char pxMidiIn_get_velo( unsigned char key );
void          pxMidiIn_ClearVelos();
bool CALLBACK pxMidiIn_Callback( HMIDIIN h, UINT msg, DWORD inst, LPARAM prm1, WPARAM prm2 );
