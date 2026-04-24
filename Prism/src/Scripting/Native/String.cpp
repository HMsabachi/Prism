// NativeString.cpp
#include "prpch.h"
#include "String.h"
#include <cstring>
#include <cstdlib>

namespace Prism::Native
{
	String CreateNativeString(const char* cstr)
	{
		String str = { nullptr, 0, 0 };

		if (cstr == nullptr) {
			return str;
		}

		size_t len = strlen(cstr);
		str.data = (char*)malloc(len + 1);
		if (str.data != nullptr) {
			strcpy(str.data, cstr);
			str.length = (uint32_t)len;
			str.capacity = (uint32_t)len + 1;
		}

		return str;
	}

	const char* NativeStringToCString(const String* str)
	{
		if (str == nullptr || str->data == nullptr) {
			return "";
		}
		return str->data;
	}

	void FreeNativeString(String* str)
	{
		if (str != nullptr && str->data != nullptr) {
			free(str->data);
			str->data = nullptr;
			str->length = 0;
			str->capacity = 0;
		}
	}

	String CopyNativeString(const String* src)
	{
		String str = { nullptr, 0, 0 };

		if (src == nullptr || src->data == nullptr) {
			return str;
		}

		str.data = (char*)malloc(src->length + 1);
		if (str.data != nullptr) {
			strcpy(str.data, src->data);
			str.length = src->length;
			str.capacity = src->length + 1;
		}

		return str;
	}
}