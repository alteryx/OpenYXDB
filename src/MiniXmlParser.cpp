#include "stdafx.h"

#include "Base/MiniXmlParser.h"
#ifndef SRCLIB_REPLACEMENT
	#include "Base/Src_Error.h"
	#include "Base/U16unit.h"
	#include "Base/UString.h"
	#include "Base/FastOutBuffer.h"
	#include "Base/StringToDouble.h"
#endif

namespace MINIXML_NAMESPACE {
const char pStr_CDATA[] = { '!', '[', 'C', 'D', 'A', 'T', 'A', '[', '\0' };
const char pStr_amp[] = { 'a', 'm', 'p', ';', '\0' };
const char pStr_quot[] = { 'q', 'u', 'o', 't', ';', '\0' };
const char pStr_apos[] = { 'a', 'p', 'o', 's', ';', '\0' };
const char pStr_gt[] = { 'g', 't', ';', '\0' };
const char pStr_lt[] = { 'l', 't', ';', '\0' };
const char pStr_xA[] = { '#', 'x', 'A', ';', '\0' };
const char pStr_xD[] = { '#', 'x', 'D', ';', '\0' };
const SRC::String::TChar pStr_value[] = { 'v', 'a', 'l', 'u', 'e', '\0' };
const SRC::String::TChar pStr_openbracket[] = { '<', '\0' };
const SRC::String::TChar pStr_openbracketforwardslash[] = { '<', '/', '\0' };

///////////////////////////////////////////////////////////////////////////////
//	class MiniXmlParser

#ifdef SRCLIB_REPLACEMENT
static SRC::WString EscapeAttributeOld(SRC::WString strVal)
{
	return strVal.ReplaceString(U16("&"), U16("&amp;"))
		.ReplaceString(U16("\""), U16("&quot;"))
		.ReplaceString(U16(">"), U16("&gt;"))
		.ReplaceString(U16("<"), U16("&lt;"))
		.ReplaceString(U16("'"), U16("&apos;"));
}
static SRC::AString EscapeAttributeOld(SRC::AString strVal)
{
	return strVal.ReplaceString("&", "&amp;")
		.ReplaceString("\"", "&quot;")
		.ReplaceString(">", "&gt;")
		.ReplaceString("<", "&lt;")
		.ReplaceString("'", "&apos;");
}
#endif

// strncmp for keywords, which will only be given as char*
template <class LChar>
static int keyncmp(const LChar* pA, const char* pB, size_t nLength)
{
	const LChar* pEnd = pA + nLength - 1;
	while (*pA && (pA < pEnd) && *pA == *pB)
		pA++, pB++;
	return *(pA) - *(pB);
}

template <class TString>
TString TEscapeAttribute(TString strVal)
{
	if (strVal.IsEmpty())
		return strVal;
#ifdef SRCLIB_REPLACEMENT
	return EscapeAttributeOld(strVal);
#else
	const typename TString::TChar* pInput = strVal.c_str();

	SRC::TFastOutBuffer<TString> outBuffer;
	bool bChanged = false;
	const typename TString::TChar* p = pInput;
	for (; *p != 0; ++p)
	{
		switch (*p)
		{
			case '&':
			case '\"':
			case '<':
			case '>':
			case '\'':
				bChanged = true;
				if (p != pInput)
					outBuffer.Append(pInput, unsigned(p - pInput));
				pInput = p + 1;
				outBuffer += '&';
				switch (*p)
				{
					case '&':
						outBuffer += 'a';
						outBuffer += 'm';
						outBuffer += 'p';
						break;
					case '\"':
						outBuffer += 'q';
						outBuffer += 'u';
						outBuffer += 'o';
						outBuffer += 't';
						break;
					case '<':
						outBuffer += 'l';
						outBuffer += 't';
						break;
					case '>':
						outBuffer += 'g';
						outBuffer += 't';
						break;
					case '\'':
						outBuffer += 'a';
						outBuffer += 'p';
						outBuffer += 'o';
						outBuffer += 's';
						break;
				}
				outBuffer += ';';
		}
	}
	if (bChanged)
	{
		if (p != pInput)
			outBuffer.Append(pInput, unsigned(p - pInput));
		return outBuffer.GetString();
	}
	else
		return strVal;
#endif
}

#if defined(E2)
UString MiniXmlParser::EscapeAttribute(UString strVal)
{
	return TEscapeAttribute(strVal);
}
SRC::WString MiniXmlParser::EscapeAttribute(SRC::WString strVal)
{
	return TEscapeAttribute(strVal);
}
#else
String MiniXmlParser::EscapeAttribute(WString strVal)
{
	return TEscapeAttribute(strVal);
}
String MiniXmlParser::EscapeAttribute(const String::TChar* pVal)
{
	return EscapeAttribute(String(pVal));
}
#endif
#if !defined(E2LINUX) && !defined(E2WINDOWS)
AString MiniXmlParser::EscapeAttribute(const char* pVal)
{
	return EscapeAttribute(SRC::AString(pVal));
}
#endif

#if !defined(E2)
AString MiniXmlParser::EscapeAttribute(AString strVal)
{
	return TEscapeAttribute(strVal);
}
#endif

template <typename StrT>
StrT TUnescapeAttribute(StrT strVal, bool bAllowCdata /*= false*/)
{
	if (strVal.IsEmpty())
		return strVal;

	// if we don't need to unescape return string without copying
	unsigned nNoEscapeNeededLength = 0;
	const typename StrT::TChar* pSrcConst = strVal.c_str();
	unsigned nInputLength = strVal.Length();

	for (; nNoEscapeNeededLength < nInputLength; ++nNoEscapeNeededLength)
	{
		typename StrT::TChar c = pSrcConst[nNoEscapeNeededLength];
		if (c == '&' || (bAllowCdata && c == '<' && keyncmp(pSrcConst + nNoEscapeNeededLength + 1, pStr_CDATA, 8) == 0))
			break;
	}

	if (nNoEscapeNeededLength == nInputLength)
		return strVal;

	typename StrT::TChar* pSrc = strVal.Lock() + nNoEscapeNeededLength;
	typename StrT::TChar* pDest = pSrc;
	while (auto c = *pSrc)
	{
		if (bAllowCdata && c == '<' && keyncmp(pSrc + 1, pStr_CDATA, 8) == 0)
		{
			pSrc += 9;
			bool bFoundEndCdata = false;
			while (auto c2 = *pSrc)
			{
				if (c2 == ']' && pSrc[1] == ']' && pSrc[2] == '>')
				{
					pSrc += 3;
					bFoundEndCdata = true;
					break;
				}
				else
				{
					*pDest++ = c2;
					pSrc++;
				}
			}
			if (!bFoundEndCdata)
				throw Error(XMSG("An unclosed CDATA was found."));
		}
		else if (c == '&')
		{
			if (keyncmp(pSrc + 1, pStr_amp, 4) == 0)
			{
				*pDest++ = '&';
				pSrc += 5;
			}
			else if (keyncmp(pSrc + 1, pStr_quot, 5) == 0)
			{
				*pDest++ = '\"';
				pSrc += 6;
			}
			else if (keyncmp(pSrc + 1, pStr_apos, 5) == 0)
			{
				*pDest++ = '\'';
				pSrc += 6;
			}
			else if (keyncmp(pSrc + 1, pStr_gt, 3) == 0)
			{
				*pDest++ = '>';
				pSrc += 4;
			}
			else if (keyncmp(pSrc + 1, pStr_lt, 3) == 0)
			{
				*pDest++ = '<';
				pSrc += 4;
			}
			else if (keyncmp(pSrc + 1, pStr_xA, 4) == 0)
			{
				*pDest++ = '\n';
				pSrc += 5;
			}
			else if (keyncmp(pSrc + 1, pStr_xD, 4) == 0)
			{
				*pDest++ = '\r';
				pSrc += 5;
			}
			else
			{
				*pDest++ = c;
				pSrc++;
			}
		}
		else
		{
			*pDest++ = c;
			pSrc++;
		}
	}
	*pDest = 0;
	strVal.Unlock();
	return strVal;
}
#ifndef SRCLIB_REPLACEMENT
UString MiniXmlParser::UnescapeAttribute(UString strVal, bool bAllowCdata)
{
	return TUnescapeAttribute(std::move(strVal), bAllowCdata);
}
#endif
SRC::WString MiniXmlParser::UnescapeAttribute(SRC::WString strVal, bool bAllowCdata)
{
	return TUnescapeAttribute(std::move(strVal), bAllowCdata);
}
void MiniXmlParser::TagInfo::InitAttributes() const
{
	if (!m_bAttributeInit)
	{
		m_vAttributes.clear();
		// the 1st attrubute can't start before the 4th char <a a="...
		for (const SRC::String::TChar* p = m_pXmlStart + 3; *p != '>' && *p != '/' && p < m_pXmlEnd; p++)
		{
			SRC::String::TChar cPrev = p[-1];
			if (cPrev == ' ' || cPrev == '\t' || cPrev == '\n')
			{
				SRC::String::TChar c = p[0];
				if (c != ' ' && c != '\t' && c != '\n')
				{
					// we have the start of an attribute
					TagInfo::Attribute att;
					att.name.first = p;
					att.name.second = p + 1;
					for (;; ++att.name.second)
					{
						if (att.name.second == m_pXmlEnd)
							throw Error(XMSG(
								"Bad XML in reading the name of an attribute: @1",
								String(att.name.first, static_cast<int>(att.name.second - att.name.first))));

						SRC::String::TChar c2 = *att.name.second;
						if (c2 == ' ' || c2 == '\t' || c2 == '\n' || c2 == '=')
							break;
					}
					att.value.first = att.name.second + 1;
					SRC::String::TChar cQuote;
					for (;; ++att.value.first)
					{
						if (att.value.first == m_pXmlEnd)
							throw Error(XMSG(
								"Bad XML seeking the starting quote for the value of attribute @1",
								String(att.name.first, static_cast<int>(att.name.second - att.name.first))));

						SRC::String::TChar c2 = att.value.first[-1];
						if (c2 == '\"' || c2 == '\'')
						{
							cQuote = c2;
							break;
						}
					}

					att.value.second = att.value.first;
					for (;; ++att.value.second)
					{
						if (att.value.second == m_pXmlEnd)
							throw Error(XMSG(
								"Bad XML seeking the ending quote (@1) for the value of attribute @2",
								String(&cQuote, 1),
								String(att.name.first, static_cast<int>(att.name.second - att.name.first))));

						SRC::String::TChar c2 = *att.value.second;
						if (c2 == cQuote)
							break;
					}

					m_vAttributes.push_back(att);
					p = att.value.second;
				}
			}
		}
		m_bAttributeInit = true;
	}
}

String MiniXmlParser::GetAttribute(
	const TagInfo& xmlTag,
	const SRC::String::TChar* pAttributeName,
	bool bThrowErrorIfMissing)
{
	if (!xmlTag.m_bAttributeInit)
		xmlTag.InitAttributes();

	int nLen = int(SRC::String::strlen(pAttributeName));
	for (std::vector<TagInfo::Attribute>::const_iterator it = xmlTag.m_vAttributes.begin();
		 it != xmlTag.m_vAttributes.end();
		 ++it)
	{
		if (nLen == (it->name.second - it->name.first)
			&& SRC::String::strncmp(pAttributeName, it->name.first, nLen) == 0)
		{
			int nAtrributeLen = int(it->value.second - it->value.first);
			String strRet{ it->value.first, nAtrributeLen };
			return UnescapeAttribute(strRet);
		}
	}

	if (bThrowErrorIfMissing)
		throw SRC::Error(XMSG("XmlParse Error: the attribute \"@1\" is missing.", pAttributeName));

	return String();
}

// Returns true and sets value if the attribute is set; returns false and does not touch value if attr isn't set
/*static*/ bool MiniXmlParser::GetAttribute(
	const TagInfo& xmlTag,
	const SRC::String::TChar* pAttributeName,
	String& value)
{
	if (!xmlTag.m_bAttributeInit)
		xmlTag.InitAttributes();

	int nLen = int(SRC::String::strlen(pAttributeName));
	for (const TagInfo::Attribute& attr : xmlTag.m_vAttributes)
	{
		if (nLen == (attr.name.second - attr.name.first)
			&& SRC::String::strncmp(pAttributeName, attr.name.first, nLen) == 0)
		{
			value.Assign(
				static_cast<const SRC::String::TChar*>(attr.value.first), int(attr.value.second - attr.value.first));
			value = UnescapeAttribute(std::move(value));
			return true;
		}
	}
	return false;
}

/*static*/ String MiniXmlParser::GetAttributeDefault(
	const TagInfo& xmlTag,
	const SRC::String::TChar* pAttributeName,
	String strDefault)
{
	String attrVal;
	if (GetAttribute(xmlTag, pAttributeName, attrVal))
		return attrVal;
	return strDefault;
}

/*static*/ int MiniXmlParser::GetAttributeDefault(
	const TagInfo& xmlTag,
	const SRC::String::TChar* pAttributeName,
	int nDefault)
{
	String strRet = GetAttribute(xmlTag, pAttributeName, false);
	if (strRet.IsEmpty())
		return nDefault;
	return SRC::StringHelper::sh_strtoi(strRet, nullptr, 0);
}
/*static*/ bool MiniXmlParser::GetAttributeDefault(
	const TagInfo& xmlTag,
	const SRC::String::TChar* pAttributeName,
	bool bDefault)
{
	String strRet = GetAttribute(xmlTag, pAttributeName, false);
	if (strRet.IsEmpty())
		return bDefault;
	else
		return 'T' == toupper(*strRet);
}

bool MiniXmlParser::FindXmlTag(
	const TagInfo& xmlTag,
	TagInfo& r_xmlElementTag,
	const SRC::String::TChar* pRequestedTagName)
{
	if (xmlTag.m_pXmlStart == NULL || *xmlTag.m_pXmlStart == 0)
		return false;

	r_xmlElementTag.m_bAttributeInit = false;

	r_xmlElementTag.m_pXmlStart = xmlTag.m_pXmlStart;

	String strStartTag = pStr_openbracket;
	String sTagName;
	if (pRequestedTagName)
	{
		sTagName = pRequestedTagName;
		strStartTag += pRequestedTagName;
	}
	else
	{
		r_xmlElementTag.m_pXmlStart++;
	}

	for (;;)
	{
		r_xmlElementTag.m_pXmlStart = SRC::String::strstr(r_xmlElementTag.m_pXmlStart, strStartTag);
		if (!r_xmlElementTag.m_pXmlStart || (r_xmlElementTag.m_pXmlStart + strStartTag.Length()) >= xmlTag.m_pXmlEnd)
			return false;

		if (sTagName.IsEmpty())
		{
			const SRC::String::TChar* pNameStart = r_xmlElementTag.m_pXmlStart + 1;
			unsigned nNameLen = 0;
			while (iswalnum(pNameStart[nNameLen]) || pNameStart[nNameLen] == L'_')
			{
				nNameLen++;
			}
			sTagName.Assign(pNameStart, nNameLen);
			strStartTag += sTagName;
			break;
		}

		SRC::String::TChar cNext = r_xmlElementTag.m_pXmlStart[strStartTag.Length()];
		if (iswalnum(cNext) || cNext == L'_')
			r_xmlElementTag.m_pXmlStart += strStartTag.Length();
		else
			break;
	}

	r_xmlElementTag.m_pXmlEnd = r_xmlElementTag.m_pXmlStart + strStartTag.Length();

	ScanForTagEnd(r_xmlElementTag.m_pXmlEnd);

	if (!*r_xmlElementTag.m_pXmlEnd)
		throw SRC::Error(XMSG("XmlParse Error: the element @1 is not properly formatted.", sTagName));

	if (*(r_xmlElementTag.m_pXmlEnd - 1) != '/')
	{
		r_xmlElementTag.m_pXmlEnd = r_xmlElementTag.m_pXmlStart + 1;

		String strCloseTag = String(pStr_openbracketforwardslash) + sTagName;

		for (;;)
		{
			const SRC::String::TChar* pNextClose = SRC::String::strstr(r_xmlElementTag.m_pXmlEnd, strCloseTag);

			if (pNextClose == nullptr)
				throw Error(XMSG("The tag \"@1\" is not properly closed.", pRequestedTagName));
#pragma warning(suppress : 28182)
			while (pNextClose
				   && (isalnum(pNextClose[strCloseTag.Length()]) || pNextClose[strCloseTag.Length()] == '_'
					   || pNextClose[strCloseTag.Length()] == '-' || pNextClose[strCloseTag.Length()] == '.'))
				pNextClose = SRC::String::strstr(pNextClose + 1, strCloseTag);
			if (!pNextClose)
				throw SRC::Error(XMSG("XmlParse Error (1): the element @1 is not properly closed.", sTagName));

			const SRC::String::TChar* pNextCloseEnd = pNextClose + strCloseTag.Length();
			while ((pNextCloseEnd < xmlTag.m_pXmlEnd) && *pNextCloseEnd != '>' && *pNextCloseEnd != 0)
				pNextCloseEnd++;

			if (!pNextCloseEnd)
				throw SRC::Error(XMSG("XmlParse Error (1): the element @1 is not properly closed.", sTagName));

			if ((pNextCloseEnd >= xmlTag.m_pXmlEnd) || *pNextCloseEnd != '>')
				throw SRC::Error(XMSG("XmlParse Error (2): the element @1 is not properly closed.", sTagName));

			pNextCloseEnd++;  // increment past the >

			TagInfo tagEmbedded;
			// skip past any embedded tags
			if (FindXmlTag(TagInfo(r_xmlElementTag.m_pXmlEnd, pNextCloseEnd), tagEmbedded, sTagName)
				&& tagEmbedded.m_pXmlStart < pNextClose)
			{
				r_xmlElementTag.m_pXmlEnd = tagEmbedded.m_pXmlEnd;
			}
			else
			{
				r_xmlElementTag.m_pXmlEnd = pNextCloseEnd;
				break;
			}
		}
	}
	else
		r_xmlElementTag.m_pXmlEnd++;  // move the end past the closing >
	if (r_xmlElementTag.m_pXmlEnd > xmlTag.m_pXmlEnd)
		throw SRC::Error(XMSG("XmlParse Error (3): the element @1 is not properly closed.", sTagName));
	return true;
}

/*static*/ bool MiniXmlParser::FindFirstChildXmlTag(const TagInfo& xmlTag, TagInfo& r_xmlElementTag)
{
	return FindXmlTag(xmlTag, r_xmlElementTag, NULL);
}

/*static*/ bool MiniXmlParser::FindNextXmlTag(
	const TagInfo& xmlTag,
	TagInfo& r_xmlElementTag,
	const SRC::String::TChar* pTagName)
{
	TagInfo xmlTagNew(xmlTag);
	xmlTagNew.m_pXmlStart = r_xmlElementTag.m_pXmlEnd;
	return FindXmlTag(xmlTagNew, r_xmlElementTag, pTagName);
}

void MiniXmlParser::ScanForTagEnd(const SRC::String::TChar*& pXmlStart)
{
	// scan for the end of the tag
	SRC::String::TChar cCurrentQuote = 0;
	while (*pXmlStart)
	{
		SRC::String::TChar c = *pXmlStart;
		if (cCurrentQuote == 0)
		{
			if (c == '\"' || c == '\'')
			{
				cCurrentQuote = c;
			}
			else if (c == '>')
			{
				break;
			}
		}
		else if (c == cCurrentQuote)
		{
			cCurrentQuote = 0;
		}

		++pXmlStart;
	}
}

String MiniXmlParser::GetInnerXml(const TagInfo& xmlTag, bool bUnescapeValue, bool bTrim)
{
	String strRet;
	const SRC::String::TChar* pXmlStart = xmlTag.m_pXmlStart;

	ScanForTagEnd(pXmlStart);

	++pXmlStart;

	// scan to the beginning of the close
	const SRC::String::TChar* pXmlEnd = xmlTag.m_pXmlEnd - 1;  // -1 because it always point 1 after the end of the tag
	while (pXmlEnd > pXmlStart && *pXmlEnd != L'<')
		--pXmlEnd;

	if (*pXmlStart && pXmlStart < pXmlEnd)
	{
		int nLen = int(pXmlEnd - pXmlStart);
		strRet.Assign(pXmlStart, nLen);
	}
	// we always trim 1st, because the a CDATA section could contain trailing spaces
	if (bTrim)
		strRet.Trim();
	if (bUnescapeValue)
		strRet = UnescapeAttribute(strRet, true);

	return strRet;
}
String MiniXmlParser::GetOuterXml(TagInfo xmlTag)
{
	String strRet;
	if (xmlTag.m_pXmlStart != NULL)
	{
		unsigned nLen = unsigned(xmlTag.m_pXmlEnd - xmlTag.m_pXmlStart);
		strRet.Assign(xmlTag.m_pXmlStart, nLen);
	}
	return strRet;
}

/*static*/ bool MiniXmlParser::FindXmlTagWithAttribute(
	const TagInfo& xmlTag,
	TagInfo& r_xmlElementTag,
	const SRC::String::TChar* pTagName,
	const SRC::String::TChar* pAttributeName,
	const SRC::String::TChar* pAttributeValue)
{
	TagInfo xmlTagStart(xmlTag);
	for (;;)
	{
		if (!FindXmlTag(xmlTagStart, r_xmlElementTag, pTagName))
			return false;
		if (GetAttribute(r_xmlElementTag, pAttributeName, false) == pAttributeValue)
			return true;
		xmlTagStart.m_pXmlStart = r_xmlElementTag.m_pXmlEnd;
	}
}
/*static*/ String MiniXmlParser::GetFromXml(
	const TagInfo& xmlTagParent,
	const SRC::String::TChar* strElementName,
	const SRC::String::TChar* strDefault,
	bool bThrowErrorIfMissing)
{
	TagInfo xmlTag;
	if (!FindXmlTag(xmlTagParent, xmlTag, strElementName))
	{
		if (bThrowErrorIfMissing)
			throw Error(XMSG("Missing value @1.", strElementName));

		return strDefault;
	}
	return GetInnerXml(xmlTag, true);
}
/*static*/ bool MiniXmlParser::GetFromXml(
	const TagInfo& xmlTagParent,
	const SRC::String::TChar* strElementName,
	bool bDefault,
	bool bThrowErrorIfMissing)
{
	TagInfo xmlTag;
	if (!FindXmlTag(xmlTagParent, xmlTag, strElementName))
	{
		if (bThrowErrorIfMissing)
			throw Error(XMSG("Missing value @1.", strElementName));

		return bDefault;
	}
	String strValue = GetAttribute(xmlTag, pStr_value, bThrowErrorIfMissing);
	SRC::String::TChar c = *strValue.c_str();

	if (iswdigit(c) || c == '-')
		return 0.0 != SRC::ConvertToDouble(strValue.c_str());
	return 'T' == toupper(c);
}
/*static*/ int MiniXmlParser::GetFromXml(
	const TagInfo& xmlTagParent,
	const SRC::String::TChar* strElementName,
	int iDefault,
	bool bThrowErrorIfMissing)
{
	TagInfo xmlTag;
	if (!FindXmlTag(xmlTagParent, xmlTag, strElementName))
	{
		if (bThrowErrorIfMissing)
			throw Error(XMSG("Missing value @1.", strElementName));

		return iDefault;
	}
	return SRC::StringHelper::sh_strtoi(GetAttribute(xmlTag, pStr_value, bThrowErrorIfMissing), nullptr, 0);
}
/*static*/ int64_t MiniXmlParser::GetFromXml(
	const TagInfo& xmlTagParent,
	const SRC::String::TChar* strElementName,
	int64_t iDefault,
	bool bThrowErrorIfMissing)
{
	TagInfo xmlTag;
	if (!FindXmlTag(xmlTagParent, xmlTag, strElementName))
	{
		if (bThrowErrorIfMissing)
			throw Error(XMSG("Missing value @1.", strElementName));

		return iDefault;
	}
	return SRC::StringHelper::sh_strtoi64(GetAttribute(xmlTag, pStr_value, bThrowErrorIfMissing), nullptr, 0);
}
/*static*/ double MiniXmlParser::GetFromXml(
	const TagInfo& xmlTagParent,
	const SRC::String::TChar* strElementName,
	double dDefault,
	bool bThrowErrorIfMissing)
{
	TagInfo xmlTag;
	if (!FindXmlTag(xmlTagParent, xmlTag, strElementName))
	{
		if (bThrowErrorIfMissing)
			throw Error(XMSG("Missing value @1.", strElementName));

		return dDefault;
	}
	return GetAttribute(xmlTag, pStr_value, bThrowErrorIfMissing).ConvertToDouble();
}

const std::vector<MiniXmlParser::TagInfo::Attribute>& MiniXmlParser::TagInfo::Attributes() const&
{
	InitAttributes();
	return m_vAttributes;
}

MiniXmlParser::TagInfo::TagInfo(const SRC::String::TChar* pStart, const SRC::String::TChar* pEnd)
	: m_bAttributeInit(false)
	, m_pXmlStart(pStart)
	, m_pXmlEnd(pEnd)
{
	if (m_pXmlStart != nullptr && pEnd == nullptr)
		m_pXmlEnd = m_pXmlStart + SRC::String::strlen(m_pXmlStart);
}

MiniXmlParser::TagInfo::TagInfo(String strXml)
	: m_strXml(std::move(strXml))
	, m_bAttributeInit(false)
{
	m_pXmlStart = m_strXml.c_str();
	m_pXmlEnd = m_pXmlStart + m_strXml.Length();
}

MiniXmlParser::TagInfo& MiniXmlParser::TagInfo::operator=(String strXml)
{
	m_bAttributeInit = false;
	m_strXml = std::move(strXml);
	m_pXmlStart = m_strXml.c_str();
	m_pXmlEnd = m_pXmlStart + m_strXml.Length();
	return *this;
}

const SRC::String::TChar* MiniXmlParser::TagInfo::Start() const
{
	return m_pXmlStart;
}

const SRC::String::TChar* MiniXmlParser::TagInfo::End() const
{
	return m_pXmlEnd;
}

}  // namespace MINIXML_NAMESPACE
