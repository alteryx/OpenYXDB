#pragma once

#ifdef __linux__
	#define _stdcall
	#define __stdcall
	#define _int64 int64_t
	#define __int64 int64_t
	#define _I64_MAX INT64_MAX
	#define _I64_MIN INT64_MIN
	#define _forceinline
	#define WINAPI
using DWORD = unsigned int;
using LPVOID = void*;
using LONG = long;  // what a super typedef. "increment i"
using HANDLE = void*;
#endif
