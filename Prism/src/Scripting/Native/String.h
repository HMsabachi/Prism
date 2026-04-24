// NativeString.h
#pragma once

namespace Prism::Native
{
	struct String
	{
		char* data;
		uint32_t length;
		uint32_t capacity;
	};

	// NativeString相关函数
	String CreateNativeString(const char* cstr);
	const char* NativeStringToCString(const String* str);
	void FreeNativeString(String* str);
	String CopyNativeString(const String* src);
}