
HINSTANCE g_hInst = NULL;

bool APIENTRY DllMain( HANDLE hInst, DWORD reason, LPVOID lpReserved )
{
	switch( reason )
	{
	case DLL_PROCESS_ATTACH: g_hInst = (HINSTANCE)hInst;
	case DLL_THREAD_ATTACH :
	case DLL_THREAD_DETACH :
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}

