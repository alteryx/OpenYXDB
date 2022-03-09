#include "stdafx.h"

#include "Base/Hash128.h"

#include <iomanip>
#include <sstream>

#pragma warning(push)
// disable signed to unsigned conversion warning
#pragma warning(disable : 4245)
#include "SpookyV2.h"
#pragma warning(pop)

namespace SRC {
Hash128::Hash128(uint64_t part1, uint64_t part2)
	: m_part1(part1)
	, m_part2(part2)
{
}

Hash128::Hash128(const void* pData, unsigned nLength)
{
	SpookyHash::Hash128(pData, nLength, &m_part1, &m_part2);
}

void Hash128::Add(const void* pData, unsigned nLength)
{
	SpookyHash::Hash128(pData, nLength, &m_part1, &m_part2);
}

void Hash128::Add(const Hash128& hash)
{
	SpookyHash::Hash128(&hash, sizeof(Hash128), &m_part1, &m_part2);
}

AString Hash128::GetHexDigest() const
{
	std::stringstream ret;

	ret << std::setfill('0') << std::setw(16) << std::hex << m_part1;
	ret << std::setfill('0') << std::setw(16) << std::hex << m_part2;

	return { ret.str().c_str() };
}

uint32_t Hash128::GetHash32() const
{
	return static_cast<uint32_t>(m_part1);
}

uint64_t Hash128::GetHash64() const
{
	return m_part1;
}

bool Hash128::operator==(const Hash128& o) const noexcept
{
	return m_part1 == o.m_part1 && m_part2 == o.m_part2;
}

bool Hash128::operator!=(const Hash128& o) const noexcept
{
	return m_part1 != o.m_part1 || m_part2 != o.m_part2;
}

bool Hash128::operator<(const Hash128& o) const noexcept
{
	return m_part1 != o.m_part1 ? m_part1 < o.m_part1 : m_part2 < o.m_part2;
}

bool Hash128::operator<=(const Hash128& o) const noexcept
{
	return m_part1 != o.m_part1 ? m_part1 < o.m_part1 : m_part2 <= o.m_part2;
}

}  // namespace SRC
