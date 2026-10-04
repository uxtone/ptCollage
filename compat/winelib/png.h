#pragma once
// libpng is a native Linux library here so we need to hide _WIN32 otherwise
// pngconf.h marks its API and callbacks __cdecl (using ms_abi under Winelib)
#pragma push_macro("_WIN32")
#pragma push_macro("WIN32")
#pragma push_macro("__WIN32__")
#pragma push_macro("_WINDOWS")
#undef _WIN32
#undef WIN32
#undef __WIN32__
#undef _WINDOWS
#include_next <png.h>
#pragma pop_macro("_WINDOWS")
#pragma pop_macro("__WIN32__")
#pragma pop_macro("WIN32")
#pragma pop_macro("_WIN32")
