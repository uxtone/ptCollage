
enum enum_BuildTuneExit
{
	enum_BuildTuneExit_Success = 0,
	enum_BuildTuneExit_Error,
	enum_BuildTuneExit_Stop,
};

enum BUILDTUNESCOPE
{
	BUILDTUNESCOPE_TOPLAST = 0,
	BUILDTUNESCOPE_BYTIME,
};

typedef struct
{
	TCHAR              output_path[ MAX_PATH ];
	int32_t            ver          ;
	BUILDTUNESCOPE     scope        ;
	float              sec_playtime ;
	float              sec_extrafade;

	enum_BuildTuneExit result_exit  ;
	DWORD              result_count ;
	float              volume       ;
}
BUILDPROGRESSSTRUCT;

INT_PTR CALLBACK
dlg_BuildProgress(  HWND hDlg, UINT msg, WPARAM w, LPARAM l );
