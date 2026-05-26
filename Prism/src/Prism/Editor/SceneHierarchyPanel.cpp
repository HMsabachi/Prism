#include "prpch.h"
#include "SceneHierarchyPanel.h"

#include <imgui.h>

#include "Prism/Core/Application.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Core/LanguageManager.h"
#include "Scripting/CSharp/CSharpScriptMetaRegistry.h"
#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TODO:
// - Eventually change imgui node IDs to be entity/asset GUID

namespace Prism {

    glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix);

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
        : m_Context(context)
    {
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
    {
        m_Context = scene;
        m_SelectionContext = {};
        if (m_SelectionContext && false)
        {
            // Try and find same entity in new scene
            auto& entityMap = m_Context->GetEntityMap();
            UUID selectedEntityID = m_SelectionContext.GetUUID();
            if (entityMap.find(selectedEntityID) != entityMap.end())
                m_SelectionContext = entityMap.at(selectedEntityID);
        }
    }

    void SceneHierarchyPanel::SetSelected(Entity entity)
    {
        m_SelectionContext = entity;
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");
        if (m_Context)
        {
            uint32_t entityCount = 0, meshCount = 0;
            for (auto& entityid : m_Context->m_Registry.view<entt::entity>())
            {
                Entity e(entityid, m_Context.Raw());
                if (e.HasComponent<IDComponent>())
                    DrawEntityNode(e);
            }

            if (ImGui::BeginPopupContextWindow("CreateEntityPopup", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
            {
                if (ImGui::MenuItem(TR("Create Empty Entity")))
                {
                    m_Context->CreateEntity("Empty Entity");
                }
                ImGui::EndPopup();
            }

            ImGui::End();

            ImGui::Begin("Properties");

            if (m_SelectionContext)
            {
                DrawComponents(m_SelectionContext);

                if (ImGui::Button(TR("Add Component")))
                    ImGui::OpenPopup("AddComponentPanel");

                if (ImGui::BeginPopup("AddComponentPanel"))
                {
                    if (!m_SelectionContext.HasComponent<CameraComponent>())
                    {
                        if (ImGui::Button(TR("Camera")))
                        {
                            m_SelectionContext.AddComponent<CameraComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<MeshComponent>())
                    {
                        if (ImGui::Button(TR("Mesh")))
                        {
                            m_SelectionContext.AddComponent<MeshComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
                    {
                        if (ImGui::Button(TR("Sprite Renderer")))
                        {
                            m_SelectionContext.AddComponent<SpriteRendererComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<RigidBody2DComponent>())
                    {
                        if (ImGui::Button(TR("Rigidbody 2D")))
                        {
                            m_SelectionContext.AddComponent<RigidBody2DComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<BoxCollider2DComponent>())
                    {
                        if (ImGui::Button(TR("Box Collider 2D")))
                        {
                            m_SelectionContext.AddComponent<BoxCollider2DComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<CircleCollider2DComponent>())
                    {
                        if (ImGui::Button(TR("Circle Collider 2D")))
                        {
                            m_SelectionContext.AddComponent<CircleCollider2DComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<RigidBodyComponent>())
                    {
                        if (ImGui::Button(TR("Rigidbody")))
                        {
                            m_SelectionContext.AddComponent<RigidBodyComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<PhysicsMaterialComponent>())
                    {
                        if (ImGui::Button(TR("Physics Material")))
                        {
                            m_SelectionContext.AddComponent<PhysicsMaterialComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<BoxColliderComponent>())
                    {
                        if (ImGui::Button(TR("Box Collider")))
                        {
                            m_SelectionContext.AddComponent<BoxColliderComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<SphereColliderComponent>())
                    {
                        if (ImGui::Button(TR("Sphere Collider")))
                        {
                            m_SelectionContext.AddComponent<SphereColliderComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<CapsuleColliderComponent>())
                    {
                        if (ImGui::Button(TR("Capsule Collider")))
                        {
                            m_SelectionContext.AddComponent<CapsuleColliderComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    if (!m_SelectionContext.HasComponent<MeshColliderComponent>())
                    {
                        if (ImGui::Button(TR("Mesh Collider")))
                        {
                            m_SelectionContext.AddComponent<MeshColliderComponent>();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    ImGui::EndPopup();
                }
            }
        }
        ImGui::End();

#if TODO
        ImGui::Begin("Mesh Debug");
        if (ImGui::CollapsingHeader(mesh->m_FilePath.c_str()))
        {
            if (mesh->m_IsAnimated)
            {
                if (ImGui::CollapsingHeader("Animation"))
                {
                    if (ImGui::Button(mesh->m_AnimationPlaying ? "Pause" : "Play"))
                        mesh->m_AnimationPlaying = !mesh->m_AnimationPlaying;

                    ImGui::SliderFloat("##AnimationTime", &mesh->m_AnimationTime, 0.0f, (float)mesh->m_Scene->mAnimations[0]->mDuration);
                    ImGui::DragFloat("Time Scale", &mesh->m_TimeMultiplier, 0.05f, 0.0f, 10.0f);
                }
            }
        }
        ImGui::End();
#endif
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        const char* name = "Unnamed Entity";
        if (entity.HasComponent<TagComponent>())
            name = entity.GetComponent<TagComponent>().Tag.c_str();

        ImGuiTreeNodeFlags node_flags = (entity == m_SelectionContext ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, node_flags, name);
        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
            if (m_SelectionChangedCallback)
                m_SelectionChangedCallback(m_SelectionContext);
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem(TR("Delete")))
                entityDeleted = true;

            ImGui::EndPopup();
        }
        if (opened)
        {
            if (entity.HasComponent<MeshComponent>())
            {
                auto mesh = entity.GetComponent<MeshComponent>().Mesh;
                // if (mesh)
                // 	DrawMeshNode(mesh);
            }

            ImGui::TreePop();
        }

        // Defer deletion until end of node UI
        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (entity == m_SelectionContext)
                m_SelectionContext = {};

            m_EntityDeletedCallback(entity);
        }
    }

    void SceneHierarchyPanel::DrawMeshNode(const Ref<Mesh>& mesh, uint32_t& imguiMeshID)
    {
        static char imguiName[128];
        memset(imguiName, 0, 128);
        sprintf(imguiName, "Mesh##%d", imguiMeshID++);

        // Mesh Hierarchy
        if (ImGui::TreeNode(imguiName))
        {
            auto rootNode = mesh->m_Scene->mRootNode;
            MeshNodeHierarchy(mesh, rootNode);
            ImGui::TreePop();
        }
    }

    static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
    {
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat orientation;
        glm::decompose(transform, scale, orientation, translation, skew, perspective);

        return { translation, orientation, scale };
    }

    void SceneHierarchyPanel::MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform, uint32_t level)
    {
        glm::mat4 localTransform = Mat4FromAssimpMat4(node->mTransformation);
        glm::mat4 transform = parentTransform * localTransform;

        if (ImGui::TreeNode(node->mName.C_Str()))
        {
            {
                auto [translation, rotation, scale] = GetTransformDecomposition(transform);
                ImGui::Text("World Transform");
                ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
                ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
            }
            {
                auto [translation, rotation, scale] = GetTransformDecomposition(localTransform);
                ImGui::Text("Local Transform");
                ImGui::Text("  Translation: %.2f, %.2f, %.2f", translation.x, translation.y, translation.z);
                ImGui::Text("  Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);
            }

            for (uint32_t i = 0; i < node->mNumChildren; i++)
                MeshNodeHierarchy(mesh, node->mChildren[i], transform, level + 1);

            ImGui::TreePop();
        }

    }

    static int s_UIContextID = 0;
    static uint32_t s_Counter = 0;
    static char s_IDBuffer[16];

    static void PushID()
    {
        ImGui::PushID(s_UIContextID++);
        s_Counter = 0;
    }

    static void PopID()
    {
        ImGui::PopID();
        s_UIContextID--;
    }

    static void BeginPropertyGrid()
    {
        PushID();
        ImGui::Columns(2);
    }

    static bool Property(const char* label, std::string& value, bool error = false)
    {
        bool modified = false;

        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        char buffer[256];
        strcpy(buffer, value.c_str());

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);

        if (error)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        if (ImGui::InputText(s_IDBuffer, buffer, 256))
        {
            value = buffer;
            modified = true;
        }
        if (error)
            ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    static void Property(const char* label, const char* value)
    {
        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);
        ImGui::InputText(s_IDBuffer, (char*)value, 256, ImGuiInputTextFlags_ReadOnly);

        ImGui::PopItemWidth();
        ImGui::NextColumn();
    }

    static bool Property(const char* label, bool& value)
    {
        bool modified = false;

        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);
        if (ImGui::Checkbox(s_IDBuffer, &value))
            modified = true;

        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    static bool Property(const char* label, int& value)
    {
        bool modified = false;

        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);
        if (ImGui::DragInt(s_IDBuffer, &value))
            modified = true;

        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    static bool Property(const char* label, float& value, float delta = 0.1f)
    {
        bool modified = false;

        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);
        if (ImGui::DragFloat(s_IDBuffer, &value, delta))
            modified = true;

        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    static bool Property(const char* label, glm::vec2& value, float delta = 0.1f)
    {
        bool modified = false;

        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);
        if (ImGui::DragFloat2(s_IDBuffer, glm::value_ptr(value), delta))
            modified = true;

        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    static bool Property(const char* label, glm::vec3& value, float delta = 0.1f)
    {
        bool modified = false;

        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);
        if (ImGui::DragFloat3(s_IDBuffer, glm::value_ptr(value), delta))
            modified = true;

        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    static bool Property(const char* label, glm::vec4& value, float delta = 0.1f)
    {
        bool modified = false;

        ImGui::Text(label);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);

        s_IDBuffer[0] = '#';
        s_IDBuffer[1] = '#';
        memset(s_IDBuffer + 2, 0, 14);
        itoa(s_Counter++, s_IDBuffer + 2, 16);
        if (ImGui::DragFloat4(s_IDBuffer, glm::value_ptr(value), delta))
            modified = true;

        ImGui::PopItemWidth();
        ImGui::NextColumn();

        return modified;
    }

    static void EndPropertyGrid()
    {
        ImGui::Columns(1);
        PopID();
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
            bool open = ImGui::TreeNodeEx((void*)((uint32_t)entity | (uint32_t)typeid(T).hash_code()), flags, "%s", name.c_str());
            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }
            ImGui::Separator();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        ImGui::AlignTextToFramePadding();

        auto id = entity.GetComponent<IDComponent>().ID;

        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;
            char buffer[256];
            memset(buffer, 0, 256);
            memcpy(buffer, tag.c_str(), tag.length());
            if (ImGui::InputText("##Tag", buffer, 256))
            {
                tag = std::string(buffer);
            }
        }

        // ID
        ImGui::SameLine();
        ImGui::TextDisabled("%llx", id);

        ImGui::Separator();

        if (entity.HasComponent<TransformComponent>())
        {
            auto& tc = entity.GetComponent<TransformComponent>();
            if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(TransformComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, TR("Transform")))
            {
                glm::vec3 rotation = glm::degrees(glm::eulerAngles(tc.Rotation));

                ImGui::Columns(2);
                ImGui::Text(TR("Translation"));
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);

                if (ImGui::DragFloat3("##translation", glm::value_ptr(tc.Position), 0.25f))
                {
                    entity.SetPosition(tc.Position);
                }

                ImGui::PopItemWidth();
                ImGui::NextColumn();

                ImGui::Text(TR("Rotation"));
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);

                if (ImGui::DragFloat3("##rotation", glm::value_ptr(rotation), 0.25f))
                {
                    entity.SetRotation(glm::radians(rotation));
                }

                ImGui::PopItemWidth();
                ImGui::NextColumn();

                ImGui::Text(TR("Scale"));
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);

                if (ImGui::DragFloat3("##scale", glm::value_ptr(tc.Scale), 0.25f))
                {
                    entity.SetScale(tc.Scale);
                }

                ImGui::PopItemWidth();
                ImGui::NextColumn();

                ImGui::Columns(1);

                ImGui::TreePop();
            }
            ImGui::Separator();
        }


        if (entity.HasComponent<MeshComponent>())
        {
            auto& mc = entity.GetComponent<MeshComponent>();
            if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(TransformComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, TR("Mesh")))
            {
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 100);
                ImGui::SetColumnWidth(1, 300);
                ImGui::SetColumnWidth(2, 40);
                ImGui::Text(TR("File Path"));
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);
                if (mc.Mesh)
                    ImGui::InputText("##meshfilepath", (char*)mc.Mesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
                ImGui::NextColumn();
                if (ImGui::Button("...##openmesh"))
                {
                    std::string file = Application::Get().OpenFile();
                    if (!file.empty())
                        mc.Mesh = Ref<Mesh>::Create(file);
                }
                ImGui::NextColumn();
                ImGui::Columns(1);
                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        if (entity.HasComponent<CameraComponent>())
        {
            auto& cc = entity.GetComponent<CameraComponent>();
            if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(CameraComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, TR("Camera")))
            {
                // Projection Type
                const char* projTypeStrings[] = { TR("Perspective"), TR("Orthographic") };
                const char* currentProj = projTypeStrings[(int)cc.Camera.GetProjectionType()];
                if (ImGui::BeginCombo(TR("Projection"), currentProj))
                {
                    for (int type = 0; type < 2; type++)
                    {
                        bool is_selected = (currentProj == projTypeStrings[type]);
                        if (ImGui::Selectable(projTypeStrings[type], is_selected))
                        {
                            currentProj = projTypeStrings[type];
                            cc.Camera.SetProjectionType((SceneCamera::ProjectionType)type);
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                BeginPropertyGrid();
                // Perspective parameters
                if (cc.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
                {
                    float verticalFOV = cc.Camera.GetPerspectiveVerticalFOV();
                    if (Property(TR("Vertical FOV"), verticalFOV))
                        cc.Camera.SetPerspectiveVerticalFOV(verticalFOV);

                    float nearClip = cc.Camera.GetPerspectiveNearClip();
                    if (Property(TR("Near Clip"), nearClip))
                        cc.Camera.SetPerspectiveNearClip(nearClip);
                    ImGui::SameLine();
                    float farClip = cc.Camera.GetPerspectiveFarClip();
                    if (Property(TR("Far Clip"), farClip))
                        cc.Camera.SetPerspectiveFarClip(farClip);
                }

                // Orthographic parameters
                else if (cc.Camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
                {
                    float orthoSize = cc.Camera.GetOrthographicSize();
                    if (Property(TR("Size"), orthoSize))
                        cc.Camera.SetOrthographicSize(orthoSize);

                    float nearClip = cc.Camera.GetOrthographicNearClip();
                    if (Property(TR("Near Clip"), nearClip))
                        cc.Camera.SetOrthographicNearClip(nearClip);
                    ImGui::SameLine();
                    float farClip = cc.Camera.GetOrthographicFarClip();
                    if (Property(TR("Far Clip"), farClip))
                        cc.Camera.SetOrthographicFarClip(farClip);
                }

                EndPropertyGrid();
                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        if (entity.HasComponent<SpriteRendererComponent>())
        {
            auto& src = entity.GetComponent<SpriteRendererComponent>();
            if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(SpriteRendererComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, TR("Sprite Renderer")))
            {

                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        DrawComponent<RigidBody2DComponent>(TR("Rigidbody 2D"), entity, [](auto& component)
            {
                const char* bodyTypeStrings[] = { TR("Static"), TR("Dynamic"), TR("Kinematic") };
                const char* currentType = bodyTypeStrings[(int)component.BodyType];
                if (ImGui::BeginCombo(TR("Type"), currentType))
                {
                    for (int i = 0; i < 3; i++)
                    {
                        bool is_selected = (currentType == bodyTypeStrings[i]);
                        if (ImGui::Selectable(bodyTypeStrings[i], is_selected))
                        {
                            component.BodyType = (RigidBody2DComponent::Type)i;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (component.BodyType == RigidBody2DComponent::Type::Dynamic)
                {
                    BeginPropertyGrid();
                    Property(TR("Fixed Rotation"), component.FixedRotation);
                    EndPropertyGrid();
                }
            });

        DrawComponent<BoxCollider2DComponent>(TR("Box Collider 2D"), entity, [](auto& component)
            {
                Property(TR("Offset"), component.Offset);
                Property(TR("Size"), component.Size);
                Property(TR("Density"), component.Density);
                Property(TR("Friction"), component.Friction);
            });

        DrawComponent<CircleCollider2DComponent>(TR("Circle Collider 2D"), entity, [](auto& component)
            {
                Property(TR("Offset"), component.Offset);
                Property(TR("Radius"), component.Radius);
                Property(TR("Density"), component.Density);
                Property(TR("Friction"), component.Friction);
            });

        DrawComponent<RigidBodyComponent>(TR("Rigidbody"), entity, [](auto& component)
            {
                const char* bodyTypeStrings[] = { TR("Static"), TR("Dynamic") };
                const char* currentType = bodyTypeStrings[(int)component.BodyType];
                if (ImGui::BeginCombo(TR("Type"), currentType))
                {
                    for (int i = 0; i < 2; i++)
                    {
                        bool is_selected = (currentType == bodyTypeStrings[i]);
                        if (ImGui::Selectable(bodyTypeStrings[i], is_selected))
                        {
                            component.BodyType = (RigidBodyComponent::Type)i;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (component.BodyType == RigidBodyComponent::Type::Dynamic)
                {
                    BeginPropertyGrid();
                    Property(TR("Mass"), component.Mass);
                    Property(TR("Is Kinematic"), component.IsKinematic);
                    EndPropertyGrid();

                    if (ImGui::TreeNode(TR("Constraints")))
                    {
                        BeginPropertyGrid();
                        Property(TR("Position X"), component.LockPositionX);
                        Property(TR("Position Y"), component.LockPositionY);
                        Property(TR("Position Z"), component.LockPositionZ);
                        Property(TR("Rotation X"), component.LockRotationX);
                        Property(TR("Rotation Y"), component.LockRotationY);
                        Property(TR("Rotation Z"), component.LockRotationZ);
                        EndPropertyGrid();

                        ImGui::TreePop();
                    }
                }
            });

        if (entity.HasComponent<PhysicsMaterialComponent>())
        {
            DrawComponent<PhysicsMaterialComponent>(TR("Physics Material"), entity, [](auto& component)
                {
                    BeginPropertyGrid();
                    Property(TR("Static Friction"), component.StaticFriction);
                    Property(TR("Dynamic Friction"), component.DynamicFriction);
                    Property(TR("Bounciness"), component.Bounciness);
                    EndPropertyGrid();
                });
        }

        DrawComponent<BoxColliderComponent>(TR("Box Collider"), entity, [](auto& component)
            {
                BeginPropertyGrid();
                Property(TR("Size"), component.Size);
                Property(TR("Offset"), component.Offset);
                EndPropertyGrid();
            });

        DrawComponent<SphereColliderComponent>(TR("Sphere Collider"), entity, [](auto& component)
            {
                BeginPropertyGrid();
                Property(TR("Radius"), component.Radius);
                EndPropertyGrid();
            });

        DrawComponent<CapsuleColliderComponent>(TR("Capsule Collider"), entity, [](auto& component)
            {
                BeginPropertyGrid();
                Property(TR("Radius"), component.Radius);
                Property(TR("Height"), component.Height);
                EndPropertyGrid();
            });

        DrawComponent<MeshColliderComponent>(TR("Mesh Collider"), entity, [](auto& component)
            {
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 100);
                ImGui::SetColumnWidth(1, 300);
                ImGui::SetColumnWidth(2, 40);
                ImGui::Text("File Path");
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);
                if (component.CollisionMesh)
                    ImGui::InputText("##meshcolliderfilepath", (char*)component.CollisionMesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##meshcolliderfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
                ImGui::NextColumn();
                if (ImGui::Button("...##openmeshcollider"))
                {
                    std::string file = Application::Get().OpenFile();
                    if (!file.empty())
                        component.CollisionMesh = Ref<Mesh>::Create(file);
                }
                ImGui::NextColumn();
                ImGui::Columns(1);
            });

        // CSharpScriptComponent
        if (entity.HasComponent<CSharpScriptComponent>())
        {
            if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(CSharpScriptComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, TR("C# Script")))
            {
                auto& comp = entity.GetComponent<CSharpScriptComponent>();

                if (ImGui::Button(TR("Add Behaviour")))
                    ImGui::OpenPopup("AddCSharpBehaviour");

                if (ImGui::BeginPopup("AddCSharpBehaviour"))
                {
                    auto classes = CSharpScriptMetaRegistry::GetAllBehaviourClasses();
                    if (classes.empty())
                    {
                        ImGui::TextDisabled("No behaviour classes found");
                    }
                    else
                    {
                        for (auto* meta : classes)
                        {
                            if (ImGui::MenuItem(meta->FullName.c_str()))
                            {
                                CSharpBehaviourBinding binding;
                                binding.BehaviourID = UUID();
                                binding.ClassName = meta->FullName;
                                binding.LifecycleMask = meta->LifecycleMask;
                                for (auto& [hash, fieldMeta] : meta->Fields)
                                {
                                    CSharpField field(fieldMeta.Name, fieldMeta.Type);
                                    if (fieldMeta.DefaultValue.Data && fieldMeta.DefaultValue.Size > 0)
                                        field.SetBuffer(fieldMeta.DefaultValue);
                                    binding.Fields[hash] = std::move(field);
                                }
                                comp.Behaviours.push_back(std::move(binding));
                            }
                        }
                    }
                    ImGui::EndPopup();
                }

                UUID pendingRemove = 0;
                int idx = 0;
                for (auto& binding : comp.Behaviours)
                {
                    ImGui::PushID(idx++);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
                    if (ImGui::SmallButton("X"))
                        pendingRemove = binding.BehaviourID;
                    ImGui::PopStyleColor();
                    ImGui::SameLine();

                    std::string shortName = binding.ClassName;
                    auto dotPos = shortName.rfind('.');
                    if (dotPos != std::string::npos)
                        shortName = shortName.substr(dotPos + 1);

                    if (ImGui::TreeNodeEx(shortName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        for (auto& [hash, field] : binding.Fields)
                        {
                            ImGui::PushID(hash);
                            switch (field.GetType())
                            {
                                case ScriptFieldType::Float:
                                {
                                    float val = field.GetValue<float>();
                                    if (ImGui::DragFloat(field.GetName().c_str(), &val, 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Double:
                                {
                                    double val = field.GetValue<double>();
                                    if (ImGui::InputDouble(field.GetName().c_str(), &val))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Bool:
                                {
                                    bool val = field.GetValue<bool>();
                                    if (ImGui::Checkbox(field.GetName().c_str(), &val))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Int32:
                                {
                                    int32_t val = field.GetValue<int32_t>();
                                    if (ImGui::DragInt(field.GetName().c_str(), &val))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Vector2:
                                {
                                    auto val = field.GetValue<glm::vec2>();
                                    if (ImGui::DragFloat2(field.GetName().c_str(), glm::value_ptr(val), 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Vector3:
                                {
                                    auto val = field.GetValue<glm::vec3>();
                                    if (ImGui::DragFloat3(field.GetName().c_str(), glm::value_ptr(val), 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Vector4:
                                {
                                    auto val = field.GetValue<glm::vec4>();
                                    if (ImGui::DragFloat4(field.GetName().c_str(), glm::value_ptr(val), 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                default:
                                    break;
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                if (pendingRemove)
                    CSharpScriptEngine::RemoveBehaviour(entity, pendingRemove);

                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        // PythonScriptComponent
        if (entity.HasComponent<PythonScriptComponent>())
        {
            if (ImGui::TreeNodeEx((void*)((uint32_t)entity | typeid(PythonScriptComponent).hash_code()), ImGuiTreeNodeFlags_DefaultOpen, TR("Python Script")))
            {
                auto& comp = entity.GetComponent<PythonScriptComponent>();

                if (ImGui::Button(TR("Add Behaviour")))
                    ImGui::OpenPopup("AddPythonBehaviour");

                if (ImGui::BeginPopup("AddPythonBehaviour"))
                {
                    auto classes = PythonScriptMetaRegistry::GetAllBehaviourClasses();
                    if (classes.empty())
                    {
                        ImGui::TextDisabled("No behaviour classes found");
                    }
                    else
                    {
                        for (auto* meta : classes)
                        {
                            if (ImGui::MenuItem(meta->FullName.c_str()))
                            {
                                PythonBehaviourBinding binding;
                                binding.BehaviourID = UUID();
                                binding.ClassName = meta->ClassName;
                                binding.ModuleName = meta->ModuleName;
                                binding.LifecycleMask = meta->LifecycleMask;
                                for (auto& [hash, fieldMeta] : meta->Fields)
                                {
                                    PythonField field(fieldMeta.Name, fieldMeta.Type);
                                    if (fieldMeta.DefaultValue.Data && fieldMeta.DefaultValue.Size > 0)
                                        field.SetBuffer(fieldMeta.DefaultValue);
                                    binding.Fields[hash] = std::move(field);
                                }
                                comp.Behaviours.push_back(std::move(binding));
                            }
                        }
                    }
                    ImGui::EndPopup();
                }

                UUID pendingRemove = 0;
                int idx = 0;
                for (auto& binding : comp.Behaviours)
                {
                    ImGui::PushID(idx++);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
                    if (ImGui::SmallButton("X"))
                        pendingRemove = binding.BehaviourID;
                    ImGui::PopStyleColor();
                    ImGui::SameLine();

                    std::string shortName = binding.ClassName;
                    auto dotPos = shortName.rfind('.');
                    if (dotPos != std::string::npos)
                        shortName = shortName.substr(dotPos + 1);

                    if (ImGui::TreeNodeEx(shortName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        for (auto& [hash, field] : binding.Fields)
                        {
                            ImGui::PushID(hash);
                            switch (field.GetType())
                            {
                                case ScriptFieldType::Float:
                                {
                                    float val = field.GetValue<float>();
                                    if (ImGui::DragFloat(field.GetName().c_str(), &val, 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Double:
                                {
                                    double val = field.GetValue<double>();
                                    if (ImGui::InputDouble(field.GetName().c_str(), &val))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Bool:
                                {
                                    bool val = field.GetValue<bool>();
                                    if (ImGui::Checkbox(field.GetName().c_str(), &val))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Int8:
                                case ScriptFieldType::Int16:
                                case ScriptFieldType::Int32:
                                case ScriptFieldType::Int64:
                                {
                                    int32_t val = (int32_t)field.GetValue<int64_t>();
                                    if (ImGui::DragInt(field.GetName().c_str(), &val))
                                        field.SetValue((int64_t)val);
                                    break;
                                }
                                case ScriptFieldType::UInt8:
                                case ScriptFieldType::UInt16:
                                case ScriptFieldType::UInt32:
                                case ScriptFieldType::UInt64:
                                {
                                    uint64_t val = field.GetValue<uint64_t>();
                                    int displayVal = (int)val;
                                    if (ImGui::DragInt(field.GetName().c_str(), &displayVal))
                                        field.SetValue((uint64_t)displayVal);
                                    break;
                                }
                                case ScriptFieldType::Vector2:
                                {
                                    auto val = field.GetValue<glm::vec2>();
                                    if (ImGui::DragFloat2(field.GetName().c_str(), glm::value_ptr(val), 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Vector3:
                                {
                                    auto val = field.GetValue<glm::vec3>();
                                    if (ImGui::DragFloat3(field.GetName().c_str(), glm::value_ptr(val), 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                case ScriptFieldType::Vector4:
                                {
                                    auto val = field.GetValue<glm::vec4>();
                                    if (ImGui::DragFloat4(field.GetName().c_str(), glm::value_ptr(val), 0.1f))
                                        field.SetValue(val);
                                    break;
                                }
                                default:
                                    break;
                            }
                            ImGui::PopID();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                if (pendingRemove)
                    PythonScriptEngine::RemoveBehaviour(entity, pendingRemove);

                ImGui::TreePop();
            }
            ImGui::Separator();
        }

    }

}
