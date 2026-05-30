#include "prpch.h"
#include "Ref.h"

namespace Prism
{
    static std::unordered_set<void*> s_LiveReferences;
    static std::mutex s_LiveReferenceMutex;

    namespace RefUtils {

        void AddToLiveReferences(void* instance)
        {
            //std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            PR_CORE_ASSERT(instance);
            s_LiveReferences.insert(instance);
        }

        void RemoveFromLiveReferences(void* instance)
        {
            //std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            PR_CORE_ASSERT(instance);
            PR_CORE_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end());
            s_LiveReferences.erase(instance);
        }

        bool IsLive(void* instance)
        {
            PR_CORE_ASSERT(instance);
            return s_LiveReferences.find(instance) != s_LiveReferences.end();
        }

        size_t PRISM_API GetLiveReferenceCount()
        {
            //std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            return s_LiveReferences.size();
        }

    }

}