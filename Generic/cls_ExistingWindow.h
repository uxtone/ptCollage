#include <pxStdDef.h>
#pragma once
typedef class cls_EXISTINGWINDOW
{
	private:

	HANDLE _hMapping;
	HANDLE _hMutex;
	HWND   *_p_hwnd_map;

	cls_EXISTINGWINDOW(              const cls_EXISTINGWINDOW &src  ){
	} // copy
	cls_EXISTINGWINDOW & operator = (const cls_EXISTINGWINDOW &right){ return *this; } // substitution

	public:

	cls_EXISTINGWINDOW();
	~cls_EXISTINGWINDOW();

	bool Check  ( const uxDS& mutex_name, const uxDS& mapping_name, UINT msg  );
	bool Mapping( const uxDS& mutex_name, const uxDS& mapping_name, HWND hWnd );

}
cls_EXISTINGWINDOW;
