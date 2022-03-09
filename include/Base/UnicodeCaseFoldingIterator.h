#pragma once
#include "Base/SCType.h"
#include "Base/UnicodeCaseFoldingTable.h"

namespace SRC {
// Walks over a string for Case Folded comparison case folding codepoints as required
// Moving the iterator forwards moves either along the base string or the case folded expansion.
// Once the iterator gets to the end of the case folded expansion it goes back to where it left off in the base string
template <class EncodingTraits, class TChar>
class CaseFoldingIterator
{
private:
	typename EncodingTraits::CodePointIterator m_itCurrentPosition;

	bool m_bIsCaseFolded;
	char32_t m_CaseFoldedBuffer[UnicodeCaseMappingTable::S_MaxNumCPInMapping];
	unsigned m_nCaseFoldedBufferPos;

	void WriteToCaseFoldedBuffer()
	{
		if ((*m_itCurrentPosition).m_buffer[0] < 0x80)  // for simple ascii codepoints shortcut the lookup table
		{
			m_CaseFoldedBuffer[0] = SRC::CType::ToLowerASCII((*m_itCurrentPosition).m_buffer[0]);
			m_CaseFoldedBuffer[1] = 0;
		}
		else
		{
			char32_t target((*m_itCurrentPosition).Number());

			auto it = std::lower_bound(
				UnicodeCaseMappingTable::S_Table,
				UnicodeCaseMappingTable::S_Table + UnicodeCaseMappingTable::S_Size,
				target,
				[](std::pair<unsigned, unsigned> const& x, unsigned t) {
					return x.first < t;
				});

			if (it->first != target)
			{
				m_CaseFoldedBuffer[0] = target;
				m_CaseFoldedBuffer[1] = 0;
			}
			else
			{
				unsigned nPos = 0;
				for (; it->first == target; ++nPos, ++it)
					m_CaseFoldedBuffer[nPos] = it->second;
				m_CaseFoldedBuffer[nPos] = 0;
			}
		}
		m_nCaseFoldedBufferPos = 0;
	}

public:
	CaseFoldingIterator(const TChar* p)
		: m_itCurrentPosition(p, 0)
		, m_bIsCaseFolded(false)
		, m_nCaseFoldedBufferPos(0)
	{
	}

	/** Looks for a null termination in the source */
	bool AtEnd()
	{
		return !m_bIsCaseFolded && (*m_itCurrentPosition).m_buffer[0] == 0;
	}

	/** Tells if we have passed N units of the source */
	bool AtEnd(unsigned N)
	{
		return m_itCurrentPosition.m_Pos >= N;
	}

	CaseFoldingIterator operator++()
	{
		if (!m_bIsCaseFolded)
		{
			++m_itCurrentPosition;
		}
		else
		{
			++m_nCaseFoldedBufferPos;
			if (m_CaseFoldedBuffer[m_nCaseFoldedBufferPos] == 0)
			{
				m_bIsCaseFolded = false;
				++m_itCurrentPosition;
			}
		}

		return *this;
	}

	void CaseFold()
	{
		if (!m_bIsCaseFolded)
		{
			WriteToCaseFoldedBuffer();
			m_bIsCaseFolded = true;
		}
	}

	inline friend bool operator==(CaseFoldingIterator& lhs, CaseFoldingIterator& rhs)
	{
		if (!lhs.m_bIsCaseFolded && !rhs.m_bIsCaseFolded && *(lhs.m_itCurrentPosition) == *(rhs.m_itCurrentPosition))
			return true;  // the base codepoint is identical without case mapping so will still be equal if we case match it
		else
		{
			lhs.CaseFold();
			rhs.CaseFold();
			return lhs.m_CaseFoldedBuffer[lhs.m_nCaseFoldedBufferPos]
				   == rhs.m_CaseFoldedBuffer[rhs.m_nCaseFoldedBufferPos];
		}
	}

	inline friend int operator-(CaseFoldingIterator& lhs, CaseFoldingIterator& rhs)
	{
		lhs.CaseFold();
		rhs.CaseFold();
		return lhs.m_CaseFoldedBuffer[lhs.m_nCaseFoldedBufferPos] - rhs.m_CaseFoldedBuffer[rhs.m_nCaseFoldedBufferPos];
	}
};
}  // namespace SRC
