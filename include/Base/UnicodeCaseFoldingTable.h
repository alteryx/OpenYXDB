#pragma once

#include "Base/Base_ImpExp.h"

// This is the unicode CaseFolding table generated from http://www.unicode.org/Public/UCD/latest/ucd/CaseFolding.txt
// It has been automatically generated by the Alteryx module Generate_UnicodeCaseFoldingTable_h.yxmd which should be in the same folder as this source file
// If you need to update it then run that module pointing it to the unicode version you want to use
// It is the Full case folding table in that it contains the entries with status C and F
// Codes with the F status have multiple entries in this table.  You must combine all of the second parts of the pairs for a code to get the full case folding mapping
// There is a sentinel value at the end of the table so you do not need to test for iter==end after a lookup
namespace SRC {
struct UnicodeCaseMappingTable
{
	static const unsigned S_Size = 1650;
	static const unsigned S_MaxNumCPInMapping = 3 + 1;  // for holding 0 terminant
	static BASE_EXPORT const std::pair<char32_t, char32_t> S_Table[S_Size + 1];
};
}  // namespace SRC
