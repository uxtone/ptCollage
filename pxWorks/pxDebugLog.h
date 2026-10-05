// '16/01/25 pxDebugLog.h

#include <pxStdDef.h>

bool pxDebugLog_init( const uxDS& base_dir_name, const uxDS& log_dir_name );
void pxDebugLog_release();
bool pxDebugLog_GetCurrentPath( uxDS& path_dst );
int  pxDebugLog_DeleteOlds( int days );

void dlog_c( const char *fmt, ... );
void dlog_t( const char *text1, const uxDS& text_t );
