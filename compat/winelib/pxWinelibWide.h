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
inline int pxwl_wtoi(const WCHAR *s) {
  char b[64];
  pxwl_narrow(b, sizeof(b), s);
  return static_cast<int>(strtol(b, nullptr, 10));
}
inline long pxwl_wtol(const WCHAR *s) {
  char b[64];
  pxwl_narrow(b, sizeof(b), s);
  return strtol(b, nullptr, 10);
}
inline double pxwl_wtof(const WCHAR *s) {
  char b[64];
  pxwl_narrow(b, sizeof(b), s);
  return strtod(b, nullptr);
}

// Numeric conversions only (%d, %f ...). A conversion that stores text would be
// handed a wide buffer and filled with narrow characters, so those are refused.
inline int pxwl_swscanf(const WCHAR *src, const WCHAR *fmt, ...) {
  char src_a[256], fmt_a[64];
  if (!src || !fmt || pxwl_wcslen(fmt) >= sizeof(fmt_a))
    return EOF;
  pxwl_narrow(src_a, sizeof(src_a), src);
  pxwl_narrow(fmt_a, sizeof(fmt_a), fmt);
  for (const char *p = fmt_a; *p; p++) {
    if (*p != '%')
      continue;
    p++;
    if (*p == '%')
      continue;
    while (*p && !strchr("diouxXeEfFgGaAscSC[np%", *p))
      p++; // flags, width, length.
    if (!*p || strchr("scSC[np", *p))
      return EOF;
  }
  va_list ap;
  va_start(ap, fmt);
  const int r = vsscanf(src_a, fmt_a, ap);
  va_end(ap);
  return r;
}

// wide printf with Microsoft semantics: %s is a wide string, %S / %hs a narrow
// one. Returns the length written, or -1 if the text did not fit (it is then
// cut short but always terminated).
inline int pxwl_vsnwprintf(WCHAR *buf, size_t n, const WCHAR *fmt, va_list ap) {
  if (!buf || !n)
    return -1;
  if (!fmt) {
    buf[0] = 0;
    return -1;
  }

  size_t o =
      0; // characters the full text needs; only those that fit are stored.
  struct put_t {
    WCHAR *buf;
    size_t n;
    size_t *o;
    void operator()(WCHAR c) const {
      if (*o + 1 < n)
        buf[*o] = c;
      ++*o;
    }
  };
  const put_t put = {buf, n, &o};

  for (const WCHAR *p = fmt; *p; p++) {
    if (*p != '%') {
      put(*p);
      continue;
    }
    p++;
    if (*p == '%') {
      put(L'%');
      continue;
    }

    // flags, width and precision are copied into a narrow spec; '*' takes its
    // int argument here. Worst case: '%' + 31 copied characters + 11 for a '*'
    // + "ll" + conversion + NUL = 47 < 64.
    char spec[64];
    size_t s = 0;
    spec[s++] = '%';
    while (*p && *p < 0x80 &&
           strchr("-+ #0123456789.*", static_cast<char>(*p)) && s < 32) {
      if (*p == '*') {
        const int w = snprintf(spec + s, 12, "%d", va_arg(ap, int));
        if (w > 0)
          s += static_cast<size_t>(w);
      } else {
        spec[s++] = static_cast<char>(*p);
      }
      p++;
    }

    enum { LEN_hh, LEN_h, LEN_none, LEN_l, LEN_ll, LEN_L } len = LEN_none;
    if (*p == 'l') {
      len = LEN_l;
      p++;
      if (*p == 'l') {
        len = LEN_ll;
        p++;
      }
    } else if (*p == 'h') {
      len = LEN_h;
      p++;
      if (*p == 'h') {
        len = LEN_hh;
        p++;
      }
    } else if (*p == 'I' && p[1] == '6' && p[2] == '4') {
      len = LEN_ll;
      p += 3;
    } else if (*p == 'I' && p[1] == '3' && p[2] == '2') {
      len = LEN_none;
      p += 3;
    } else if (*p == 'z' || *p == 'j' || *p == 't') {
      len = LEN_ll;
      p++;
    } // all 64-bit here.
    else if (*p == 'L') {
      len = LEN_L;
      p++;
    } else if (*p == 'w') {
      len = LEN_l;
      p++;
    } // %ws
    if (!*p)
      break;
    if (*p >= 0x80) {
      put(L'%');
      put(*p);
      continue;
    } // not a conversion character.
    const char conv = static_cast<char>(*p);

    char tmp[512];
    tmp[0] = '\0';
    switch (conv) {
    case 's':
    case 'S':
      if ((conv == 'S' && len != LEN_l) ||
          (conv == 's' && (len == LEN_h || len == LEN_hh))) {
        const char *a = va_arg(ap, const char *);
        if (!a)
          a = "(null)";
        WCHAR w[512];
        const int w_num = static_cast<int>(sizeof(w) / sizeof(w[0]));
        if (MultiByteToWideChar(CP_ACP, 0, a, -1, w, w_num)) {
          for (const WCHAR *q = w; *q; q++)
            put(*q);
        } else {
          for (; *a; a++)
            put(static_cast<WCHAR>(static_cast<unsigned char>(*a)));
        } // too long to convert in one piece.
      } else {
        const WCHAR *w = va_arg(ap, const WCHAR *);
        if (!w)
          w = L"(null)";
        for (; *w; w++)
          put(*w);
      }
      continue;
    case 'c':
    case 'C':
      put(static_cast<WCHAR>(
          va_arg(ap, int))); // char and WCHAR both arrive promoted to int.
      continue;
    case 'd':
    case 'i':
      if (len == LEN_ll) {
        spec[s++] = 'l';
        spec[s++] = 'l';
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, long long));
      } else if (len == LEN_l) {
        spec[s++] = 'l';
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, long));
      } else {
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, int));
      }
      break;
    case 'u':
    case 'x':
    case 'X':
    case 'o':
      if (len == LEN_ll) {
        spec[s++] = 'l';
        spec[s++] = 'l';
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, unsigned long long));
      } else if (len == LEN_l) {
        spec[s++] = 'l';
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, unsigned long));
      } else {
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, unsigned int));
      }
      break;
    case 'f':
    case 'F':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
      if (len == LEN_L) {
        spec[s++] = 'L';
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, long double));
      } else {
        spec[s++] = conv;
        spec[s] = '\0';
        snprintf(tmp, sizeof(tmp), spec, va_arg(ap, double));
      }
      break;
    case 'p':
      snprintf(tmp, sizeof(tmp), "%p", va_arg(ap, void *));
      break;
    default: // unknown conversion (or %n, never honoured): print it literally,
             // consume nothing.
      put(L'%');
      put(*p);
      continue;
    }
    for (const char *t = tmp; *t; t++)
      put(static_cast<WCHAR>(static_cast<unsigned char>(*t)));
  }
  buf[o < n ? o : n - 1] = 0;
  return (o < n && o <= static_cast<size_t>(INT_MAX)) ? static_cast<int>(o)
                                                      : -1;
}

