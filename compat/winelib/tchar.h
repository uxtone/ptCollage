#pragma once
#include_next <tchar.h>

#ifdef __cplusplus

#include <pxWinelibFile.h>
#undef _tfopen
#define _tfopen pxwl_fopen

#include <pxWinelibWide.h>

#endif // __cplusplus