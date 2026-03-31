#pragma once
#include "Prism/Core/Core.h"
#include <string>

namespace Prism
{
	class PRISM_API Texture
	{
	public:
		virtual ~Texture() = default;

		virtual int GetWidth() const = 0;
		virtual int GetHeight() const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetData(void* data, uint32_t size) const = 0;
	};
	class PRISM_API Texture2D : public Texture
	{
	public:
		virtual ~Texture2D() = default;
		static void Init();
	public:
		static Ref<Texture2D> ErrorTexture;
	public:
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(const uint32_t width, const uint32_t height);

	};
}