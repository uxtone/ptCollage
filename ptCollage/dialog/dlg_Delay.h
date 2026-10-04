
#include <pxtnDelay.h>

typedef struct
{
	DELAYUNIT unit    ;
	float     freq    ;
	float     rate    ;
	int32_t       group   ;
	bool      b_delete;
}
EFFECTSTRUCT_DELAY;

INT_PTR CALLBACK
dlg_Delay_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l );
