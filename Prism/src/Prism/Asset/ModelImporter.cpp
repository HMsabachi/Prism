#include "prpch.h"
#include "ModelImporter.h"
#include "TexturePacker.h"

#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Material.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <filesystem>

namespace Prism
{
    static glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
        result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
        result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
        result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
        return result;
    }

    static const uint32_t s_MeshImportFlags =
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_GenNormals |
        aiProcess_GenUVCoords |
        aiProcess_OptimizeMeshes |
        aiProcess_ValidateDataStructure;

    static void TraverseNodes(aiNode* node, std::vector<Submesh>& submeshes,
                              const glm::mat4& parentTransform = glm::mat4(1.0f))
    {
        glm::mat4 transform = parentTransform * Mat4FromAssimpMat4(node->mTransformation);
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            uint32_t mesh = node->mMeshes[i];
            submeshes[mesh].NodeName = node->mName.C_Str();
            submeshes[mesh].Transform = transform;
        }
        for (uint32_t i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], submeshes, transform);
    }

    static std::unordered_map<std::string, ModelImportResult> s_Result;


    void ModelImporter::Shutdown()
    {
        s_Result.clear();
    }

    ModelImportResult ModelImporter::Import(const std::string& filepath)
    {
        ModelImportResult result;
        if (s_Result.find(filepath) != s_Result.end())
        {
            result.Mesh = s_Result[filepath].Mesh;
            for (auto& material : s_Result[filepath].Materials)
                result.Materials.push_back(Material::Create(material));
            return result;
        }

        auto importer = std::make_unique<Assimp::Importer>();
        const aiScene* scene = importer->ReadFile(filepath, s_MeshImportFlags);
        if (!scene || !scene->HasMeshes())
            return result;

        MeshData meshData;
        meshData.FilePath = filepath;
        meshData.IsAnimated = scene->mAnimations != nullptr;

        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        meshData.Submeshes.reserve(scene->mNumMeshes);
        for (size_t m = 0; m < scene->mNumMeshes; m++)
        {
            aiMesh* mesh = scene->mMeshes[m];
            Submesh& submesh = meshData.Submeshes.emplace_back();
            submesh.BaseVertex = vertexCount;
            submesh.BaseIndex = indexCount;
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.IndexCount = mesh->mNumFaces * 3;
            submesh.VertexCount = mesh->mNumVertices;
            submesh.MeshName = mesh->mName.C_Str();

            vertexCount += submesh.VertexCount;
            indexCount += submesh.IndexCount;

            if (meshData.IsAnimated)
            {
                for (size_t i = 0; i < mesh->mNumVertices; i++)
                {
                    AnimatedVertex vertex;
                    vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                    vertex.Normal   = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
                    if (mesh->HasTangentsAndBitangents())
                    {
                        vertex.Tangent  = { mesh->mTangents[i].x,  mesh->mTangents[i].y,  mesh->mTangents[i].z };
                        vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
                    }
                    if (mesh->HasTextureCoords(0))
                        vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                    meshData.AnimVertices.push_back(vertex);
                }
            }
            else
            {
                auto& aabb = submesh.BoundingBox;
                aabb.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
                aabb.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
                for (size_t i = 0; i < mesh->mNumVertices; i++)
                {
                    Vertex vertex;
                    vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                    vertex.Normal   = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
                    aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
                    aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
                    aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
                    aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
                    aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
                    aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);
                    if (mesh->HasTangentsAndBitangents())
                    {
                        vertex.Tangent  = { mesh->mTangents[i].x,  mesh->mTangents[i].y,  mesh->mTangents[i].z };
                        vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
                    }
                    if (mesh->HasTextureCoords(0))
                        vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                    meshData.Vertices.push_back(vertex);
                }
            }

            for (size_t i = 0; i < mesh->mNumFaces; i++)
            {
                Index idx = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
                meshData.Indices.push_back(idx);
            }
        }

        TraverseNodes(scene->mRootNode, meshData.Submeshes);

        meshData.Importer = std::move(importer);

        if (meshData.IsAnimated)
        {
            meshData.BoneCount = 0;
            for (size_t m = 0; m < scene->mNumMeshes; m++)
            {
                aiMesh* mesh = scene->mMeshes[m];
                Submesh& submesh = meshData.Submeshes[m];
                for (size_t i = 0; i < mesh->mNumBones; i++)
                {
                    aiBone* bone = mesh->mBones[i];
                    std::string boneName(bone->mName.data);
                    int boneIndex = 0;
                    if (meshData.BoneMapping.find(boneName) == meshData.BoneMapping.end())
                    {
                        boneIndex = meshData.BoneCount++;
                        BoneInfo bi;
                        bi.BoneOffset = Mat4FromAssimpMat4(bone->mOffsetMatrix);
                        meshData.BoneInfo.push_back(bi);
                        meshData.BoneMapping[boneName] = boneIndex;
                    }
                    else
                    {
                        boneIndex = meshData.BoneMapping[boneName];
                    }
                    for (size_t j = 0; j < bone->mNumWeights; j++)
                    {
                        int vertexID = submesh.BaseVertex + bone->mWeights[j].mVertexId;
                        float weight = bone->mWeights[j].mWeight;
                        meshData.AnimVertices[vertexID].AddBoneData(boneIndex, weight);
                    }
                }
            }
        }

        result.Mesh = Ref<Mesh>::Create(std::move(meshData));

        if (!scene->HasMaterials())
            return result;

        auto shader = AssetManager::GetShaderLibrary()->Get("Standard/PrismPBR");
        std::filesystem::path parentDir = std::filesystem::path(filepath).parent_path();

        std::vector<Ref<Material>> materialLookup(scene->mNumMaterials);
        for (unsigned i = 0; i < scene->mNumMaterials; i++)
        {
            auto* aiMat = scene->mMaterials[i];
            auto material = Material::Create(shader);

            if (meshData.IsAnimated)
                material->SetKeyword("SKINNED", true);

            aiString name;
            if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
                material->SetName(name.C_Str());
            else
                material->SetName("Mat " + std::to_string(i));

            float roughness = 0.0f;
            if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != AI_SUCCESS)
            {
                float glossiness = 1.0f;
                if (aiMat->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossiness) == AI_SUCCESS)
                    roughness = 1.0f - glossiness;
                else
                {
                    float shininess = 80.0f;
                    aiMat->Get(AI_MATKEY_SHININESS, shininess);
                    roughness = 1.0f - glm::sqrt(shininess / 100.0f);
                }
            }
            material->SetFloat("u_Roughness", roughness);

            float metalness = 0.0f;
            if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metalness) != AI_SUCCESS)
                aiMat->Get(AI_MATKEY_REFLECTIVITY, metalness);
            material->SetFloat("u_Metalness", metalness);

            aiColor3D aiColor;
            aiString aiTexPath;
            bool hasAlbedoMap = false;
            if (aiMat->GetTexture(aiTextureType_BASE_COLOR, 0, &aiTexPath) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS)
            {
                auto texPath = (parentDir / std::string(aiTexPath.data)).string();
                auto texture = Texture2D::Create(texPath, true);
                if (texture->Loaded())
                {
                    material->SetTexture("u_AlbedoTexture", texture);
                    material->SetKeyword("ALBEDO_MAP", true);
                    hasAlbedoMap = true;
                }
            }
            if (!hasAlbedoMap)
            {
                aiColor4D baseColor;
                if (aiMat->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
                    material->SetColor3("u_AlbedoColor", glm::vec3{baseColor.r, baseColor.g, baseColor.b});
                else
                {
                    aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
                    material->SetColor3("u_AlbedoColor", glm::vec3{aiColor.r, aiColor.g, aiColor.b});
                }
            }

            if (aiMat->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &aiTexPath) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS)
            {
                auto texPath = (parentDir / std::string(aiTexPath.data)).string();
                auto texture = Texture2D::Create(texPath);
                if (texture->Loaded())
                {
                    material->SetTexture("u_NormalTexture", texture);
                    material->SetKeyword("NORMAL_MAP", true);
                }
            }

            std::vector<OrmPackSource> ormSources;
            aiString mrPath;
            if (aiMat->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &mrPath) == AI_SUCCESS)
            {
                std::string p = (parentDir / std::string(mrPath.data)).string();
                ormSources.push_back({ p, 2, 2 });
                ormSources.push_back({ p, 1, 1 });
            }
            else
            {
                if (aiMat->GetTexture(aiTextureType_METALNESS, 0, &aiTexPath) == AI_SUCCESS)
                    ormSources.push_back({ (parentDir / std::string(aiTexPath.data)).string(), 0, 2 });
                if (aiMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &aiTexPath) == AI_SUCCESS ||
                    aiMat->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS)
                    ormSources.push_back({ (parentDir / std::string(aiTexPath.data)).string(), 0, 1 });
            }
            if (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aiTexPath) == AI_SUCCESS)
                ormSources.push_back({ (parentDir / std::string(aiTexPath.data)).string(), 0, 0 });

            if (!ormSources.empty())
            {
                std::string ormName = material->GetName();
                for (char& ch : ormName)
                    if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|')
                        ch = '_';
                std::filesystem::path cachePath = parentDir / (ormName + "_ORM.cache");
                if (!std::filesystem::exists(cachePath))
                    PackOrmTexture(ormSources, cachePath.string());

                auto ormTexture = Texture2D::Create(cachePath.string(), false);
                if (ormTexture->Loaded())
                {
                    material->SetTexture("u_OrmTexture", ormTexture);
                    for (const auto& s : ormSources)
                    {
                        if (s.DstChannel == 2) material->SetKeyword("METALNESS_MAP", true);
                        else if (s.DstChannel == 1) material->SetKeyword("ROUGHNESS_MAP", true);
                        else if (s.DstChannel == 0) material->SetKeyword("AO_MAP", true);
                    }
                }
            }

            aiColor3D emissiveColor;
            if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS)
                material->SetColor3("u_EmissiveColor", glm::vec3{emissiveColor.r, emissiveColor.g, emissiveColor.b});

            float emissiveIntensity = 1.0f;
            aiMat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveIntensity);
            material->SetFloat("u_EmissiveIntensity", emissiveIntensity);

            if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &aiTexPath) == AI_SUCCESS)
            {
                auto texPath = (parentDir / std::string(aiTexPath.data)).string();
                auto texture = Texture2D::Create(texPath, true);
                if (texture->Loaded())
                {
                    material->SetTexture("u_EmissiveTexture", texture);
                    material->SetKeyword("EMISSIVE_MAP", true);
                }
            }

            materialLookup[i] = material;
        }

        for (auto& submesh : result.Mesh->GetSubmeshes())
            result.Materials.push_back(
                submesh.MaterialIndex < materialLookup.size()
                    ? materialLookup[submesh.MaterialIndex] : nullptr);
        s_Result[filepath] = result;
        return result;
    }
}
