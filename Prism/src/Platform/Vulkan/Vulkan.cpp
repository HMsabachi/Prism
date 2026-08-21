#include "prpch.h"
#include "Platform/Vulkan/Vulkan.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Prism::Utils
{
    static const char* StageToString(VkPipelineStageFlagBits stage)
    {
        switch (stage)
        {
            case VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT: return "VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT";
            case VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT: return "VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT";
        }
        PR_CORE_ASSERT(false);
        return nullptr;
    }

    static void RetrieveQueueCheckpoints(VkQueue queue, const char* queueName)
    {
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
        auto getQueueCheckpointData = (PFN_vkGetQueueCheckpointDataNV)vkGetDeviceProcAddr(device, "vkGetQueueCheckpointDataNV");
        if (!getQueueCheckpointData)
            return;

        const uint32_t checkpointCount = 4;
        VkCheckpointDataNV data[checkpointCount];
        for (uint32_t i = 0; i < checkpointCount; i++)
            data[i].sType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV;

        uint32_t retrievedCount = checkpointCount;
        getQueueCheckpointData(queue, &retrievedCount, data);
        PR_CORE_ERROR("RetrieveDiagnosticCheckpoints ({0}):", queueName);
        for (uint32_t i = 0; i < retrievedCount; i++)
        {
            VulkanCheckpointData* checkpoint = (VulkanCheckpointData*)data[i].pCheckpointMarker;
            PR_CORE_ERROR("Checkpoint: {0} (stage: {1})", checkpoint->Data, StageToString(data[i].stage));
        }
    }

    void RetrieveDiagnosticCheckpoints()
    {
        Ref<VulkanDevice> device = VulkanContext::GetCurrentDevice();
        RetrieveQueueCheckpoints(device->GetQueue(), "Graphics Queue");
        RetrieveQueueCheckpoints(device->GetComputeQueue(), "Compute Queue");
        __debugbreak();
    }
}
