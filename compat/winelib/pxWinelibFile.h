#pragma once
// Winelib links against glibc, whose fopen() knows nothing about DOS paths
// ("Z:\home\..."). Translate through Wine before handing the path to libc.

// Note: glibc has no text mode. A "t" in the mode is ignored, so files opened
// with "wt" get LF line ends here, not CR LF. */
#include <stdio.h>
#include <windows.h>

#ifdef __cplusplus

inline FILE *pxwl_fopen(const WCHAR *path, const char *mode) {
  if (!path || !mode || !path[0])
    return nullptr;
  char *unix_path =
      wine_get_unix_file_name(path); // allocated on the process heap.
  if (!unix_path)
    return nullptr;
  FILE *fp = fopen(unix_path, mode);
  HeapFree(GetProcessHeap(), 0, unix_path);
  return fp;
}

inline FILE *pxwl_fopen(const WCHAR *path, const WCHAR *mode) {
  char mode_a[16];
  size_t i = 0;
  if (!mode)
    return nullptr;
  for (; mode[i]; i++) {
    if (i + 1 >= sizeof(mode_a) || mode[i] >= 0x80)
      return nullptr; // not a mode string.
    mode_a[i] = static_cast<char>(mode[i]);
  }
  mode_a[i] = '\0';
  return pxwl_fopen(path, mode_a);
}

inline FILE *pxwl_fopen(const char *path, const char *mode) {
  if (!path)
    return nullptr;
  WCHAR path_w[MAX_PATH * 2];
  const int path_w_num = static_cast<int>(sizeof(path_w) / sizeof(path_w[0]));
  if (!MultiByteToWideChar(CP_ACP, 0, path, -1, path_w, path_w_num))
    return nullptr; // 0: invalid or too long.
  return pxwl_fopen(path_w, mode);
}

#endif
