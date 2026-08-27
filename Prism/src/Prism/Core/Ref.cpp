#include "prpch.h"
#include "Ref.h"
#include "Application.h"

namespace Prism
{
    static std::unordered_set<RefCounted*> s_LiveReferences;
    static std::mutex s_LiveReferenceMutex;

    namespace RefUtils
    {
        void PRISM_API AddToLiveReferences(RefCounted* instance)
        {
            PR_CORE_ASSERT(instance);
            std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            s_LiveReferences.insert(instance);
        }

        void PRISM_API RemoveFromLiveReferences(RefCounted* instance)
        {
            PR_CORE_ASSERT(instance);
            std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            auto it = s_LiveReferences.find(instance);
            PR_CORE_ASSERT(it != s_LiveReferences.end());
            s_LiveReferences.erase(it);
        }

        bool PRISM_API IsLive(RefCounted* instance)
        {
            PR_CORE_ASSERT(instance);
            return s_LiveReferences.find(instance) != s_LiveReferences.end();
        }

        size_t PRISM_API GetLiveReferenceCount()
        {
            return s_LiveReferences.size();
        }
    }
}
