#pragma once

#ifdef _WIN32
using U16unit = wchar_t;
	#define U16(x) L##x
#else
using U16unit = char16_t;
	#define U16(x) u##x
#endif
