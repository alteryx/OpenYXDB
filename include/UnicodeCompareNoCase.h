#pragma once

#include "Base/Base_ImpExp.h"
#include "Base/U16unit.h"

namespace SRC { namespace CompareNoCaseUnicode {
BASE_EXPORT int CompareNoCaseUTF16(const U16unit* pA, const U16unit* pB);
BASE_EXPORT int CompareNoCaseUTF16(const U16unit* pA, const U16unit* pB, int N);

}}  // namespace SRC::CompareNoCaseUnicode
