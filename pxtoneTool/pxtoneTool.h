#ifdef PXTONETOOL_STATIC
#define DLLAPI                       // static library.
#elif defined PXTONETOOL_EXPORTS
#define DLLAPI __declspec(dllexport) // DLL for making.
#else
#define DLLAPI __declspec(dllimport) // DLL for using.
#endif

#include <pxStdDef.h>

#include <pxFile2.h>
#include <pxtonewinWoice.h>

#include "HearSelect.h"


bool DLLAPI pxtoneTool_HearSelect_Initialize( pxtonewinWoice* woi, pxFile2* app_file_profile );
bool DLLAPI pxtoneTool_HearSelect_Dialog    ( HWND hWnd, HEARSELECTDIALOGSTRUCT* hear );
