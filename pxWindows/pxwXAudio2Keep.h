// '17/01/19 pxwXAudio2Keep.


#ifndef pxwXAudio2Keep_H
#define pxwXAudio2Keep_H

#include <XAudio2.h>
#include <pxStdDef.h>

#include <string>

class pxwXAudio2Keep_loadlib // LoadLibraryVersion
{
public:
pxwXAudio2Keep_loadlib();
virtual ~pxwXAudio2Keep_loadlib();

    // Apply this workaround.
bool invoke( bool b_debug );

private:
    // Handle to loaded XAudio2 DLL.
HMODULE _h_lib;

    // Full path to theXAudio2 DLL.
std::basic_string<TCHAR> _path_dll;

bool _registry_find_dll_path( REFCLSID clsidXaudio2Dll );
bool _explicit_load  ();
bool _explicit_unload();
};




class pxwXAudio2Keep_getclass // CoGetClassObjectVersion
{
public:
pxwXAudio2Keep_getclass         ();
virtual ~pxwXAudio2Keep_getclass();

bool invoke( bool b_debug );

private:

IUnknown* _p_obj;
};

#endif
