// '17/01/21 ファイルを pxCSV2 で実装し直す。

#ifndef Menu_History_H
#define Menu_History_H

#include <pxStdDef.h>

#include <pxFile2.h>

bool Menu_History_init( HMENU hMenu, int32_t max_history, const UINT* idms, uint32_t idm_dummy, const pxFile2* file_profile );
void Menu_History_Release   ();
bool Menu_History_Load      ();
bool Menu_History_Save      ();
void Menu_History_Add       ( const TCHAR* path_new );
void Menu_History_Delete    ( uint32_t idm );
bool Menu_History_GetPath   ( uint32_t idm, TCHAR* path_dst );

#endif
