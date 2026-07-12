#include "prpch.h"
#include "Mesh.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Pipeline.h"
#include "Prism/Renderer/Buffer/VertexBuffer.h"
#include "Prism/Renderer/Buffer/IndexBuffer.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include "imgui.h"


#include <filesystem>
#pragma warning(disable: 4267)

namespace Prism {

#define MESH_DEBUG_LOG 0
#if MESH_DEBUG_LOG
#define PR_MESH_LOG(...) PR_CORE_TRACE(__VA_ARGS__)
#else
#define PR_MESH_LOG(...)
#endif

    glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
        result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
        result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
        result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
        return result;
    }

    static const uint32_t s_MeshImportFlags =
            aiProcess_CalcTangentSpace |        // 创建切线和副切线
            aiProcess_Triangulate |             // 将所有面转换为三角形
            aiProcess_SortByPType |             // 将所有点、线和面分开
            aiProcess_GenNormals |              // 如果模型没有法线则创建法线
            aiProcess_GenUVCoords |             // 如果模型没有纹理坐标则创建纹理坐标
            aiProcess_OptimizeMeshes |          // 优化网格
            aiProcess_ValidateDataStructure;    // 验证数据结构

    struct LogStream : public Assimp::LogStream
    {
        static void Initialize()
        {
            if (Assimp::DefaultLogger::isNullLogger())
            {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
            }
        }

        void write(const char* message) override
        {
            PR_CORE_WARN("Assimp: {0}", message);
        }
    };



    Mesh::Mesh(MeshData&& data)
        : m_StaticVertices(std::move(data.Vertices))
        , m_AnimatedVertices(std::move(data.AnimVertices))
        , m_Indices(std::move(data.Indices))
        , m_Submeshes(std::move(data.Submeshes))
        , m_IsAnimated(data.IsAnimated)
        , m_Importer(std::move(data.Importer))
        , m_BoneCount(data.BoneCount)
        , m_BoneInfo(std::move(data.BoneInfo))
        , m_BoneMapping(std::move(data.BoneMapping))
    {
        FilePath = std::move(data.FilePath);
        m_Scene = m_Importer ? m_Importer->GetScene() : nullptr;
        if (m_Scene)
            m_InverseTransform = glm::inverse(Mat4FromAssimpMat4(m_Scene->mRootNode->mTransformation));

        if (!m_IsAnimated && !m_StaticVertices.empty())
        {
            m_TriangleCache.reserve(m_Submeshes.size());
            for (size_t m = 0; m < m_Submeshes.size(); m++)
            {
                auto& submesh = m_Submeshes[m];
                m_TriangleCache[m] = {};
                for (uint32_t i = submesh.BaseIndex; i < submesh.BaseIndex + submesh.IndexCount; i += 3)
                {
                    auto& idx = m_Indices[i / 3];
                    m_TriangleCache[m].emplace_back(
                        m_StaticVertices[idx.V1 + submesh.BaseVertex],
                        m_StaticVertices[idx.V2 + submesh.BaseVertex],
                        m_StaticVertices[idx.V3 + submesh.BaseVertex]);
                }
            }
        }

        VertexBufferLayout vertexLayout;
        if (m_IsAnimated)
        {
            m_VertexBuffer = VertexBuffer::Create(m_AnimatedVertices.data(), m_AnimatedVertices.size() * sizeof(AnimatedVertex));
            vertexLayout = {
                { ShaderDataType::Float3, "a_Position",  VertexSemantic::Position},
                { ShaderDataType::Float3, "a_Normal",    VertexSemantic::Normal},
                { ShaderDataType::Float3, "a_Tangent",   VertexSemantic::Tangent},
                { ShaderDataType::Float3, "a_Binormal",  VertexSemantic::Binormal},
                { ShaderDataType::Float2, "a_TexCoord",  VertexSemantic::TexCoord0},
                { ShaderDataType::Int4,   "a_BoneIDs",    VertexSemantic::BoneIndices},
                { ShaderDataType::Float4, "a_BoneWeights", VertexSemantic::BoneWeights},
            };
        }
        else
        {
            m_VertexBuffer = VertexBuffer::Create(m_StaticVertices.data(), m_StaticVertices.size() * sizeof(Vertex));
            vertexLayout = {
                { ShaderDataType::Float3, "a_Position",  VertexSemantic::Position},
                { ShaderDataType::Float3, "a_Normal",    VertexSemantic::Normal},
                { ShaderDataType::Float3, "a_Tangent",   VertexSemantic::Tangent},
                { ShaderDataType::Float3, "a_Binormal",  VertexSemantic::Binormal},
                { ShaderDataType::Float2, "a_TexCoord",  VertexSemantic::TexCoord0},
            };
        }
        m_VertexBuffer->SetLayout(vertexLayout);
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));

        PipelineSpecification pipelineSpec;
        pipelineSpec.Layout = vertexLayout;
        m_Pipeline = Pipeline::Create(pipelineSpec);
    }


    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform)
        : m_StaticVertices(vertices), m_Indices(indices), m_IsAnimated(false)
    {
        Submesh submesh;
        submesh.BaseVertex = 0;
        submesh.BaseIndex = 0;
        submesh.IndexCount = indices.size() * 3;
        submesh.Transform = transform;
        m_Submeshes.push_back(submesh);

        m_VertexBuffer = VertexBuffer::Create(m_StaticVertices.data(), m_StaticVertices.size() * sizeof(Vertex));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));

        VertexBufferLayout vertexLayout = {
            { ShaderDataType::Float3, "a_Position", VertexSemantic::Position },
            { ShaderDataType::Float3, "a_Normal", VertexSemantic::Normal },
            { ShaderDataType::Float3, "a_Tangent", VertexSemantic::Tangent },
            { ShaderDataType::Float3, "a_Binormal", VertexSemantic::Binormal },
            { ShaderDataType::Float2, "a_TexCoord", VertexSemantic::TexCoord0 },
        };
        m_VertexBuffer->SetLayout(vertexLayout);

        PipelineSpecification pipelineSpec;
        pipelineSpec.Layout = vertexLayout;
        m_Pipeline = Pipeline::Create(pipelineSpec);
    }

    Mesh::~Mesh()
    {
    }
