
#include <pxtnService.h>
extern pxtnService *g_pxtn;

static int32_t _focus   = 0;

////////////////////
// focus
////////////////////
void UnitFocus_Set( int32_t index, bool bOperated )
{
	if( index < g_pxtn->Unit_Num() )
	{
		_focus = index;
		g_pxtn->Unit_Get_variable( index )->set_operated( bOperated );
	}
}

int32_t UnitFocus_Get()
{
	int32_t num = g_pxtn->Unit_Num();
	if( !num          ) return -1;
	if( _focus <    0 ) _focus =       0;
	if( _focus >= num ) _focus = num - 1;
	return _focus;
}

bool UnitFocus_Shift( bool bNext )
{
	if( _focus < 0 ) return false;

	if( bNext )
	{
		if( _focus < g_pxtn->Unit_Num() -1 ) _focus++;
		else return false;
	}
	else
	{
		if( _focus > 0                     ) _focus--;
		else return false;
	}

	return true;
}

// 編集対象の数
int32_t UnitFocus_CountFocusedOrOperated()
{
	int32_t num;
	int32_t count = 0;

	if( _focus < 0 ) return 0;

	num = g_pxtn->Unit_Num();

	for( int32_t u = 0; u < num; u++ )
	{
		const pxtnUnit *p_unit = g_pxtn->Unit_Get( u );
		if( p_unit )
		{
			if( u == _focus || p_unit->get_operated() ) count++;
		}
	}
	return count;
}


bool UnitFocus_IsFocusedOrOperated( int32_t u )
{
	if( _focus < 0 ) return false;

	const pxtnUnit *p_unit = g_pxtn->Unit_Get( u );
	if( !p_unit                ) return false;
	if( u == _focus            ) return true;
	if( p_unit->get_operated() ) return true;

	return false;
}

#include "interface/if_ToolPanel.h"
bool UnitFocus_IsFocusOnly()
{
	if( if_ToolPanel_GetMode() == enum_ToolMode_U && UnitFocus_CountFocusedOrOperated() > 1 ) return false;
	return true;
}

//////////////////
