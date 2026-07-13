#include "prpch.h"
#include "PhysicsUtil.h"

#include <filesystem>

#include "Prism/Scene/Entity.h"
#include "Prism/Utilities/FileSystem.h"

namespace Prism {

    physx::PxTransform ToPhysXTransform(const glm::vec3& position, const glm::quat& rotation)
    {
        physx::PxQuat r = ToPhysXQuat(glm::normalize(rotation));
        physx::PxVec3 p = ToPhysXVector(position);
        return physx::PxTransform(p, r);
    }

    physx::PxTransform ToPhysXTransform(const glm::mat4& matrix)
    {
        physx::PxQuat r = ToPhysXQuat(glm::normalize(glm::toQuat(matrix)));
        physx::PxVec3 p = ToPhysXVector(glm::vec3(matrix[3]));
        return physx::PxTransform(p, r);
    }

    physx::PxMat44 ToPhysXMatrix(const glm::mat4& matrix)
    {
        return *(physx::PxMat44*)&matrix;
    }

    physx::PxVec3 ToPhysXVector(const glm::vec3& vector)
    {
        return *(physx::PxVec3*)&vector;
    }

    physx::PxVec4 ToPhysXVector(const glm::vec4& vector)
    {
        return *(physx::PxVec4*)&vector;
    }

    physx::PxQuat ToPhysXQuat(const glm::quat& quat)
    {
        return *(physx::PxQuat*)&quat;
    }

    glm::mat4 FromPhysXTransform(const physx::PxTransform& transform)
    {
        glm::quat rotation = FromPhysXQuat(transform.q);
        glm::vec3 position = FromPhysXVector(transform.p);
        return glm::translate(glm::mat4(1.0F), position) * glm::toMat4(rotation);
    }

    glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix)
    {
        return *(glm::mat4*)&matrix;
    }

    glm::vec3 FromPhysXVector(const physx::PxVec3& vector)
    {
        return *(glm::vec3*)&vector;
    }

    glm::vec4 FromPhysXVector(const physx::PxVec4& vector)
    {
        return *(glm::vec4*)&vector;
    }

    glm::quat FromPhysXQuat(const physx::PxQuat& quat)
    {
        return *(glm::quat*)&quat;
    }

    physx::PxFilterFlags PrismFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1,
        physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
    {
        if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
        {
            pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
            return physx::PxFilterFlag::eDEFAULT;
        }

        pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

        if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
        {
            pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
            pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
            return physx::PxFilterFlag::eDEFAULT;
        }

        return physx::PxFilterFlag::eSUPPRESS;
    }

    static std::string GetCachePath(const std::string& filepath)
    {
        std::filesystem::path path = filepath;
        return "DataCache/Colliders/" + path.filename().string() + ".pxm";
    }

    void ConvexMeshSerializer::DeleteIfSerializedAndInvalidated(const std::string& filepath)
    {
        std::string cachePath = GetCachePath(filepath);
        if (FileSystem::Exists(cachePath))
            FileSystem::DeleteFile(cachePath);
    }

    void ConvexMeshSerializer::SerializeMesh(const std::string& filepath, const Buffer& data)
    {
        std::string cachePath = GetCachePath(filepath);

        FILE* f = fopen(cachePath.c_str(), "wb");
        if (f)
        {
            fwrite(data.Data, sizeof(byte), data.Size / sizeof(byte), f);
            fclose(f);
        }
    }

    bool ConvexMeshSerializer::IsSerialized(const std::string& filepath)
    {
        std::string cachePath = GetCachePath(filepath);
        return FileSystem::Exists(cachePath);
    }

    Buffer ConvexMeshSerializer::DeserializeMesh(const std::string& filepath)
    {
        std::string cachePath = GetCachePath(filepath);
        FILE* f = fopen(cachePath.c_str(), "rb");
        uint32_t size = 0;

        Buffer buffer;

        if (f)
        {
            fseek(f, 0, SEEK_END);
            size = (uint32_t)ftell(f);
            fseek(f, 0, SEEK_SET);

            buffer.Allocate(size);
            fread(buffer.Data, sizeof(byte), size / sizeof(byte), f);
            fclose(f);
        }

        return buffer;
    }

}
