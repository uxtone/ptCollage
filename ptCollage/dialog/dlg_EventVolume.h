enum enum_EventKind
{
	enum_EventKind_Key     ,
	enum_EventKind_Velocity,
	enum_EventKind_TimePan ,
	enum_EventKind_VolPan  ,
	enum_EventKind_Volume  ,
};

INT_PTR CALLBACK
dlg_EventVolume( HWND hDlg, UINT msg, WPARAM w, LPARAM l );
