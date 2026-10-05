// '11/11/25

#define _CRT_SECURE_NO_WARNINGS

#include <pxStdDef.h>

#ifdef _WIN32

#include <pxwAppEtc.h>

#elif defined pxSCE

#include <pxsce.h>

#else

#include <pxmApp.h>
#include <string.h>
#include <stdio.h>

#endif

void pxwrAppEtc_Version( char *p_str )
{
#ifdef pxPLATFORM_windows

	int v1, v2, v3;
	pxwAppEtc_version( &v1, &v2, &v3, NULL );
	if( !v1 && !v2 && !v3 ) sprintf( p_str, "-.--" );
	else                    sprintf( p_str, "%d.%d%d", v1, v2, v3 );

#elif defined pxSCE
	int32_t v  = 0;
	int32_t v1 = 0;
	int32_t v2 = 0;
	int32_t v3 = 0;
	int32_t v4 = 0;

	if( pxsce_get_params( pxSCE_USERPARAM_master_ver, &v ) )
	{
		v1 = v / 1000; v -= v1 * 1000;
		v2 = v /  100; v -= v2 *  100;
		v3 = v /   10; v -= v3 *   10;
		v4 = v /    1;
		sprintf( p_str, "%d.%d%d", v2, v3, v4 );
	}
	else
	{
		sprintf( p_str, "-.--" );
	}
#else
	pxmApp_Version( p_str );
#endif

#ifndef pxRELEASE
	strcat( p_str, " d" );
#endif
}

int   pxwrAppEtc_Local()
{
#ifdef pxPLATFORM_windows
	return pxwAppEtc_Local();
#elif defined pxSCE
	return pxsce_get_lang ();
#else
	return pxmApp_Local   ();
#endif
}

void pxwrAppEtc_Sleep( float sec )
{
#ifdef pxPLATFORM_windows
	pxwAppEtc_sleep( sec );
#elif defined pxSCE
	pxsce_sleep    ( sec );
#else
	pxmApp_Sleep   ( (int32_t)(sec*1000) );
#endif
}

