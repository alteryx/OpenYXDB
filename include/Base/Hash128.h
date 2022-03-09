#pragma once

#include <cstdint>

#include "Base/Base_ImpExp.h"
#ifdef SRCLIB_REPLACEMENT
	#include "SrcLib_Replacement.h"
#else
	#include "Base/Src_String_fwd.h"
#endif

namespace SRC {
///////////////////////////////////////////////////////////////////////////////
// class Hash128
//
// produces a 128 bit hash that is NOT cryptographically secure, but is fast
// the algorithm is not guaranteed to stay the same, so these can't be persisted
//
// currently implemented with the SpookyHashV2 algorithm from: http://burtleburtle.net/bob/hash/spooky.html

struct BASE_EXPORT Hash128
{
	uint64_t m_part1{ 0x123456789abcdef0 };  // seed picked for no good reason
	uint64_t m_part2{ 0x23456789abcdef01 };

	Hash128() = default;
	Hash128(uint64_t part1, uint64_t part2);

	/** Compute a hash of the data at pData, for nLength bytes. It just reads the bytes out of memory, the
		* hash code will be different for the same string in UTF8, or UTF16. */
	Hash128(const void* pData, unsigned nLength);

	//These methods are DEPRECATED
	// Splitting a string and calling Add will result in a different hash than calling it all as a single string
	// it will only compare true if both sides use the same pattern of Add
	void Add(const void* pData, unsigned nLength);
	void Add(const Hash128& hash);

	/** The author says that any subset of the bits are equally random, suitable for smaller hash functions */
	uint32_t GetHash32() const;

	/** The author says that any subset of the bits are equally random, suitable for smaller hash functions */
	uint64_t GetHash64() const;

	AString GetHexDigest() const;

	bool operator==(const Hash128& o) const noexcept;
	bool operator!=(const Hash128& o) const noexcept;
	bool operator<(const Hash128& o) const noexcept;
	bool operator<=(const Hash128& o) const noexcept;
};

}  // namespace SRC
