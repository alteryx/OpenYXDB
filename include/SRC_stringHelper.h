#pragma once

#include <algorithm>
#include <limits>

#include "Base/Base_ImpExp.h"
#include "Base/SCType.h"
#include "Base/U16unit.h"

// These functions assume the size of a buffer includes the null terminator.
// Ex. char buf[20] will store 19 characters and 1 null terminator.
namespace SRC { namespace StringHelper {

template <typename ChT, typename ValT>
ValT BASE_EXPORT TstrtoNum(const ChT* nptr, ChT const** endptr, int base) noexcept;

// matching the behavior in https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/itoa-s-itow-s?view=vs-2019
// also see http://www.cplusplus.com/reference/cstdlib/itoa/
template <typename ChT, typename ValT>
int BASE_EXPORT utowcs(ValT uval, ChT* buffer, size_t size, int radix) noexcept;

template <typename ChT, typename ValT>
int BASE_EXPORT itowcs(ValT value, ChT* buffer, size_t size, int radix) noexcept;

int32_t BASE_EXPORT sh_strtoi(const char* nptr, char** endptr, int base) noexcept;
int32_t BASE_EXPORT sh_strtoi(const U16unit* nptr, U16unit** endptr, int base) noexcept;
int32_t BASE_EXPORT sh_strtoi(const char* pBuffer) noexcept;
int32_t BASE_EXPORT sh_strtoi(const U16unit* pBuffer) noexcept;

int64_t BASE_EXPORT sh_strtoi64(const char* ptr, char** endptr, int base) noexcept;
int64_t BASE_EXPORT sh_strtoi64(const U16unit* ptr, U16unit** endptr, int base) noexcept;
int64_t BASE_EXPORT sh_strtoi64(const char* ptr) noexcept;
int64_t BASE_EXPORT sh_strtoi64(const U16unit* ptr) noexcept;

// return an errno code, instead of setting errno
int BASE_EXPORT sh_itostr(int n, char* pBuffer, size_t nSize) noexcept;
int BASE_EXPORT sh_itostr(int n, U16unit* pBuffer, size_t nSize) noexcept;

int BASE_EXPORT sh_i64tostr(int64_t n, char* pBuffer, size_t nSize) noexcept;
int BASE_EXPORT sh_i64tostr(int64_t n, U16unit* pBuffer, size_t nSize) noexcept;

// returns the length of the result, guaranteed to be <nSize, and null terminated
int BASE_EXPORT sh_dtostr(char* pBuffer, size_t nSize, const int iDecPlaces, const double d) noexcept;
int BASE_EXPORT sh_dtostr(U16unit* pBuffer, size_t nSize, const int iDecPlaces, const double d) noexcept;

}}  // namespace SRC::StringHelper
