#pragma once
// required for libpng: glibc's setjmp() is a macro for _setjmp(), and ntdll
// exports its own function of that name with the Windows calling convention,
// which Winelib will pick while linking unless we undef it and alias to the
// glibc version

#include <setjmp.h>
#undef setjmp
#define setjmp(a) __sigsetjmp((a), 0)
