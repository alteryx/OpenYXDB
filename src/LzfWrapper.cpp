#include "stdafx.h"

#include "Base/LzfWrapper.h"  //Leave the Base here else R build will break

extern "C"
{
#include "lzf.h"
}

namespace SRC { namespace LzfWrapper {

unsigned int lzf_compress(const void* const in_data, unsigned int in_len, void* out_data, unsigned int out_len)
{
	return ::lzf_compress(in_data, in_len, out_data, out_len);
}

unsigned int lzf_decompress(const void* const in_data, unsigned int in_len, void* out_data, unsigned int out_len)
{
	return ::lzf_decompress(in_data, in_len, out_data, out_len);
}

}}  // namespace SRC::LzfWrapper