#pragma region 骨骼动画相关
    static std::string LevelToSpaces(uint32_t level)
    {
        std::string result = "";
        for (uint32_t i = 0; i < level; i++)
            result += "--";
        return result;
    }

    void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, uint32_t level)
    {
        glm::mat4 transform = parentTransform * Mat4FromAssimpMat4(node->mTransformation);
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            uint32_t mesh = node->mMeshes[i];
            auto& submesh = m_Submeshes[mesh];
            submesh.NodeName = node->mName.C_Str();
            submesh.Transform = transform;
        }
        // PR_MESH_LOG("{0} {1}", LevelToSpaces(level), node->mName.C_Str());

        for (uint32_t i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level + 1);
    }
    uint32_t Mesh::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
        {
            if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
                return i;
        }

        return 0;
    }
    uint32_t Mesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        PR_CORE_ASSERT(pNodeAnim->mNumRotationKeys > 0);

        for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
        {
            if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
                return i;
        }

        return 0;
    }


    uint32_t Mesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
    {
        PR_CORE_ASSERT(pNodeAnim->mNumScalingKeys > 0);

        for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
        {
            if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
                return i;
        }

        return 0;
    }


    glm::vec3 Mesh::InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim)
    {
        if (nodeAnim->mNumPositionKeys == 1)
        {
            // No interpolation necessary for single value
            auto v = nodeAnim->mPositionKeys[0].mValue;
            return { v.x, v.y, v.z };
        }

        uint32_t PositionIndex = FindPosition(animationTime, nodeAnim);
        uint32_t NextPositionIndex = (PositionIndex + 1);
        PR_CORE_ASSERT(NextPositionIndex < nodeAnim->mNumPositionKeys);
        float DeltaTime = (float)(nodeAnim->mPositionKeys[NextPositionIndex].mTime - nodeAnim->mPositionKeys[PositionIndex].mTime);
        float Factor = glm::clamp((animationTime - (float)nodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime, 0.0f, 1.0f);
        const aiVector3D& Start = nodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = nodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        auto aiVec = Start + Factor * Delta;
        return { aiVec.x, aiVec.y, aiVec.z };
    }


    glm::quat Mesh::InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim)
    {
        if (nodeAnim->mNumRotationKeys == 1)
        {
            // No interpolation necessary for single value
            auto v = nodeAnim->mRotationKeys[0].mValue;
            return glm::quat(v.w, v.x, v.y, v.z);
        }

        uint32_t RotationIndex = FindRotation(animationTime, nodeAnim);
        uint32_t NextRotationIndex = (RotationIndex + 1);
        PR_CORE_ASSERT(NextRotationIndex < nodeAnim->mNumRotationKeys);
        float DeltaTime = (float)(nodeAnim->mRotationKeys[NextRotationIndex].mTime - nodeAnim->mRotationKeys[RotationIndex].mTime);
        float Factor = glm::clamp((animationTime - (float)nodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime, 0.0f, 1.0f);
        const aiQuaternion& StartRotationQ = nodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ = nodeAnim->mRotationKeys[NextRotationIndex].mValue;
        auto q = aiQuaternion();
        aiQuaternion::Interpolate(q, StartRotationQ, EndRotationQ, Factor);
        q = q.Normalize();
        return glm::quat(q.w, q.x, q.y, q.z);
    }


    glm::vec3 Mesh::InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim)
    {
        if (nodeAnim->mNumScalingKeys == 1)
        {
            // No interpolation necessary for single value
            auto v = nodeAnim->mScalingKeys[0].mValue;
            return { v.x, v.y, v.z };
        }

        uint32_t index = FindScaling(animationTime, nodeAnim);
        uint32_t nextIndex = (index + 1);
        PR_CORE_ASSERT(nextIndex < nodeAnim->mNumScalingKeys);
        float deltaTime = (float)(nodeAnim->mScalingKeys[nextIndex].mTime - nodeAnim->mScalingKeys[index].mTime);
        float factor = glm::clamp((animationTime - (float)nodeAnim->mScalingKeys[index].mTime) / deltaTime, 0.0f, 1.0f);
        const auto& start = nodeAnim->mScalingKeys[index].mValue;
        const auto& end = nodeAnim->mScalingKeys[nextIndex].mValue;
        auto delta = end - start;
        auto aiVec = start + factor * delta;
        return { aiVec.x, aiVec.y, aiVec.z };
    }

    void Mesh::ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& parentTransform)
    {
        std::string name(pNode->mName.data);
        const aiAnimation* animation = m_Scene->mAnimations[0];
        glm::mat4 nodeTransform(Mat4FromAssimpMat4(pNode->mTransformation));
        const aiNodeAnim* nodeAnim = FindNodeAnim(animation, name);

        if (nodeAnim)
        {
            glm::vec3 translation = InterpolateTranslation(AnimationTime, nodeAnim);
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));

            glm::quat rotation = InterpolateRotation(AnimationTime, nodeAnim);
            glm::mat4 rotationMatrix = glm::toMat4(rotation);

            glm::vec3 scale = InterpolateScale(AnimationTime, nodeAnim);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));

            nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
        }

        glm::mat4 transform = parentTransform * nodeTransform;

        if (m_BoneMapping.find(name) != m_BoneMapping.end())
        {
            uint32_t BoneIndex = m_BoneMapping[name];
            m_BoneInfo[BoneIndex].FinalTransformation = m_InverseTransform * transform * m_BoneInfo[BoneIndex].BoneOffset;
        }

        for (uint32_t i = 0; i < pNode->mNumChildren; i++)
            ReadNodeHierarchy(AnimationTime, pNode->mChildren[i], transform);
    }

    const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
    {
        for (uint32_t i = 0; i < animation->mNumChannels; i++)
        {
            const aiNodeAnim* nodeAnim = animation->mChannels[i];
            if (std::string(nodeAnim->mNodeName.data) == nodeName)
                return nodeAnim;
        }
        return nullptr;
    }
    void Mesh::BoneTransform(float time)
    {
        ReadNodeHierarchy(time, m_Scene->mRootNode, glm::mat4(1.0f));
        m_BoneTransforms.resize(m_BoneCount);
        for (size_t i = 0; i < m_BoneCount; i++)
            m_BoneTransforms[i] = m_BoneInfo[i].FinalTransformation;
    }
