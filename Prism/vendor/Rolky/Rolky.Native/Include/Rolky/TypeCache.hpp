#pragma once

#include "Core.hpp"
#include "StableVector.hpp"

namespace Rolky {
	class Type;

	class [[deprecated(ROLKY_GLOBAL_ALC_MSG)]] TypeCache
	{
	public:
		[[deprecated(ROLKY_GLOBAL_ALC_MSG)]]
		static TypeCache& Get();

		[[deprecated(ROLKY_GLOBAL_ALC_MSG)]]
		Type* CacheType(Type&& InType);

		[[deprecated(ROLKY_GLOBAL_ALC_MSG_P(ManagedAssembly::GetLocalType))]]
		Type* GetTypeByName(std::string_view InName) const;

		[[deprecated(ROLKY_GLOBAL_ALC_MSG)]]
		Type* GetTypeByID(TypeId InTypeID) const;

		[[deprecated(ROLKY_GLOBAL_ALC_MSG)]]
		void Clear();

	private:
		StableVector<Type> m_Types;
		std::unordered_map<std::string, Type*> m_NameCache;
		std::unordered_map<TypeId, Type*> m_IDCache;
	};

}
