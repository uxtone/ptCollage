
#include "./pxwFilePath.h"

bool pxwDlg_SelFile_OpenLoad( HWND hWnd, TCHAR *path_get, const TCHAR *path_def_dir, const TCHAR *title, const TCHAR *ext, const TCHAR *filter )
{
	OPENFILENAME ofn = {0};

	ofn.lStructSize     = sizeof(OPENFILENAME);//構造体のサイズ
	ofn.hwndOwner       = hWnd    ;//親ウインドウハンドル
	ofn.lpstrFilter     = filter  ; //		"PixTone files {*.pxt}\0*.pxt\0" "All files {*.*}\0*.*\0\0";
	ofn.lpstrFile       = path_get;//ファイルパスを格納するポインタ及び初期パス
	ofn.nMaxFile        = MAX_PATH;//パスサイズ
	ofn.lpstrFileTitle  = NULL    ;//選択ファイル名を入れるポインタ
	ofn.nMaxFileTitle   = 0       ;//選択ファイル名サイズ
	ofn.lpstrInitialDir = path_def_dir;//初期ディレクトリ（NULLでカレントディレクトリになる）
	ofn.lpstrTitle      = title   ;//ダイアログのキャプション
	ofn.Flags           = NULL
//				|OFN_ALLOWMULTISELECT	//複数ファイル選択可（\0で区切られる）●
//				|OFN_CREATEPROMPT		//無いファイルの場合、許可を求める●
		|OFN_FILEMUSTEXIST      //既存のファイルのみ許可●
		|OFN_HIDEREADONLY       //読み取り専用チェックボックスを隠す
//				|OFN_NOCHANGEDIR		//カレントディレクトリを移動しない
//				|OFN_NOLONGNAMES		//長いファイル名を表示しない
		|OFN_PATHMUSTEXIST      //既存のパスのみ許可●
	;
	ofn.lpstrDefExt     = ext;//デフォルトでつけられる拡張子

	if( GetOpenFileName( &ofn ) )//←これがメイン
		return true;
	return false;
}

bool pxwDlg_SelFile_OpenSave( HWND hWnd, TCHAR *path_get, const TCHAR* path_def_dir, const TCHAR *title, const TCHAR *ext, const TCHAR *filter )
{
	OPENFILENAME ofn = {0};

	ofn.lStructSize     = sizeof(OPENFILENAME);//構造体のサイズ
	ofn.hwndOwner       = hWnd;//親ウインドウハンドル
	ofn.lpstrFilter     = filter;//"wave files {*.wav}\0*.wav\0" "All files {*.*}\0*.*\0\0";
	ofn.lpstrFile       = path_get;//ファイルパスを格納するポインタ及び初期パス
	ofn.nMaxFile        = MAX_PATH;//パスサイズ
	ofn.lpstrFileTitle  = NULL;//選択ファイル名を入れるポインタ
	ofn.nMaxFileTitle   = 0;//選択ファイル名サイズ
	ofn.lpstrInitialDir = path_def_dir;//初期ディレクトリ（NULLでカレントディレクトリになる）
	ofn.lpstrTitle      = title;//ダイアログのキャプション
	ofn.Flags           = NULL
		|OFN_HIDEREADONLY       //読み取り専用チェックボックスを隠す
//				|OFN_NOCHANGEDIR		//カレントディレクトリを移動しない
//				|OFN_NOLONGNAMES		//長いファイル名を表示しない
		|OFN_OVERWRITEPROMPT    //既存のファイルの場合、許可を求める●
	;
	ofn.lpstrDefExt     = ext;//デフォルトでつけられる拡張子

	if( GetSaveFileName( &ofn ) )//←これがメイン
		return true;
	DWORD a = GetLastError();
	return false;
}
