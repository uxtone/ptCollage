#pragma once
#include_next <tchar.h>

// Winelib builds get Wine's <tchar.h> but glibc's C runtime, which has none of
// Microsoft's *_s functions and cannot take DOS paths or 16-bit wide strings.
// The pieces this project uses are provided here (narrow) and in
// pxWinelibWide.h (wide).
#ifdef __cplusplus

#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <pxWinelibFile.h>
#undef _tfopen
#define _tfopen pxwl_fopen

// Returns the length written, or -1 if the text did not fit (it is then cut
// short but always terminated; Microsoft's version aborts instead).
inline int pxwl_vsnprintf_s(char *buf, size_t n, const char *fmt, va_list ap) {
	if (!buf || !n)
		return -1;
	if (!fmt) {
		buf[0] = '\0';
		return -1;
	}
	const int r = vsnprintf(buf, n, fmt, ap);
	if (r < 0) {
		buf[0] = '\0';
		return -1;
	}
	return static_cast<size_t>(r) < n ? r : -1;
}

__attribute__((format(printf, 3, 4))) inline int
_stprintf_s(char *buf, size_t n, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	const int r = pxwl_vsnprintf_s(buf, n, fmt, ap);
	va_end(ap);
	return r;
}

template <size_t N>
__attribute__((format(printf, 2, 3))) inline int
_stprintf_s(char (&buf)[N], const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	const int r = pxwl_vsnprintf_s(buf, N, fmt, ap);
	va_end(ap);
	return r;
}

inline int _vstprintf_s(char *buf, size_t n, const char *fmt, va_list ap) {
	return pxwl_vsnprintf_s(buf, n, fmt, ap);
}

template <size_t N>
inline int _vstprintf_s(char (&buf)[N], const char *fmt, va_list ap) {
	return pxwl_vsnprintf_s(buf, N, fmt, ap);
}

inline int strcpy_s(char *dst, size_t n, const char *src) {
	if (!dst || !n)
		return EINVAL;
	if (!src) {
		dst[0] = '\0';
		return EINVAL;
	}
	const size_t len = strlen(src);
	if (len >= n) {
		dst[0] = '\0';
		return ERANGE;
	} // Microsoft's version aborts here; fail safely instead.
	memmove(dst, src, len + 1);
	return 0;
}

template <size_t N> inline int strcpy_s(char (&dst)[N], const char *src) {
	return strcpy_s(dst, N, src);
}

inline int _itoa_s(int value, char *buf, size_t n, int radix) {
	if (!buf || !n)
		return EINVAL;
	if (radix < 2 || radix > 36) {
		buf[0] = '\0';
		return EINVAL;
	}

	static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char tmp[34]; // 32 binary digits + sign + NUL
	size_t i = 0;
	const unsigned int base = static_cast<unsigned int>(radix);
	const bool neg = (radix == 10 &&
		value < 0); // other radixes show the two's complement bits.
	unsigned int v = static_cast<unsigned int>(
		value); // modular conversion, defined for every int.
	if (neg)
		v = 0u - v; // magnitude, also for INT_MIN.
	do {
		tmp[i++] = digits[v % base];
		v /= base;
	} while (v);
	if (neg)
		tmp[i++] = '-';

	if (i + 1 > n) {
		buf[0] = '\0';
		return ERANGE;
	}
	for (size_t j = 0; j < i; j++)
		buf[j] = tmp[i - 1 - j];
	buf[i] = '\0';
	return 0;
}

template <size_t N> inline int _itoa_s(int value, char (&buf)[N], int radix) {
	return _itoa_s(value, buf, N, radix);
}

#ifndef _ttof
#define _ttof atof
#endif

#include <pxWinelibWide.h>

#endif // __cplusplus
