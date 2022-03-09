#pragma once

#include "Base/Base_ImpExp.h"

//Pass-through wrapper for lzf
namespace SRC { namespace LzfWrapper {

BASE_EXPORT unsigned int lzf_compress(
	const void* const in_data,
	unsigned int in_len,
	void* out_data,
	unsigned int out_len);
BASE_EXPORT unsigned int lzf_decompress(
	const void* const in_data,
	unsigned int in_len,
	void* out_data,
	unsigned int out_len);

}}  // namespace SRC::LzfWrapper
