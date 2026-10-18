#ifndef _GCC_WIN_STRINGS
#define _GCC_WIN_STRINGS

// Intentionally empty: on Windows (win32) the MSVC CRT provides _stricmp,
// _strnicmp and _strlwr natively. These declarations were only needed for
// GCC/Unix builds, which are no longer part of this project.

#endif // _GCC_WIN_STRINGS
