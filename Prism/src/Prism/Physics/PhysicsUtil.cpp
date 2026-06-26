#include "prpch.h"
#include "PhysicsUtil.h"

#include <filesystem>

#include "Prism/Scene/Entity.h"

namespace Prism {

    physx::PxTransform ToPhysXTransform(const Transform& transform)
    {
        physx::PxQuat r = ToPhysXQuat(glm::normalize(glm::quat(glm::radians(transform.GetRotation()))));
        physx::PxVec3 p = ToPhysXVector(transform.GetPosition());
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

    static std::filesystem::path GetSerializedDir(const std::string& filepath)
    {
        std::filesystem::path p = filepath;
        std::string stem = p.stem().string();
        return p.parent_path() / stem;
    }

    void ConvexMeshSerializer::DeleteIfSerializedAndInvalidated(const std::string& filepath)
    {
        auto dir = GetSerializedDir(filepath);
        if (std::filesystem::is_directory(dir))
            std::filesystem::remove_all(dir);
    }

    void ConvexMeshSerializer::SerializeMesh(const std::string& filepath, const physx::PxDefaultMemoryOutputStream& data, const std::string& submeshName)
    {
        auto dir = GetSerializedDir(filepath);
        std::filesystem::create_directories(dir);

        std::filesystem::path path;
        if (!submeshName.empty())
            path = dir / (submeshName + ".pxm");
        else
            path = dir / "default.pxm";

        std::string cachedFilepath = path.string();

        FILE* f = fopen(cachedFilepath.c_str(), "wb");
        if (f)
        {
            fwrite(data.getData(), sizeof(physx::PxU8), data.getSize() / sizeof(physx::PxU8), f);
            fclose(f);
        }
    }

    bool ConvexMeshSerializer::IsSerialized(const std::string& filepath)
    {
        auto dir = GetSerializedDir(filepath);
        return std::filesystem::is_directory(dir);
    }

    static physx::PxU8* s_MeshDataBuffer;

    physx::PxDefaultMemoryInputData ConvexMeshSerializer::DeserializeMesh(const std::string& filepath, const std::string& submeshName)
    {
        std::filesystem::path p = filepath;
        std::string stem = p.stem().string();
        auto dir = p.parent_path() / stem;
        auto path = dir / (submeshName + ".pxm");

        uint32_t size = 0;

        FILE* f = fopen(path.string().c_str(), "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            size = (uint32_t)ftell(f);
            fseek(f, 0, SEEK_SET);

            if (s_MeshDataBuffer)
                delete[] s_MeshDataBuffer;

            s_MeshDataBuffer = new physx::PxU8[size / sizeof(physx::PxU8)];
            fread(s_MeshDataBuffer, sizeof(physx::PxU8), size / sizeof(physx::PxU8), f);
            fclose(f);
        }
        PR_CORE_ASSERT(size > 0, "Failed to deserialize mesh: {0}", path.string());

        return physx::PxDefaultMemoryInputData(s_MeshDataBuffer, size);
    }

}
