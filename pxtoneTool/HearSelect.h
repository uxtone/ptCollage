#ifndef HearSelect_H
#define HearSelect_H


#include <pxStdDef.h>

#include <stddef.h>

#define HEARSELECTVISIBLE_PCM      0x01
#define HEARSELECTVISIBLE_PTV      0x02
#define HEARSELECTVISIBLE_PTN      0x04
#define HEARSELECTVISIBLE_OGGV     0x08
#define HEARSELECTVISIBLE_ADDUNIT  0x10

#define HEARSELECTSTATUS_LOOP      0x01
#define HEARSELECTSTATUS_SORT_TYPE 0x02
#define HEARSELECTSTATUS_ADDUNIT   0x04

typedef struct
{
	int   visible_flags;
	uxDS  path_selected;
	uxDS  dir_default  ;
	bool  b_add_unit   ;
	bool  b_japanese   ;
	bool  b_loop       ;
}
HEARSELECTDIALOGSTRUCT;

#endif
