#pragma once
// Winelib builds with -fshort-wchar (WCHAR == 16-bit wchar_t) but links glibc,
// whose wcsXXX and wprintf functions assume a 32-bit wchar_t. Nothing wide may
// reach libc, so the TCHAR routines the project uses are implemented here.
// Included from the compat <tchar.h>, C++ only.
#ifdef __cplusplus
#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <windows.h>

inline size_t pxwl_wcslen(const WCHAR *s) {
	size_t n = 0;
	while (s[n])
		n++;
	return n;
}
inline WCHAR *pxwl_wcscpy(WCHAR *d, const WCHAR *s) {
	WCHAR *r = d;
	while ((*d++ = *s++)) {
	}
	return r;
}
inline WCHAR *pxwl_wcscat(WCHAR *d, const WCHAR *s) {
	pxwl_wcscpy(d + pxwl_wcslen(d), s);
	return d;
}
inline WCHAR *pxwl_wcsncpy(WCHAR *d, const WCHAR *s, size_t n) {
	size_t i = 0;
	for (; i < n && s[i]; i++)
		d[i] = s[i];
	for (; i < n; i++)
		d[i] = 0;
	return d;
}
inline int pxwl_wcsncmp(const WCHAR *a, const WCHAR *b, size_t n) {
	for (; n; n--, a++, b++) {
		if (*a != *b)
			return *a < *b ? -1 : 1;
		if (!*a)
			break;
	}
	return 0;
}
inline int pxwl_wcscmp(const WCHAR *a, const WCHAR *b) {
	return pxwl_wcsncmp(a, b, SIZE_MAX);
}

inline WCHAR pxwl_towlower(WCHAR c) {
	if (c < 0x80)
		return (c >= 'A' && c <= 'Z') ? static_cast<WCHAR>(c + ('a' - 'A')) : c;
	// CharLowerW documents this: a "pointer" whose upper bits are all zero is one
	// character, returned the same way.
	const LPWSTR lowered =
		CharLowerW(reinterpret_cast<LPWSTR>(static_cast<ULONG_PTR>(c)));
	return static_cast<WCHAR>(reinterpret_cast<ULONG_PTR>(lowered));
}
inline int pxwl_wcsnicmp(const WCHAR *a, const WCHAR *b, size_t n) {
	for (; n; n--, a++, b++) {
		const WCHAR x = pxwl_towlower(*a), y = pxwl_towlower(*b);
		if (x != y)
			return x < y ? -1 : 1;
		if (!x)
			break;
	}
	return 0;
}
inline int pxwl_wcsicmp(const WCHAR *a, const WCHAR *b) {
	return pxwl_wcsnicmp(a, b, SIZE_MAX);
}

// like the C functions these drop const on the result; the parameter is int so
// 'x' and L'x' both work.
inline WCHAR *pxwl_wcschr(const WCHAR *s, int c) {
	const WCHAR wc = static_cast<WCHAR>(c);
	for (;; s++) {
		if (*s == wc)
			return const_cast<WCHAR *>(s);
		if (!*s)
			return nullptr;
	}
}
inline WCHAR *pxwl_wcsrchr(const WCHAR *s, int c) {
	const WCHAR wc = static_cast<WCHAR>(c);
	const WCHAR *r = nullptr;
	for (;; s++) {
		if (*s == wc)
			r = s;
		if (!*s)
			return const_cast<WCHAR *>(r);
	}
}
inline WCHAR *pxwl_wcsstr(const WCHAR *s, const WCHAR *f) {
	const size_t n = pxwl_wcslen(f);
	if (!n)
		return const_cast<WCHAR *>(s);
	for (; *s; s++) {
		if (!pxwl_wcsncmp(s, f, n))
			return const_cast<WCHAR *>(s);
	}
	return nullptr;
}
inline int pxwl_wcscpy_s(WCHAR *d, size_t n, const WCHAR *s) {
	if (!d || !n)
		return EINVAL;
	if (!s) {
		d[0] = 0;
		return EINVAL;
	}
	const size_t len = pxwl_wcslen(s);
	if (len >= n) {
		d[0] = 0;
		return ERANGE;
	}
	memmove(d, s, (len + 1) * sizeof(WCHAR));
	return 0;
}
template <size_t N> inline int pxwl_wcscpy_s(WCHAR (&d)[N], const WCHAR *s) {
	return pxwl_wcscpy_s(d, N, s);
}

// numbers only ever hold ASCII, so narrow them and let libc parse.
inline void pxwl_narrow(char *d, size_t n, const WCHAR *s) {
	size_t i = 0;
	if (!n)
		return;
	if (s)
		for (; i + 1 < n && s[i]; i++)
			d[i] = s[i] < 0x80 ? static_cast<char>(s[i]) : '?';
	d[i] = '\0';
}

#ifdef _UNICODE
#undef _tcslen
#define _tcslen pxwl_wcslen
#undef _tcscpy
#define _tcscpy pxwl_wcscpy
#undef _tcscat
#define _tcscat pxwl_wcscat
#undef _tcsncpy
#define _tcsncpy pxwl_wcsncpy
#undef _tcscmp
#define _tcscmp pxwl_wcscmp
#undef _tcsncmp
#define _tcsncmp pxwl_wcsncmp
#undef _tcsicmp
#define _tcsicmp pxwl_wcsicmp
#undef _tcsnicmp
#define _tcsnicmp pxwl_wcsnicmp
#undef _tcschr
#define _tcschr pxwl_wcschr
#undef _tcsrchr
#define _tcsrchr pxwl_wcsrchr
#undef _tcsstr
#define _tcsstr pxwl_wcsstr
#undef _tcscpy_s
#define _tcscpy_s pxwl_wcscpy_s
#endif

#endif
