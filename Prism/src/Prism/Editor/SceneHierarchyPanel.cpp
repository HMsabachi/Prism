#include "prpch.h"
#include "SceneHierarchyPanel.h"

#include "Prism/ImGui/ImGui.h"

#include "Prism/Core/Application.h"
#include "Prism/Core/Warning.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/MeshFactory.h"
#include "Prism/Asset/ModelImporter.h"
#include "Prism/Physics/PXPhysicsWrappers.h"
#include "Prism/Physics/PhysicsLayer.h"
#include "Prism/Utilities/FileSystem.h"
#include "Prism/Core/LanguageManager.h"
#include "Scripting/CSharp/CSharpScriptMetaRegistry.h"
#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Prism/Scene/Systems/ScriptSystem.h"
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
PR_WARNING_DISABLE(4312)

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
                if (ImGui::BeginMenu(TR("Create")))
                {
                    if (ImGui::MenuItem(TR("Empty Entity")))
                    {
                        auto newEntity = m_Context->CreateEntity("Empty Entity");
                        SetSelected(newEntity);
                    }
                    if (ImGui::MenuItem(TR("Mesh")))
                    {
                        auto newEntity = m_Context->CreateEntity("Mesh");
                        newEntity.AddComponent<MeshRendererComponent>();
                        SetSelected(newEntity);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem(TR("Directional Light")))
                    {
                        auto newEntity = m_Context->CreateEntity("Directional Light");
                        newEntity.AddComponent<DirectionalLightComponent>();
                        newEntity.Transformation().SetRotation(glm::degrees(glm::eulerAngles(glm::quat(glm::radians(glm::vec3{ 80.0f, 10.0f, 0.0f })))));
                        SetSelected(newEntity);
                    }
                    if (ImGui::MenuItem(TR("Sky Light")))
                    {
                        auto newEntity = m_Context->CreateEntity("Sky Light");
                        newEntity.AddComponent<SkyLightComponent>();
                        SetSelected(newEntity);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            ImGui::End();

            ImGui::Begin("Properties");

            if (m_SelectionContext)
            {
                DrawComponents(m_SelectionContext);
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
        node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
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
            // TODO: Children
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

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        if (entity.HasComponent<T>())
        {
            const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
            ImGui::PopStyleVar();
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
                ImGui::OpenPopup("ComponentSettings");

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem(TR("Remove component")))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveComponent<T>();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        ImGui::AlignTextToFramePadding();

        auto id = entity.GetComponent<IDComponent>().ID;

        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;
            char buffer[256];
            memset(buffer, 0, 256);
            memcpy(buffer, tag.c_str(), tag.length());
            ImGui::PushItemWidth(contentRegionAvailable.x * 0.5f);
            if (ImGui::InputText("##Tag", buffer, 256))
            {
                tag = std::string(buffer);
            }
            ImGui::PopItemWidth();
        }

        // ID
        ImGui::SameLine();
        ImGui::TextDisabled("%llx", id);
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 textSize = ImGui::CalcTextSize(TR("Add Component"));
        ImGui::SameLine(contentRegionAvailable.x - (textSize.x + GImGui->Style.FramePadding.y));
        if (ImGui::Button(TR("Add Component")))
            ImGui::OpenPopup("AddComponentPanel");

        if (ImGui::BeginPopup("AddComponentPanel"))
        {
            if (!entity.HasComponent<CameraComponent>())
            {
                if (ImGui::Button(TR("Camera")))
                {
                    entity.AddComponent<CameraComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<MeshRendererComponent>())
            {
                if (ImGui::Button(TR("Mesh")))
                {
                    entity.AddComponent<MeshRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<SpriteRendererComponent>())
            {
                if (ImGui::Button(TR("Sprite Renderer")))
                {
                    entity.AddComponent<SpriteRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<DirectionalLightComponent>())
            {
                if (ImGui::Button(TR("Directional Light")))
                {
                    entity.AddComponent<DirectionalLightComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<SkyLightComponent>())
            {
                if (ImGui::Button(TR("Sky Light")))
                {
                    entity.AddComponent<SkyLightComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<RigidBody2DComponent>())
            {
                if (ImGui::Button(TR("Rigidbody 2D")))
                {
                    entity.AddComponent<RigidBody2DComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<BoxCollider2DComponent>())
            {
                if (ImGui::Button(TR("Box Collider 2D")))
                {
                    entity.AddComponent<BoxCollider2DComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<CircleCollider2DComponent>())
            {
                if (ImGui::Button(TR("Circle Collider 2D")))
                {
                    entity.AddComponent<CircleCollider2DComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<RigidBodyComponent>())
            {
                if (ImGui::Button(TR("Rigidbody")))
                {
                    entity.AddComponent<RigidBodyComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<PhysicsMaterialComponent>())
            {
                if (ImGui::Button(TR("Physics Material")))
                {
                    entity.AddComponent<PhysicsMaterialComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<BoxColliderComponent>())
            {
                if (ImGui::Button(TR("Box Collider")))
                {
                    auto& component = entity.AddComponent<BoxColliderComponent>();
                    component.DebugMesh = MeshFactory::CreateBox(component.Size);
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<SphereColliderComponent>())
            {
                if (ImGui::Button(TR("Sphere Collider")))
                {
                    auto& component = entity.AddComponent<SphereColliderComponent>();
                    component.DebugMesh = MeshFactory::CreateSphere(component.Radius);
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<CapsuleColliderComponent>())
            {
                if (ImGui::Button(TR("Capsule Collider")))
                {
                    auto& component = entity.AddComponent<CapsuleColliderComponent>();
                    component.DebugMesh = MeshFactory::CreateCapsule(component.Radius, component.Height);
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!entity.HasComponent<MeshColliderComponent>())
            {
                if (ImGui::Button(TR("Mesh Collider")))
                {
                    entity.AddComponent<MeshColliderComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }

        if (entity.HasComponent<TransformComponent>())
        {
            const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
            Transform& transform = entity.Transformation();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), treeNodeFlags, "%s", TR("Transform"));
            ImGui::PopStyleVar();

            if (open)
            {
                glm::vec3 translation = transform.GetPosition();
                glm::vec3 rotation = transform.GetRotation();
                glm::vec3 scale = transform.GetScale();

                bool updateTransform = false;
                updateTransform |= UI::DrawVec3Control(TR("Translation"), translation);
                updateTransform |= UI::DrawVec3Control(TR("Rotation"), rotation);
                updateTransform |= UI::DrawVec3Control(TR("Scale"), scale, 1.0f);

                if (updateTransform)
                {
                    transform.SetPosition(translation);
                    transform.SetRotation(rotation);
                    transform.SetScale(scale);
                }

                ImGui::TreePop();
            }
        }


        DrawComponent<MeshRendererComponent>(TR("Mesh"), entity, [](auto& component)
            {
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 100);
                ImGui::SetColumnWidth(1, 300);
                ImGui::SetColumnWidth(2, 40);
                ImGui::Text(TR("File Path"));
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);
                if (component.Mesh)
                    ImGui::InputText("##meshfilepath", (char*)component.Mesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
                ImGui::NextColumn();
                if (ImGui::Button("...##openmesh"))
                {
                    std::string file = Application::Get().OpenFile();
                    if (!file.empty())
                    {
                        auto result = ModelImporter::Import(FileSystem::GetRelativePath(file));
                        component.Mesh = result.Mesh;
                        component.SetMaterials(result.Materials);
                    }
                }
                ImGui::NextColumn();
                ImGui::Columns(1);
            });

        DrawComponent<CameraComponent>(TR("Camera"), entity, [](auto& component)
            {
                const char* projTypeStrings[] = { TR("Perspective"), TR("Orthographic") };
                const char* currentProj = projTypeStrings[(int)component.Camera.GetProjectionType()];
                if (ImGui::BeginCombo(TR("Projection"), currentProj))
                {
                    for (int type = 0; type < 2; type++)
                    {
                        bool is_selected = (currentProj == projTypeStrings[type]);
                        if (ImGui::Selectable(projTypeStrings[type], is_selected))
                        {
                            currentProj = projTypeStrings[type];
                            component.Camera.SetProjectionType((SceneCamera::ProjectionType)type);
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                UI::BeginPropertyGrid();
                if (component.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
                {
                    float verticalFOV = component.Camera.GetPerspectiveVerticalFOV();
                    if (UI::Property(TR("Vertical FOV"), verticalFOV))
                        component.Camera.SetPerspectiveVerticalFOV(verticalFOV);

                    float nearClip = component.Camera.GetPerspectiveNearClip();
                    if (UI::Property(TR("Near Clip"), nearClip))
                        component.Camera.SetPerspectiveNearClip(nearClip);
                    ImGui::SameLine();
                    float farClip = component.Camera.GetPerspectiveFarClip();
                    if (UI::Property(TR("Far Clip"), farClip))
                        component.Camera.SetPerspectiveFarClip(farClip);
                }
                else if (component.Camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
                {
                    float orthoSize = component.Camera.GetOrthographicSize();
                    if (UI::Property(TR("Size"), orthoSize))
                        component.Camera.SetOrthographicSize(orthoSize);

                    float nearClip = component.Camera.GetOrthographicNearClip();
                    if (UI::Property(TR("Near Clip"), nearClip))
                        component.Camera.SetOrthographicNearClip(nearClip);
                    ImGui::SameLine();
                    float farClip = component.Camera.GetOrthographicFarClip();
                    if (UI::Property(TR("Far Clip"), farClip))
                        component.Camera.SetOrthographicFarClip(farClip);
                }
                UI::EndPropertyGrid();
            });

        DrawComponent<SpriteRendererComponent>(TR("Sprite Renderer"), entity, [](auto& component)
            {
            });

        DrawComponent<DirectionalLightComponent>(TR("Directional Light"), entity, [](auto& component)
            {
                UI::BeginPropertyGrid();
                UI::PropertyColor(TR("Radiance"), component.Radiance);
                UI::Property(TR("Intensity"), component.Intensity);
                UI::Property(TR("Cast Shadows"), component.CastShadows);
                UI::Property(TR("Soft Shadows"), component.SoftShadows);
                UI::Property(TR("Source Size"), component.LightSize);
                UI::EndPropertyGrid();
            });

        DrawComponent<SkyLightComponent>(TR("Sky Light"), entity, [](auto& component)
            {
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 100);
                ImGui::SetColumnWidth(1, 300);
                ImGui::SetColumnWidth(2, 40);
                ImGui::Text(TR("File Path"));
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);
                if (!component.SceneEnvironment.FilePath.empty())
                    ImGui::InputText("##envfilepath", (char*)component.SceneEnvironment.FilePath.c_str(), 256, ImGuiInputTextFlags_ReadOnly);
                else
                    ImGui::InputText("##envfilepath", (char*)"Empty", 256, ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
                ImGui::NextColumn();
                if (ImGui::Button("...##openenv"))
                {
                    std::string file = Application::Get().OpenFile("*.hdr");
                    if (!file.empty())
                        component.SceneEnvironment = Environment::Load(file);
                }
                ImGui::Columns(1);

                UI::BeginPropertyGrid();
                UI::Property(TR("Intensity"), component.Intensity, 0.01f, 0.0f, 5.0f);
                UI::EndPropertyGrid();
            });

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
                    UI::BeginPropertyGrid();
                    UI::Property(TR("Fixed Rotation"), component.FixedRotation);
                    UI::EndPropertyGrid();
                }
            });

        DrawComponent<BoxCollider2DComponent>(TR("Box Collider 2D"), entity, [](auto& component)
            {
                UI::Property(TR("Offset"), component.Offset);
                UI::Property(TR("Size"), component.Size);
                UI::Property(TR("Density"), component.Density);
                UI::Property(TR("Friction"), component.Friction);
            });

        DrawComponent<CircleCollider2DComponent>(TR("Circle Collider 2D"), entity, [](auto& component)
            {
                UI::Property(TR("Offset"), component.Offset);
                UI::Property(TR("Radius"), component.Radius);
                UI::Property(TR("Density"), component.Density);
                UI::Property(TR("Friction"), component.Friction);
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

                // Layer has been removed, set to Default layer
                if (!PhysicsLayerManager::IsLayerValid(component.Layer))
                    component.Layer = 0;

                uint32_t currentLayer = component.Layer;
                const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(currentLayer);
                ImGui::TextUnformatted(TR("Layer"));
                ImGui::SameLine();
                if (ImGui::BeginCombo("##LayerSelection", layerInfo.Name.c_str()))
                {
                    for (const auto& layer : PhysicsLayerManager::GetLayers())
                    {
                        bool is_selected = (currentLayer == layer.LayerID);
                        if (ImGui::Selectable(layer.Name.c_str(), is_selected))
                        {
                            currentLayer = layer.LayerID;
                            component.Layer = layer.LayerID;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (component.BodyType == RigidBodyComponent::Type::Dynamic)
                {
                    UI::BeginPropertyGrid();
                    UI::Property(TR("Mass"), component.Mass);
                    UI::Property(TR("Linear Drag"), component.LinearDrag);
                    UI::Property(TR("Angular Drag"), component.AngularDrag);
                    UI::Property(TR("Disable Gravity"), component.DisableGravity);
                    UI::Property(TR("Is Kinematic"), component.IsKinematic);
                    UI::EndPropertyGrid();

                    if (ImGui::TreeNode(TR("Constraints")))
                    {
                        UI::BeginPropertyGrid();

                        UI::BeginCheckboxGroup(TR("Freeze Position"));
                        UI::PropertyCheckboxGroup("X", component.LockPositionX);
                        UI::PropertyCheckboxGroup("Y", component.LockPositionY);
                        UI::PropertyCheckboxGroup("Z", component.LockPositionZ);
                        UI::EndCheckboxGroup();

                        UI::BeginCheckboxGroup(TR("Freeze Rotation"));
                        UI::PropertyCheckboxGroup("X", component.LockRotationX);
                        UI::PropertyCheckboxGroup("Y", component.LockRotationY);
                        UI::PropertyCheckboxGroup("Z", component.LockRotationZ);
                        UI::EndCheckboxGroup();

                        UI::EndPropertyGrid();

                        ImGui::TreePop();
                    }
                }
            });

        if (entity.HasComponent<PhysicsMaterialComponent>())
        {
            DrawComponent<PhysicsMaterialComponent>(TR("Physics Material"), entity, [](auto& component)
                {
                    UI::BeginPropertyGrid();
                    UI::Property(TR("Static Friction"), component.StaticFriction);
                    UI::Property(TR("Dynamic Friction"), component.DynamicFriction);
                    UI::Property(TR("Bounciness"), component.Bounciness);
                    UI::EndPropertyGrid();
                });
        }

        DrawComponent<BoxColliderComponent>(TR("Box Collider"), entity, [](auto& component)
            {
                UI::BeginPropertyGrid();
                if (UI::Property(TR("Size"), component.Size))
                {
                    component.DebugMesh = MeshFactory::CreateBox(component.Size);
                }
                UI::Property(TR("Offset"), component.Offset);
                UI::Property(TR("Is Trigger"), component.IsTrigger);
                UI::EndPropertyGrid();
            });

        DrawComponent<SphereColliderComponent>(TR("Sphere Collider"), entity, [](auto& component)
            {
                UI::BeginPropertyGrid();
                if (UI::Property(TR("Radius"), component.Radius))
                {
                    component.DebugMesh = MeshFactory::CreateSphere(component.Radius);
                }
                UI::Property(TR("Is Trigger"), component.IsTrigger);
                UI::EndPropertyGrid();
            });

        DrawComponent<CapsuleColliderComponent>(TR("Capsule Collider"), entity, [](auto& component)
            {
                UI::BeginPropertyGrid();
                bool changed = false;

                if (UI::Property(TR("Radius"), component.Radius))
                    changed = true;

                if (UI::Property(TR("Height"), component.Height))
                    changed = true;

                if (changed)
                {
                    component.DebugMesh = MeshFactory::CreateCapsule(component.Radius, component.Height);
                }
                UI::Property(TR("Is Trigger"), component.IsTrigger);
                UI::EndPropertyGrid();
            });

        DrawComponent<MeshColliderComponent>(TR("Mesh Collider"), entity, [](auto& component)
            {
                UI::BeginPropertyGrid();
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
                    {
                        component.CollisionMesh = ModelImporter::Import(FileSystem::GetRelativePath(file)).Mesh;
                        if (component.IsConvex)
                            PXPhysicsWrappers::CreateConvexMesh(component, true);
                        else
                            PXPhysicsWrappers::CreateTriangleMesh(component, true);
                    }
                }
                ImGui::NextColumn();
                ImGui::Columns(1);

                if (UI::Property(TR("Is Convex"), component.IsConvex))
                {
                    component.ProcessedMeshes.clear();
                    if (component.IsConvex)
                        PXPhysicsWrappers::CreateConvexMesh(component, true);
                    else
                        PXPhysicsWrappers::CreateTriangleMesh(component, true);
                }
                UI::Property(TR("Is Trigger"), component.IsTrigger);
                UI::EndPropertyGrid();
            });

        // CSharpScriptComponent
        if (entity.HasComponent<CSharpScriptComponent>())
        {
            const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(CSharpScriptComponent).hash_code(), treeNodeFlags, "%s", TR("C# Script"));
            ImGui::PopStyleVar();

            if (open)
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
                                auto* ss = m_Context->GetSystem<ScriptSystem>();
                                ss->AddCSharpBehaviour(m_SelectionContext, meta->ClassID);
                            }
                        }
                    }
                    ImGui::EndPopup();
                }

                UUID pendingRemove = 0;
                int idx = 0;
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    ImGui::PushID(idx++);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
                    if (ImGui::SmallButton("X"))
                        pendingRemove = binding.BehaviourID;
                    ImGui::PopStyleColor();
                    ImGui::SameLine();

                    std::string shortName;
                    if (auto* meta = CSharpScriptMetaRegistry::GetClassMetadata(binding.ClassID))
                        shortName = meta->ClassName;

                    if (ImGui::TreeNodeEx(shortName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        {
                            auto* ss = m_Context->GetSystem<ScriptSystem>();
                            bool enabled = ss->GetEnabled(binding.BehaviourID);
                            if (ImGui::Checkbox("Enabled", &enabled))
                                ss->SetEnabled(binding.BehaviourID, enabled);
                            ImGui::Separator();
                        }

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
                {
                    auto* ss = m_Context->GetSystem<ScriptSystem>();
                    ss->RemoveCSharpBehaviour(entity, pendingRemove);
                }

                ImGui::TreePop();
            }
        }

        // PythonScriptComponent
        if (entity.HasComponent<PythonScriptComponent>())
        {
            const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(PythonScriptComponent).hash_code(), treeNodeFlags, "%s", TR("Python Script"));
            ImGui::PopStyleVar();

            if (open)
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
                                auto* ss = m_Context->GetSystem<ScriptSystem>();
                                ss->AddPythonBehaviour(entity, meta->ClassID);
                            }
                        }
                    }
                    ImGui::EndPopup();
                }

                UUID pendingRemove = 0;
                int idx = 0;
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    ImGui::PushID(idx++);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
                    if (ImGui::SmallButton("X"))
                        pendingRemove = binding.BehaviourID;
                    ImGui::PopStyleColor();
                    ImGui::SameLine();

                    std::string shortName;
                    if (auto* meta = PythonScriptMetaRegistry::GetClassMetadata(binding.ClassID))
                        shortName = meta->ClassName;

                    if (ImGui::TreeNodeEx(shortName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        {
                            auto* ss = m_Context->GetSystem<ScriptSystem>();
                            bool enabled = ss->GetEnabled(binding.BehaviourID);
                            if (ImGui::Checkbox("Enabled", &enabled))
                                ss->SetEnabled(binding.BehaviourID, enabled);
                            ImGui::Separator();
                        }

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
                {
                    auto* ss = m_Context->GetSystem<ScriptSystem>();
                    ss->RemovePythonBehaviour(entity, pendingRemove);
                }

                ImGui::TreePop();
            }
        }

    }

}
