// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN64

	#define _CRT_SECURE_NO_WARNINGS
	#include <Windows.h>

	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0601
	#endif

#endif

#include <algorithm>
#include <memory>
#include <string>

#include "SrcLib_Replacement.h"
