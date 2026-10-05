// '16/12/28 CursorKeyCtrl.

#ifndef CursorKeyCtrl_H
#define CursorKeyCtrl_H

enum CursorKeyMode
{
	CursorKey_none = 0,
	CursorKey_wave    ,
	CursorKey_envelope,
};

class CursorKeyCtrl
{
private:
void operator = (const CursorKeyCtrl& src){}
CursorKeyCtrl   (const CursorKeyCtrl& src){}

CursorKeyMode _mode ;
int32_t       _idx  ;
const void*   _p_tgt;

public:
CursorKeyCtrl();

void clear();
void set  ( CursorKeyMode mode, int32_t idx, const void* p_tgt );

CursorKeyMode get( int32_t* p_idx ) const;
const void* get_tgt() const;
};

#endif
