// Copyright 2015, Alteryx Inc.  All rights reserved
#pragma once

#include "Base/Base_ImpExp.h"

namespace SRC { namespace CType {
/** In antique C there were macros to classify characters. Nowadays, we're modern
and know all about locales. So whether or not a character is "a space" or is "lower case"
depends on the locale. So the boring old isspace() macro now gets your thread specific data,
checks to see if your locale has recently changed, sets a critical section lock, gets the locale,
gets the ctype facet, and calls a virtual function in it to answer this, now complex, question.
This is now an expensive question to ask, but it is important for most parsing routines

Therefore, we now have re-introduced simple, pedestrian functions that can answer these questions.
Use them quickly.
*/

/* All that stuff about "plain text = ascii = characters are 8 bits" is not only wrong, it's
hopelessly wrong, and if you're still programming that way, you're not much better than a medical
doctor who doesn't believe in germs. Please do not write another line of code until you finish
reading this article.
    http://joelonsoftware.com/articles/Unicode.html
*/

/** A "Codepage" is a collection of 256 characters to be used in a one byte per
character encoding of text. Interesting codepages are:
ASCII -- is exactly the first 128 Unicode characters, represented one for one.
	The 0x80 bit is never set, so there is no confusion about encoding.
ISO 8859-1 -- is exactly the first 256 Unicode characters, represented one for one.
	I'm not sure about the values 0x80 to 0x9f, the wikipedia page indicates that
	they are not assigned, though Unicode uses them as control characters. In particular,
	0x85 is "next line".
	https://en.wikipedia.org/wiki/ISO/IEC_8859-1
Windows 1252 -- is a superset of ISO 8859-1 where they assigned characters to some
    boring code units in the range 0x80 to 0x9F. These added characters include
	ellipsis, Euro, curved quotes, Trademark, and dash. It is common for text encoded
	as 1252 to be mislabeled as 8859-1, so it is helpful to accept it on input, but
	do not mislabel on output.
	https://en.wikipedia.org/wiki/Windows-1252
UTF-8 and UTF-16 are not Codepages, they are multi-byte encoding schemes for ALL
	Unicode	characters. UTF-8 can use one to four bytes, UTF-16 will use one or two
	U16unit (two or four bytes).
	https://en.wikipedia.org/wiki/UTF-8
	https://en.wikipedia.org/wiki/UTF-16
Do not confuse text encoded with a codepage into chars, with text encoded with UTF-8.
UTF-8 uses one to four bytes to encode all legal Unicode characters. Text in a codepage
uses single bytes to encode just 256 different characters.
*/

/** this array, though only 256 bytes long, is indexed by Unicode values, codepage
* ISO 8859-1. This code only supports space characters in this range, assuming
* all chars out of the range are not spaces. It marks only the standard C space characters.
* It does not add U+0085 NEL = Next Line, and U+00A0 Non Breaking Space.
* Note that 1252 uses 0x85 to be ellipsis, so calling that a "space" character would be,
* shall we say, "interesting." */
BASE_EXPORT extern const char sIsSpace[256];

/** This array, though only 256 bytes long, is indexed by a subset of Unicode
* values, codepage ISO 8859-1. It includes many accented characters, but it does
* not handle things out of this range. For example, U+0100 is a Latin Capital A with
* a macron (a bar over the letter). U+0101 is the corresponding lower case. That's out
* of range, so it does not get translated. 
* This is for internal handling of XML and messages, not for user data, because it
* only changes things in Latin-1 / ASCII, not any other language (Greek, Cyrillic, etc.) */
BASE_EXPORT extern const char sToLower_en[256];

/** This array, though only 256 bytes long, is indexed by a subset of Unicode
* values, codepage ISO 8859-1. It includes many accented characters, but it does
* not handle things out of this range. For example, U+00B5 is a lower case Micro, which could be
* mapped to U+039C, Capital Micro. That's out of range, so it does not get mapped. 
* This is for internal handling of XML and messages, not for user data, because it
* only changes things in Latin-1 / ASCII, not any other language (Greek, Cyrillic, etc.)  */
BASE_EXPORT extern const char sToUpper_en[256];

/** tell if a character is a basic space character. */
template <class TChar>
char IsSpace(TChar ch)
{
	// I expect the compiler will simplify the conditions when TChar is
	// an unsigned type or is a char.
	return 0 <= ch && static_cast<unsigned>(ch) < sizeof(sIsSpace) && sIsSpace[static_cast<unsigned>(ch)];
}

/** Skip over the non-space characters in the null-terminated str, returning a
* pointer to the first character that is a space, or to the null at the end.
* You can call this with 8859-1, ASCII, and UTF-16 and UTF-8. */
template <class TChar>
TChar* SkipNotSpace(TChar* str)
{
	// this is like the locale scan_is(space, str, str+len), skipping until
	// it finds a character that IS a space. I find it clearer to say that
	// it skips over the chars that are not space chars.
	// this one has to treat the null at the end of the string specially,
	// because it is "not space" but I don't want to keep skipping once I hit it.
	for (;; ++str)
	{
		if (IsSpace(*str) || !*str)
			return str;
	}
}

/** Skip over the space characters in the null-terminated string, returning a
* pointer to the first non-space, or to the null at the end
* You can call this with 8859-1, ASCII, and UTF-16 and UTF-8 */
template <class TChar>
TChar* SkipIsSpace(TChar* str)
{
	// this is like the locale scan_not(space,str, str+len), skipping until
	// it finds a character that is NOT a space. I find it clearer to say that
	// it skips over the space chars.
	// this one does not have to treat the null at the end of the string specially,
	// because it is naturally "not space"
	for (;; ++str)
	{
		if (!IsSpace(*str))
			return str;
	}
}

/** Returns the uppercase equivalent of ch. If ch is not "a lower case letter"
* in the 8859-1 codepage, the value returned is ch, unchanged.
* You can call this with 8859-1, ASCII, and UTF-16.
* DO NOT CALL THIS WITH UTF-8 text because it may confuse code points with the
* 0x80 bit set as accented letters, not the part of a multibyte character.
* This is for internal handling of XML and messages, not for user data, because it
* only changes things in Latin-1 / ASCII, not any other language (Greek, Cyrillic, etc.) */
template <class TChar>
TChar ToUpperASCII(TChar ch)
{
	return (0 <= ch && ch < 256) ? sToUpper_en[static_cast<unsigned>(ch)] : ch;
}

/** replaces any lowercase characters in the null-terminated str with its uppercase equivalent.
* Modifies str in place, and returns it if you wish to write chaining style. 
* Don't call with any const char*, because the compiler will give you stupid error messages.
* You can call this with 8859-1, ASCII, and UTF-16.
* DO NOT CALL THIS WITH UTF-8 text because it may confuse code units with the
* 0x80 bit set as accented letters, not the part of a multibyte character.
* This is for internal handling of XML and messages, not for user data, because it
* only changes things in Latin-1 / ASCII, not any other language (Greek, Cyrillic, etc.) */
template <class TChar>
inline TChar* ToUpperASCII(TChar* str)
{
	TChar* orig = str;
	for (; *str; ++str)
		*str = ToUpperASCII(*str);
	return orig;
}

/** Returns the lowercase equivalent of ch. If ch is not "an upper case letter"
* in the 8859-1 codepage, the value returned is ch, unchanged.
* You can call this with 8859-1, ASCII, and UTF-16.
* DO NOT CALL THIS WITH UTF-8 text because it may confuse code points with the
* 0x80 bit set as accented letters, not the part of a multibyte character.
* This is for internal handling of XML and messages, not for user data, because it
* only changes things in Latin-1 / ASCII, not any other language (Greek, Cyrillic, etc.) */
template <class TChar>
TChar ToLowerASCII(TChar ch)
{
	return (0 <= ch && ch < 256) ? sToLower_en[static_cast<unsigned>(ch)] : ch;
}

/** replaces any uppercase characters in the null-terminated str with its lowercase equivalent.
* Modifies str in place, and returns it if you wish to write chaining style.
* Don't call with any const char*, because the compiler will give you stupid error messages.
* You can call this with 8859-1, ASCII, and UTF-16.
* DO NOT CALL THIS WITH UTF-8 text because it may confuse code units with the
* 0x80 bit set as accented letters, not the part of a multibyte character.
* This is for internal handling of XML and messages, not for user data, because it
* only changes things in Latin-1 / ASCII, not any other language (Greek, Cyrillic, etc.) */
template <class TChar>
inline TChar* ToLowerASCII(TChar* str)
{
	TChar* orig = str;
	for (; *str; ++str)
		*str = ToLower(*str);
	return orig;
}

}}  // namespace SRC::CType
