#pragma once

//Elwood: This file deliberately left inlined

#include "Base/String_ImpExp.h"
#include "Base/U16unit.h"

namespace SRC {
struct UTF16Traits
{
	typedef U16unit TChar;
	static constexpr int BytesPerUnit = sizeof(U16unit);

	// these report the number of char16's used to write the code point and moves the
	// buffer pointer.
	// All valid code points can be written, so the error return versions just turn into
	// the basic versions without error return
	STRING_EXPORT unsigned static WriteCodePoint(const char32_t cp, U16unit*& buffer);
	inline unsigned static WriteCodePoint(char32_t cp, U16unit*& buffer, char32_t* /*badcp*/)
	{
		return WriteCodePoint(cp, buffer);
	}

	static int UnitsForChar(char32_t ch)
	{
		return (ch < 0x10000) ? 1 : 2;
	}

	struct STRING_EXPORT CodePoint
	{
		const U16unit* m_buffer;
		unsigned m_length;

		CodePoint(const U16unit* buffer, unsigned length)
			: m_buffer(buffer)
			, m_length(length)
		{
		}

		char32_t Number() const noexcept;

		unsigned CharUnits() const
		{
			return m_length;
		}

		// if the lengths are different then the bytes up to the shortest length are different by definition of the encoding
		inline friend bool operator==(const CodePoint& a, const CodePoint& b)
		{
			return memcmp(a.m_buffer, b.m_buffer, (a.m_length < b.m_length ? a.m_length : b.m_length) * sizeof(U16unit))
				   == 0;
		}
	};

	struct CodePointIterator
	{
		const U16unit* m_str;
		unsigned m_Pos;
		STRING_EXPORT CodePointIterator(const U16unit* str, unsigned pos);

		inline bool operator!=(const CodePointIterator& other) const
		{
			return m_Pos != other.m_Pos;
		}
		inline bool operator==(const CodePointIterator& other) const
		{
			return m_Pos == other.m_Pos;
		}
		inline bool operator<(const CodePointIterator& other) const
		{
			return m_Pos < other.m_Pos;
		}
		inline bool operator>(const CodePointIterator& other) const
		{
			return m_Pos > other.m_Pos;
		}

		STRING_EXPORT CodePointIterator& operator++();
		STRING_EXPORT CodePointIterator& operator--();
		STRING_EXPORT const CodePoint operator*() const;

		// advance by N code points, but don't go beyond the null at the end.
		STRING_EXPORT CodePointIterator& operator+=(unsigned N);

		// go back by N code points
		STRING_EXPORT CodePointIterator& operator-=(unsigned N);

		// tell how many code points there are from here to the end
		STRING_EXPORT unsigned LengthPoints() const;

		// returns the unicode of current iterator position.
		// will return -1 for both invalid unicode encodings, and invalid unicode itself.
		STRING_EXPORT int ValidNumber() const noexcept;

		unsigned Pos() const
		{
			return m_Pos;
		}

		// if you already know how many units this code point is, you can just add it here.
		void IncUnits(int deltaUnits)
		{
			m_Pos += deltaUnits;
		}

		// if you know the underlying string has (or even may have) been reallocated, but the position
		// in the buffer should be the same, you can fix your invalidated iterator
		void SetString(const U16unit* newStr)
		{
			m_str = newStr;
		}

		// tell how many code points there are from here to where RHS points
		//			STRING_EXPORT unsigned operator-(const CodePointIterator& rhs) const;

		// give the value of the code point at the current position, and advance to
		// the next code point.
		// Does its best to repair broken UTF-16 (Lead surrogate without trail, or
		// pointing at a trailing) by returning the replacement character (U+FFFD)
		// and advancing one char16.
		STRING_EXPORT char32_t NumberAdvRepair() noexcept;
	};

	class STRING_EXPORT CodePoints
	{
		CodePointIterator m_Begin;
		CodePointIterator m_End;

	public:
		CodePoints(const U16unit* p, int nLength);

		inline CodePointIterator begin()
		{
			return m_Begin;
		}
		inline CodePointIterator end()
		{
			return m_End;
		}
	};
};
}  // namespace SRC