#pragma endregion

    void Mesh::OnUpdate(float ts)
    {
        if (m_IsAnimated)
        {
            if (m_AnimationPlaying)
            {
                m_WorldTime += ts;

                float ticksPerSecond = (float)(m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0f) * m_TimeMultiplier;
                m_AnimationTime += ts * ticksPerSecond;
                m_AnimationTime = fmod(m_AnimationTime, (float)m_Scene->mAnimations[0]->mDuration);
            }

            // TODO: We only need to recalc bones if rendering has been requested at the current animation frame
            BoneTransform(m_AnimationTime);
        }
    }

    void Mesh::DumpVertexBuffer()
    {
        // TODO: Convert to ImGui
        PR_MESH_LOG("------------------------------------------------------");
        PR_MESH_LOG("Vertex Buffer Dump");
        PR_MESH_LOG("Mesh: {0}", FilePath);
        if (m_IsAnimated)
        {
            for (size_t i = 0; i < m_AnimatedVertices.size(); i++)
            {
                auto& vertex = m_AnimatedVertices[i];
                PR_MESH_LOG("Vertex: {0}", i);
                PR_MESH_LOG("Position: {0}, {1}, {2}", vertex.Position.x, vertex.Position.y, vertex.Position.z);
                PR_MESH_LOG("Normal: {0}, {1}, {2}", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
                PR_MESH_LOG("Binormal: {0}, {1}, {2}", vertex.Binormal.x, vertex.Binormal.y, vertex.Binormal.z);
                PR_MESH_LOG("Tangent: {0}, {1}, {2}", vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z);
                PR_MESH_LOG("TexCoord: {0}, {1}", vertex.Texcoord.x, vertex.Texcoord.y);
                PR_MESH_LOG("--");
            }
        }
        else
        {
            for (size_t i = 0; i < m_StaticVertices.size(); i++)
            {
                auto& vertex = m_StaticVertices[i];
                PR_MESH_LOG("Vertex: {0}", i);
                PR_MESH_LOG("Position: {0}, {1}, {2}", vertex.Position.x, vertex.Position.y, vertex.Position.z);
                PR_MESH_LOG("Normal: {0}, {1}, {2}", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
                PR_MESH_LOG("Binormal: {0}, {1}, {2}", vertex.Binormal.x, vertex.Binormal.y, vertex.Binormal.z);
                PR_MESH_LOG("Tangent: {0}, {1}, {2}", vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z);
                PR_MESH_LOG("TexCoord: {0}, {1}", vertex.Texcoord.x, vertex.Texcoord.y);
                PR_MESH_LOG("--");
            }
        }
        PR_MESH_LOG("------------------------------------------------------");
    }

}