inline int _vstprintf_s(WCHAR *buf, size_t n, const WCHAR *fmt, va_list ap) {
  return pxwl_vsnwprintf(buf, n, fmt, ap);
}
template <size_t N>
inline int _vstprintf_s(WCHAR (&buf)[N], const WCHAR *fmt, va_list ap) {
  return pxwl_vsnwprintf(buf, N, fmt, ap);
}

inline int _stprintf_s(WCHAR *buf, size_t n, const WCHAR *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const int r = pxwl_vsnwprintf(buf, n, fmt, ap);
  va_end(ap);
  return r;
}
template <size_t N>
inline int _stprintf_s(WCHAR (&buf)[N], const WCHAR *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const int r = pxwl_vsnwprintf(buf, N, fmt, ap);
  va_end(ap);
  return r;
}

inline int pxwl_itow_s(int value, WCHAR *buf, size_t n, int radix) {
  char tmp[34];
  if (!buf || !n)
    return EINVAL;
  const int err = _itoa_s(value, tmp, sizeof(tmp),
                          radix); // the narrow version in the compat <tchar.h>.
  if (err) {
    buf[0] = 0;
    return err;
  }
  const size_t len = strlen(tmp);
  if (len >= n) {
    buf[0] = 0;
    return ERANGE;
  }
  for (size_t i = 0; i <= len; i++)
    buf[i] = static_cast<WCHAR>(static_cast<unsigned char>(tmp[i]));
  return 0;
}
template <size_t N>
inline int pxwl_itow_s(int value, WCHAR (&buf)[N], int radix) {
  return pxwl_itow_s(value, buf, N, radix);
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
#undef _ttoi
#define _ttoi pxwl_wtoi
#undef _ttol
#define _ttol pxwl_wtol
#undef _ttof
#define _ttof pxwl_wtof
#undef _stscanf
#define _stscanf pxwl_swscanf
#undef _itot_s
#define _itot_s pxwl_itow_s
#endif

#endif
