// '14/01/08 pxDateTime.cpp

#ifndef pxSTDAFX_H
#include <time.h>
#include <string.h>
#endif


#include "./pxDateTime.h"

static uint16_t _thousands = 2000;

void pxDateTime_init( uint16_t thousands )
{
	_thousands = thousands;
}

double pxDateTime_compare_seconds( const pxDATETIME *p1, const pxDATETIME *p2 )
{
	time_t    tt1, tt2;
	struct tm ts;

	memset( &ts, 0, sizeof(ts) );

	ts.tm_year = p1->Y + 100 - _thousands;
	ts.tm_mon  = p1->M -    1;
	ts.tm_mday = p1->D;
	ts.tm_hour = p1->h;
	ts.tm_min  = p1->m;
	ts.tm_sec  = p1->s; tt1 = mktime( &ts );

	ts.tm_year = p2->Y + 100 - _thousands;
	ts.tm_mon  = p2->M -    1;
	ts.tm_mday = p2->D;
	ts.tm_hour = p2->h;
	ts.tm_min  = p2->m;
	ts.tm_sec  = p2->s; tt2 = mktime( &ts );

	return difftime( tt1, tt2 );
}

double pxDateTime_compare_days( const pxDATETIME *p1, const pxDATETIME *p2 )
{
	double diff_sec = pxDateTime_compare_seconds( p1, p2 );
	return diff_sec / (60*60*24);
}


bool pxDateTime_get_now( pxDATETIME *p_datetime )
{
	struct tm t ;
	time_t    tt; time( &tt );

	memcpy( &t, localtime( &tt ), sizeof(t) );

	p_datetime->Y = t.tm_year - 100 + _thousands;
	p_datetime->M = t.tm_mon  +   1;
	p_datetime->D = t.tm_mday      ;
	p_datetime->h = t.tm_hour      ;
	p_datetime->m = t.tm_min       ;
	p_datetime->s = t.tm_sec       ;
	return true;
}
