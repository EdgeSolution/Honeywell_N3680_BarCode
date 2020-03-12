#pragma once
// This file shall help to overcome portability issues regarding wchar_t.
// We add overloads with the same name as the regular char* functions but take a wchar_t*.
// This approach avoids the use of Txxx functions or conditinal compile (but only works with C++, not C).
// So original files do not need to be changed for UNICODE builds in most cases.

#ifdef UNICODE
inline int puts(const wchar_t *str) { return _putws(str);	}

inline int printf(const wchar_t *pFormat, ...)
{
	va_list	pArgs;
	int written=0;
	va_start( pArgs, pFormat );
	written = vwprintf_s(pFormat, pArgs );
	va_end(pArgs);
	return written;
}

inline FILE *fopen(const wchar_t *filename, const wchar_t *mode)	{ return _wfopen(filename, mode); }

#endif
