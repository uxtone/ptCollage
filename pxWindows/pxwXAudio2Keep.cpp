
#include "./pxwXAudio2Keep.h"

pxwXAudio2Keep_loadlib::pxwXAudio2Keep_loadlib(void)
	:_h_lib(NULL)
{
}

pxwXAudio2Keep_loadlib::~pxwXAudio2Keep_loadlib(void)
{
	_explicit_unload();
}

// lol https://github.com/TASEmulators/desmume/blob/master/desmume/src/frontend/windows/directx/XAudio2.h
#define GARBONZOBEAN(className, y) \
		class DECLSPEC_UUID_WRAPPER(y) className; \
		EXTERN_C const GUID CLSID_##className

// XAudio 2.5 (August 2009 SDK)
//GARBONZOBEAN(XAudio2, 4c9b6dde-6809-46e6-a278-9b6a97588670);
//GARBONZOBEAN(XAudio2_Debug, 715bdd1a-aa82-436b-b0fa-6acea39bd0a1);

//class MIDL_INTERFACE("4c9b6dde-6809-46e6-a278-9b6a97588670")) XAudio2;

bool pxwXAudio2Keep_loadlib::invoke( bool b_debug )
{
//    REFCLSID clsid = b_debug ? __uuidof(XAudio2_Debug) : __uuidof(XAudio2);
//    if( !_registry_find_dll_path(clsid) ) return false;
//	return _explicit_load();
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// Use the XAudio2 CLSID to find the XAudio2 DLL path in the registry.
//
//////////////////////////////////////////////////////////////////////////

typedef std::basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > tstring;

bool pxwXAudio2Keep_loadlib::_registry_find_dll_path(REFCLSID clsidXaudio2Dll)
{
	// Construct a string representation of the registry key in which the
	// path to the desired XAudio2 COM DLL should be located.

	tstring subKey;
	{
		LPOLESTR szClsid = NULL;
		StringFromCLSID( clsidXaudio2Dll, &szClsid );
		subKey += _T("\\CLSID\\");
		#ifdef UNICODE
		subKey += szClsid;
		#else
		char clsid_a[64];
		WideCharToMultiByte( CP_ACP, 0, szClsid, -1, clsid_a, sizeof(clsid_a), NULL, NULL );
		subKey += clsid_a;
		#endif
		subKey += _T("\\InProcServer32");
		CoTaskMemFree(szClsid);
	}

	bool b_found = false; // assume the worst

	// Open the XAudio2 DLL's COM registry key.

	HKEY hKey = 0;

	if ( RegOpenKeyEx(HKEY_CLASSES_ROOT, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		DWORD err = GetLastError();
	}
	else
	{
		// Found the registry key. Now read its value: it should be the full
		// path the the XAudio2 COM DLL.

		TCHAR buf[ MAX_PATH ] = { 0 };
		DWORD bufSize = sizeof(buf);

		if( RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)buf, &bufSize) != ERROR_SUCCESS )
		{
			DWORD err = GetLastError();
		}
		else
		{
			b_found   = true;
			_path_dll = buf ; // Save the path for logging.
		}

		RegCloseKey( hKey );
	}

	return b_found;
}


//////////////////////////////////////////////////////////////////////////
//
// Forcibly increment the internal reference count in the XAudio2 DLL by
// explicitly loading it. This is a crude but effective way to prevent
// it being unloading by the COM run time.
//
//////////////////////////////////////////////////////////////////////////
bool pxwXAudio2Keep_loadlib::_explicit_load()
{
	_h_lib = LoadLibrary( _path_dll.c_str() );
	if( _h_lib == NULL ) return false;
	return true;
}


bool pxwXAudio2Keep_loadlib::_explicit_unload()
{
	if( !_h_lib                ) return false;
	if( !FreeLibrary( _h_lib ) ) return false;
	return true;
}


//////////////////////////////////////////////////////////////////////////

pxwXAudio2Keep_getclass::pxwXAudio2Keep_getclass()
	:_p_obj( NULL )
{
}

pxwXAudio2Keep_getclass::~pxwXAudio2Keep_getclass()
{
	if( _p_obj ) _p_obj->Release(); _p_obj = NULL;
}

bool pxwXAudio2Keep_getclass::invoke( bool b_debug )
{
//    REFCLSID clsid = b_debug ? __uuidof(XAudio2_Debug) : __uuidof(XAudio2);
	LPVOID   ppv   = NULL;
	//if( FAILED(CoGetClassObject(clsid, CLSCTX_INPROC_SERVER, NULL, __uuidof(IClassFactory), &ppv)) ) return false;
	_p_obj = static_cast<IUnknown*>( ppv );
	return true;
}
