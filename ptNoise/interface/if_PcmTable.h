void         if_PcmTable_Initialize        ();
void         if_PcmTable_SetRect           ( const fRECT *rc_view );
const fRECT* if_PcmTable_GetRect           ();
void         if_PcmTable_Put               ( const fRECT *rc_view );
void         if_PcmTable_Put_CursorPosition( float cur_x, float cur_y );
bool         if_PcmTable_IsHitPCM          ( float cur_x, float cur_y );
void         if_PcmTable_Scope_SetStart    ( float cur_x );
void         if_PcmTable_Scope_SetEnd      ( float cur_x );
bool         if_PcmTable_Scope_GetScope    ( int32_t* p_start, int32_t* p_end );
bool         if_PcmTable_Scope_IsHitPCM    ( float cur_x, float cur_y );

void         if_PcmTable_SetParameter      ( int32_t zoom );
