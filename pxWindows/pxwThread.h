// '16/01/28 pxwThread.

#ifndef pxwThread_H
#define pxwThread_H

typedef bool (*pxwThread_callback)( void *user );

class pxwThread
{
private:
bool               _b_init ;
HANDLE             _h_thrd ;
unsigned int       _thrd_id;
pxwThread_callback _proc   ;
void               *_user  ;
char               _name[ 32 ];

CRITICAL_SECTION*  _cs_working;
bool               _b_working ;
bool               _b_success ;

static unsigned __stdcall _thrd_func_static( void* arg );
void                      _thrd_func       (           );

public:
pxwThread();
~pxwThread();

bool         init();
bool         standby( pxwThread_callback proc, void *user, const char* name );
bool         resume ( int priority );
unsigned int thread_id() const;
bool         is_working();
bool         join( float timeout_sec, bool* p_b_success );
};

#endif
