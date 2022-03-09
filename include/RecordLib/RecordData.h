// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56549
// include guards prevent redefiniton errors while building e2
#ifndef XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_RECORDDATA_H
#define XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_RECORDDATA_H

namespace SRC {
struct RecordData
{
};

inline char* ToCharP(RecordData* p)
{
	return reinterpret_cast<char*>(p);
}

inline const char* ToCharP(const RecordData* p)
{
	return reinterpret_cast<const char*>(p);
}
}  // namespace SRC

#endif /* XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_RECORDDATA_H */
