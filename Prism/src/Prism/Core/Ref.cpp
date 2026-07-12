#include "prpch.h"
#include "Ref.h"
#include "Application.h"

namespace Prism
{
    static std::unordered_set<void*> s_LiveReferences;
    static std::mutex s_LiveReferenceMutex;

    namespace RefUtils {

        void PRISM_API AddToLiveReferences(void* instance)
        {
            std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            PR_CORE_ASSERT(instance);
            s_LiveReferences.insert(instance);
        }

        void PRISM_API RemoveFromLiveReferences(void* instance)
        {
            std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            if (Application::Get().IsRunning() != 1) [[unlikely]] return; // Avoid assertions during shutdown
            PR_CORE_ASSERT(instance);
            PR_CORE_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end());
            s_LiveReferences.erase(instance);
        }

        bool PRISM_API IsLive(void* instance)
        {
            PR_CORE_ASSERT(instance);
            return s_LiveReferences.find(instance) != s_LiveReferences.end();
        }

        size_t PRISM_API GetLiveReferenceCount()
        {
            std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
            return s_LiveReferences.size();
        }

    }

}
