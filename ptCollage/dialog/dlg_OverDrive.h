
typedef struct
{
	float cut  ;
	float amp  ;
	int32_t   group;

	bool  b_delete;
}
EFFECTSTRUCT_OVERDRIVE;

INT_PTR CALLBACK
dlg_OverDrive_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l );
