#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace SRC { namespace Unicode {

// Unicode constants
// Leading (high) surrogates: 0xd800 - 0xdbff
// Trailing (low) surrogates: 0xdc00 - 0xdfff
const char32_t LEAD_SURROGATE_MIN = 0xd800u;
const char32_t LEAD_SURROGATE_MAX = 0xdbffu;
const char32_t TRAIL_SURROGATE_MIN = 0xdc00u;
const char32_t TRAIL_SURROGATE_MAX = 0xdfffu;
const char32_t MAX_UNICODE_VALUE = 0x10ffffu;
const char16_t UTF8_FOLLOWER_MIN = 0x80u;
const char16_t UTF8_FOLLOWER_MAX = 0xBFu;

inline bool IsUTF16LeadSurrogate(char32_t cp) noexcept
{
	return (cp >= LEAD_SURROGATE_MIN && cp <= LEAD_SURROGATE_MAX);
}
inline bool IsUTF16TrailSurrogate(char32_t cp) noexcept
{
	return (cp >= TRAIL_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
}

inline bool IsUTF8Follower(char16_t cp) noexcept
{
	return (UTF8_FOLLOWER_MIN <= cp && cp <= UTF8_FOLLOWER_MAX);
}

template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
constexpr bool TypeCanHoldUnicodeAssert()
{
	return static_cast<uint64_t>(std::numeric_limits<T>::max()) >= MAX_UNICODE_VALUE;
}

inline bool IsValidUnicode(char32_t unicode) noexcept
{
	return unicode <= MAX_UNICODE_VALUE && (unicode < LEAD_SURROGATE_MIN || unicode > TRAIL_SURROGATE_MAX)
		   && (unicode & 0xFFFF) < 0xFFFE;
}

}}  // namespace SRC::Unicode
