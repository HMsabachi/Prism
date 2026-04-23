// NativeString.h
#pragma once

namespace Prism::Native
{
	struct NativeString
	{
		char* data;
		uint32_t length;
		uint32_t capacity;
	};

	// NativeString相关函数
	NativeString CreateNativeString(const char* cstr);
	const char* NativeStringToCString(const NativeString* str);
	void FreeNativeString(NativeString* str);
	NativeString CopyNativeString(const NativeString* src);
}