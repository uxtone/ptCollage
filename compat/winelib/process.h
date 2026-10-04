#pragma once
// _beginthreadex / _endthreadex are MSVCRT defined, so we reimplement on top of
// the Win32 thread calls. The thread routine and the id pointer are passed
// without casts on purpose: under Winelib `unsigned` and DWORD are the same
// type, so they convert implicitly. If that ever stops being true this must
// fail to compile rather than call through a mismatched signature.
#include <stdint.h>
#include <windows.h>

#ifdef __cplusplus

inline uintptr_t _beginthreadex(void *security, unsigned stack_size,
                                unsigned(__stdcall *start)(void *), void *arg,
                                unsigned initflag, unsigned *thrdaddr) {
  LPTHREAD_START_ROUTINE routine = start;
  LPDWORD id = thrdaddr;
  HANDLE h = CreateThread(static_cast<LPSECURITY_ATTRIBUTES>(security),
                          stack_size, routine, arg, initflag, id);
  return reinterpret_cast<uintptr_t>(h); // 0 on failure, like the CRT.
}

inline void _endthreadex(unsigned retval) { ExitThread(retval); }

#endif
