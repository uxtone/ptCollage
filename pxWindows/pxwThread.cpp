#include <uxStr.h>
// '16/01/28 pxwThread.

#include "./pxwThread.h"

#include <pxMem.h>

pxwThread::pxwThread()
{
	_b_init      = false;
	_h_thrd      = NULL ;
	_b_working   = false;
	_thrd_id     =     0;
	_proc        = NULL ;
	_cs_working  = NULL ;

	pxMem_zero( _name, sizeof(_name) );
}

pxwThread::~pxwThread()
{
	if( !_b_init ) return;
	_b_init = false;
	DeleteCriticalSection( _cs_working );
	delete _cs_working;
}

bool pxwThread::init()
{
	if( _b_init ) return false;
	bool b_ret = false;
	if( !( _cs_working = new CRITICAL_SECTION() ) ) goto term;
	InitializeCriticalSection( _cs_working );
	_b_init = true;
	b_ret = true;
term:
	if( !b_ret ){ delete _cs_working; _cs_working = NULL; }

	return b_ret;
}

unsigned pxwThread::_thrd_func_static( void* arg )
{
	static_cast<pxwThread*>( arg )->_thrd_func();
	_endthreadex( 0 );
	return 0;
}

void pxwThread::_thrd_func()
{
	if( !_b_init ) return;
	if( FAILED( CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) ) return;
	_b_success = _proc( _user );
	OutputDebugString( uxT( "pxw end01\n" ) );
//	CoUninitialize();
	OutputDebugString( uxT( "pxw end02\n" ) );
	EnterCriticalSection( _cs_working );
	_b_working = false;
	LeaveCriticalSection( _cs_working );
}

bool pxwThread::standby( pxwThread_callback proc, void *user, const char *name )
{
	if( !_b_init ) return false;
	{
		bool b_working = false;
		EnterCriticalSection( _cs_working );
		b_working = _b_working;
		LeaveCriticalSection( _cs_working );
		if( b_working ) return false;
	}

	bool b_ret = false;
	int  prio  = THREAD_PRIORITY_NORMAL;

	_proc      = proc ;
	_user      = user ;
	_b_success = false;
	if( name ) strcpy( _name, name );

	EnterCriticalSection( _cs_working );
	_b_working = true;
	LeaveCriticalSection( _cs_working );

	if( !( _h_thrd = (HANDLE)_beginthreadex( NULL, 0, _thrd_func_static, reinterpret_cast<void*>(this), CREATE_SUSPENDED, &_thrd_id ) ) ) goto End;

	b_ret = true;
End:
	if( !b_ret )
	{
		EnterCriticalSection( _cs_working );
		_b_working = false;
		LeaveCriticalSection( _cs_working );
	}
	return b_ret;
}

bool pxwThread::resume ( int priority )
{
	if( !_b_init ) return false;
	if( !_h_thrd ) return false;

	int prio = THREAD_PRIORITY_NORMAL;
	switch( priority )
	{
	case 4 : prio = THREAD_PRIORITY_HIGHEST     ; break;
	case 3 : prio = THREAD_PRIORITY_ABOVE_NORMAL; break;
	case 2 : prio = THREAD_PRIORITY_NORMAL      ; break;
	case 1 : prio = THREAD_PRIORITY_LOWEST      ; break;
	case 0 : prio = THREAD_PRIORITY_IDLE        ; break;
	}
	SetThreadPriority( _h_thrd, prio );

	if( ResumeThread( _h_thrd ) == -1 ) return false;
	return true;
}

unsigned int pxwThread::thread_id() const
{
	if( !_b_init ) return 0;
	return _thrd_id;
}

bool pxwThread::is_working()
{
	if( !_b_init ) return false;
	bool b_ret = false;
	if( !TryEnterCriticalSection( _cs_working ) ) return true;
	b_ret = _b_working;
	LeaveCriticalSection( _cs_working );
	return b_ret;
}

bool pxwThread::join( float timeout_sec, bool* p_b_success )
{
	if( p_b_success ) *p_b_success = false;
	if( !_b_init ) return false;
	if( !_h_thrd ) return false;

	bool  b_ret = false;
	DWORD sec   =     0;

	if( timeout_sec < 0 ) sec = INFINITE;
	else                  sec = (DWORD)(timeout_sec * 1000);
	switch( WaitForSingleObject( _h_thrd, sec ) )
	{
	case WAIT_ABANDONED: break;
	case WAIT_OBJECT_0 : b_ret = true; if( p_b_success ) *p_b_success = _b_success; break;
	case WAIT_TIMEOUT  : break;
	}
	if( b_ret ){ CloseHandle( _h_thrd ); _h_thrd = NULL; }
	return b_ret;
}
