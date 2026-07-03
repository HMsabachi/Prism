#include "EditorLayer.h"
#include "Prism/ImGui/ImGui.h"
#include "Prism/Asset/ModelImporter.h"
#include "Prism/Asset/AssetManager.h"
using Prism::UI::Property;
using Prism::UI::PropertyFlag;

#include "Prism/ImGui/ImGuizmo.h"
#include "Prism/Core/LanguageManager.h"
#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Prism/Scene/Systems/Physics2DSystem.h"
#include "Prism/Scene/Systems/RenderSystem.h"
#include "Prism/Core/Warning.h"
PR_WARNING_DISABLE(4312)

#include "Prism/Physics/Physics.h"
#include "Prism/Editor/PhysicsSettingsWindow.h"
#include "Prism/Editor/AssetEditorPanel.h"
#include "Prism/Math/Math.h"
#include "Prism/Utilities/FileSystemWatcher.h"

#include <filesystem>

namespace Prism
{
    static void ImGuiShowHelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }


        EditorLayer::EditorLayer()
            : m_SceneType(SceneType::Model), m_EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f))
        {

        }

        EditorLayer::~EditorLayer()
        {

        }

        void EditorLayer::OnAttach()
        {

            using namespace glm;


            // Editor
            m_CheckerboardTex = Texture2D::Create("assets/editor/Checkerboard.tga");
            m_PlayButtonTex = Texture2D::Create("assets/editor/PlayButton.png");

            // 语言设置
            LanguageManager::Get().LoadLanguage("Assets/Lang/en-US.yml");

            m_EditorScene = Ref<Scene>::Create("EditorScene", true);
            m_ActiveScene = m_EditorScene;
            UpdateWindowTitle("Untitled Scene");
            CSharpScriptEngine::SetSceneContext(m_EditorScene);
            PythonScriptEngine::SetSceneContext(m_EditorScene);
            m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>(m_EditorScene);
            m_SceneHierarchyPanel->SetSelectionChangedCallback(std::bind(&EditorLayer::SelectEntity, this, std::placeholders::_1));
            m_SceneHierarchyPanel->SetEntityDeletedCallback(std::bind(&EditorLayer::OnEntityDeleted, this, std::placeholders::_1));

            m_AssetManagerPanel = CreateScope<AssetManagerPanel>();
            m_ObjectsPanel = CreateScope<ObjectsPanel>();

            SceneSerializer serializer(m_EditorScene);
            //serializer.Deserialize("Assets/Scenes/Physics3DTest.psc");
            //m_SceneFilePath = "Assets/Scenes/Physics3DTest.psc";
            //serializer.Deserialize("Assets/Scenes/FPSDemo.psc");
            //m_SceneFilePath = "Assets/Scenes/FPSDemo.psc";

            FileSystemWatcher::StartWatching();
            AssetEditorPanel::RegisterDefaultEditors();
        }

        void EditorLayer::OnDetach()
        {
            FileSystemWatcher::StopWatching();
            if (m_SceneState == SceneState::Play)
                OnSceneStop();
             m_SceneHierarchyPanel = nullptr;
             m_EditorScene = nullptr;
        }

        void EditorLayer::OnScenePlay()
        {
            m_SelectionContext.clear();

            m_SceneState = SceneState::Play;

            //if (m_ReloadScriptOnPlay)
                //CSharpScriptEngine::ReloadAppAssembly("assets/scripts/ExampleApp.dll");

            m_RuntimeScene = Ref<Scene>::Create();
            m_EditorScene->CopyTo(m_RuntimeScene);

            m_RuntimeScene->OnRuntimeStart();
            m_ActiveScene = m_RuntimeScene;
            m_SceneHierarchyPanel->SetContext(m_RuntimeScene);
            UpdateWindowTitle("Runtime Scene");
        }

        void EditorLayer::OnSceneStop()
        {
            m_RuntimeScene->OnRuntimeStop();
            m_SceneState = SceneState::Edit;

            // Unload runtime scene
            m_RuntimeScene = nullptr;
            m_ActiveScene = m_EditorScene;

            m_SelectionContext.clear();
            CSharpScriptEngine::SetSceneContext(m_EditorScene);
            PythonScriptEngine::SetSceneContext(m_EditorScene);
            m_SceneHierarchyPanel->SetContext(m_EditorScene);

            Input::SetCursorMode(CursorMode::Normal);

            UpdateWindowTitle("Untitled Scene");
        }

        void EditorLayer::UpdateWindowTitle(const std::string& sceneName)
        {
            std::string title = sceneName + " - Prism Engine - " + Application::GetPlatformName() + " (" + Application::GetConfigurationName() + ")";
            Application::Get().GetWindow().SetTitle(title);
        }


        void EditorLayer::OnUpdate()
        {
            float ts = Time::GetDeltaTime();

            switch (m_SceneState)
            {
            case SceneState::Edit:
            {
                if (m_ViewportPanelFocused)
                    m_EditorCamera.OnUpdate(ts);

                auto* rs = m_ActiveScene->GetSystem<RenderSystem>();
                if (rs)
                    rs->SetEditorCamera(m_EditorCamera);
                m_ActiveScene->OnRender(ts);

                if (m_DrawOnTopBoundingBoxes)
                {
                    Renderer::BeginRenderPass(rs->GetFinalRenderPass(), false);
                    auto viewProj = m_EditorCamera.GetViewProjection();
                    Renderer2D::BeginScene(viewProj, false);
                    Renderer2D::EndScene();
                    Renderer::EndRenderPass();
                }

                if (m_SelectionContext.size() && false)
                {
                    auto& selection = m_SelectionContext[0];

                    if (selection.Mesh && selection.Entity.HasComponent<MeshRendererComponent>())
                    {
                        Renderer::BeginRenderPass(rs->GetFinalRenderPass(), false);
                        auto viewProj = m_EditorCamera.GetViewProjection();
                        Renderer2D::BeginScene(viewProj, false);
                        glm::vec4 color = (m_SelectionMode == SelectionMode::Entity) ? glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f } : glm::vec4{ 0.2f, 0.9f, 0.2f, 1.0f };
                        glm::mat4 worldTransform = m_ActiveScene->GetTransformRelativeToParent(selection.Entity);
                        Renderer::DrawAABB(selection.Mesh->BoundingBox, worldTransform * selection.Mesh->Transform, color);
                        Renderer2D::EndScene();
                        Renderer::EndRenderPass();
                    }
                }
                break;
            }
            case SceneState::Play:
            {
                if (m_ViewportPanelFocused)
                    m_EditorCamera.OnUpdate(ts);

                m_ActiveScene->OnUpdate();
                m_ActiveScene->OnRender(ts);

                // Box2D collider debug drawing
                {
                    Renderer::BeginRenderPass(m_ActiveScene->GetSystem<RenderSystem>()->GetFinalRenderPass(), false);
                    auto viewProj = m_EditorCamera.GetViewProjection();
                    Renderer2D::BeginScene(viewProj, false);
                    {
                        auto boxColliderView = m_ActiveScene->GetAllEntitiesWith<BoxCollider2DComponent>();
                        for (auto e : boxColliderView)
                        {
                            Entity entity = { e, m_ActiveScene.Raw() };
                            if (!entity.HasComponent<TransformComponent>())
                                continue;

                            auto& tc = entity.Transformation();
                            auto& boxCollider = entity.GetComponent<BoxCollider2DComponent>();

                            glm::vec3 position = tc.GetPosition() + glm::vec3(boxCollider.Offset, 0.0f);
                            float angle = tc.GetRotation().z;
                            glm::vec2 size = boxCollider.Size * 2.0f;

                            Renderer2D::DrawRotatedRect(position, size, angle, glm::vec4(0.5f, 0.0f, 0.5f, 0.3f));
                        }

                        auto circleColliderView = m_ActiveScene->GetAllEntitiesWith<CircleCollider2DComponent>();
                        for (auto e : circleColliderView)
                        {
                            Entity entity = { e, m_ActiveScene.Raw() };
                            if (!entity.HasComponent<TransformComponent>())
                                continue;

                            auto& tc = entity.Transformation();
                            auto& circleCollider = entity.GetComponent<CircleCollider2DComponent>();

                            glm::vec3 position = tc.GetPosition() + glm::vec3(circleCollider.Offset, 0.0f);

                            Renderer2D::DrawCircle(position, circleCollider.Radius, glm::vec4(0.5f, 0.0f, 0.5f, 0.3f));
                        }
                    }
                    Renderer2D::EndScene();
                    Renderer::EndRenderPass();
                }
                break;
            }
            case SceneState::Pause:
            {
                if (m_ViewportPanelFocused)
                    m_EditorCamera.OnUpdate(ts);

                m_ActiveScene->OnRender(ts);
                break;
            }
            }
        }

        void EditorLayer::ShowBoundingBoxes(bool show, bool onTop /*= false*/)
        {
            if (auto* rs = m_ActiveScene->GetSystem<RenderSystem>())
                rs->GetOptions().ShowBoundingBoxes = show && !onTop;
            m_DrawOnTopBoundingBoxes = show && onTop;
        }

        void EditorLayer::SelectEntity(Entity entity)
        {
            SelectedSubmesh selection;
            if (entity.HasComponent<MeshRendererComponent>())
            {
                auto& meshComp = entity.GetComponent<MeshRendererComponent>();

                if (meshComp.Mesh)
                {
                    selection.Mesh = &meshComp.Mesh->GetSubmeshes()[0];
                }
            }
            selection.Entity = entity;
            m_SelectionContext.clear();
            m_SelectionContext.push_back(selection);

            m_EditorScene->SetSelectedEntity(entity);
        }

        float EditorLayer::GetSnapValue()
        {
            switch (m_GizmoType)
            {
                case ImGuizmo::OPERATION::TRANSLATE: return 0.5f;
                case ImGuizmo::OPERATION::ROTATE: return 45.0f;
                case ImGuizmo::OPERATION::SCALE: return 0.5f;
            }
            return 0.0f;
        }

        void EditorLayer::DrawMaterialProperty(const PrismShaderCompiler::AST::ShaderUniform& uni, Material& material)
        {
            const auto& name = uni.Name;
            const auto& displayName = uni.DisplayName.empty() ? uni.Name : uni.DisplayName;

            switch (uni.Type)
            {
            case PrismShaderCompiler::PropertyType::Float:
            {
                float value = material.GetFloat(name);
                if (Property(displayName, value))
                    material.SetFloat(name, value);
                break;
            }
            case PrismShaderCompiler::PropertyType::Range:
            {
                float value = material.GetFloat(name);
                if (Property(displayName, value, uni.RangeMin, uni.RangeMax, PropertyFlag::SliderProperty))
                    material.SetFloat(name, value);
                break;
            }
            case PrismShaderCompiler::PropertyType::Color:
            {
                glm::vec4 color = material.GetColor(name);
                if (Property(displayName, color, PropertyFlag::ColorProperty))
                    material.SetColor(name, color);
                break;
            }
            case PrismShaderCompiler::PropertyType::Color3:
            {
                glm::vec3 color = material.GetColor3(name);
                if (Property(displayName, color, PropertyFlag::ColorProperty))
                    material.SetColor3(name, color);
                break;
            }
            case PrismShaderCompiler::PropertyType::Vector2:
            {
                glm::vec2 vec2 = material.GetVec2(name);
                if (Property(displayName, vec2))
                    material.SetVec2(name, vec2);
                break;
            }
            case PrismShaderCompiler::PropertyType::Vector3:
            {
                glm::vec3 vec3 = material.GetVec3(name);
                if (Property(displayName, vec3))
                    material.SetVec3(name, vec3);
                break;
            }
            case PrismShaderCompiler::PropertyType::Vector4:
            {
                glm::vec4 vec4 = material.GetVec4(name);
                if (Property(displayName, vec4))
                    material.SetVec4(name, vec4);
                break;
            }
            case PrismShaderCompiler::PropertyType::Int:
            {
                int value = material.GetInt(name);
                if (Property(displayName, value))
                    material.SetInt(name, value);
                break;
            }
            case PrismShaderCompiler::PropertyType::Bool:
            {
                bool value = material.GetBool(name);
                if (Property(displayName, value))
                    material.SetBool(name, value);
                break;
            }
            case PrismShaderCompiler::PropertyType::Texture2D:
            {
                auto texture2D = material.GetTexture2D(name);
                if (Property(displayName, texture2D, m_CheckerboardTex->GetRendererID()))
                {
                    std::string filename = Application::Get().OpenFile("");
                    if (!filename.empty())
                        material.SetTexture(name, Texture2D::Create(filename));
                }
                break;
            }
            case PrismShaderCompiler::PropertyType::TextureCube:
            {
                auto textureCube = material.GetTextureCube(name);
                Property(displayName, textureCube, m_CheckerboardTex->GetRendererID());
                break;
            }
            default:
                break;
            }
        }

        void EditorLayer::NewScene()
        {
            m_EditorScene = Ref<Scene>::Create("Empty Scene", true);
            m_ActiveScene = m_EditorScene;
            m_SceneHierarchyPanel->SetContext(m_EditorScene);
            CSharpScriptEngine::SetSceneContext(m_EditorScene);
            PythonScriptEngine::SetSceneContext(m_EditorScene);
            UpdateWindowTitle("Untitled Scene");
            m_SceneFilePath = std::string();

            m_EditorScene->SetSelectedEntity({});
            m_SelectionContext.clear();

            m_EditorCamera = EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f));
        }

        void EditorLayer::OpenScene()
        {
            auto& app = Application::Get();
            std::string filepath = app.OpenFile("Prism Scene (*.psc)\0*.psc\0");
            if (!filepath.empty())
                OpenScene(filepath);
        }

        void EditorLayer::OpenScene(const std::string& filepath)
        {
            Ref<Scene> newScene = Ref<Scene>::Create();
            SceneSerializer serializer(newScene);
            serializer.Deserialize(filepath);
            m_EditorScene = newScene;
            m_ActiveScene = m_EditorScene;
            std::filesystem::path path = filepath;
            UpdateWindowTitle(path.filename().string());
            m_SceneHierarchyPanel->SetContext(m_EditorScene);
            CSharpScriptEngine::SetSceneContext(m_EditorScene);
            PythonScriptEngine::SetSceneContext(m_EditorScene);

            m_EditorScene->SetSelectedEntity({});
            m_SelectionContext.clear();

            m_SceneFilePath = filepath;
        }

        void EditorLayer::SaveScene()
        {
            if (m_SceneFilePath.empty())
            {
                SaveSceneAs();
                return;
            }
            SceneSerializer serializer(m_EditorScene);
            serializer.Serialize(m_SceneFilePath);
        }

        void EditorLayer::SaveSceneAs()
        {
            auto& app = Application::Get();
            std::string filepath = app.SaveFile("Prism Scene (*.psc)\0*.psc\0");
            if (!filepath.empty())
            {
                SceneSerializer serializer(m_EditorScene);
                serializer.Serialize(filepath);

                std::filesystem::path path = filepath;
                UpdateWindowTitle(path.filename().string());
                m_SceneFilePath = filepath;
            }
        }

        void EditorLayer::OnImGuiRender()
        {
        #define ENABLE_DOCKSPACE 1
        #if ENABLE_DOCKSPACE
            static bool p_open = true;

            static bool opt_fullscreen = true;
            static bool opt_padding = false;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if (opt_fullscreen)
            {
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }
            else
            {
                dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
            }
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", &p_open, window_flags);
            ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            ImGuiStyle& style = ImGui::GetStyle();
            float minWinSizeX = style.WindowMinSize.x;
            style.WindowMinSize.x = 370.0f;
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }
            style.WindowMinSize.x = minWinSizeX;
            // Editor Panel ------------------------------------------------------------------------------


            ImGui::Begin("Model");

            ImGui::Begin("Environment");
            ImGui::Columns(2);
            ImGui::AlignTextToFramePadding();
            {
                auto* scene = m_ActiveScene.Raw();
                if (auto* p2d = scene ? scene->GetSystem<Physics2DSystem>() : nullptr)
                {
                    float physics2DGravity = p2d->GetGravity();
                    if (Property(TR("Gravity"), physics2DGravity, -10000.0f, 10000.0f, PropertyFlag::DragProperty))
                        p2d->SetGravity(physics2DGravity);
                }
            }
            Property(TR("Env Map Rotation"), m_EnvMapRotation, -360.0f, 360.0f, PropertyFlag::SliderProperty);
            if (Property(TR("Show Bounding Boxes"), m_UIShowBoundingBoxes))
                ShowBoundingBoxes(m_UIShowBoundingBoxes, m_UIShowBoundingBoxesOnTop);
            if (m_UIShowBoundingBoxes && Property(TR("On Top"), m_UIShowBoundingBoxesOnTop))
                ShowBoundingBoxes(m_UIShowBoundingBoxes, m_UIShowBoundingBoxesOnTop);

            char* label = m_SelectionMode == SelectionMode::Entity ? const_cast<char*>(TR("Entity")) : const_cast<char*>(TR("Mesh"));
            if (ImGui::Button(label))
            {
                m_SelectionMode = m_SelectionMode == SelectionMode::Entity ? SelectionMode::SubMesh : SelectionMode::Entity;
            }

            ImGui::Columns(1);

            ImGui::End();

            ImGui::Separator();
            {
                ImGui::Text(TR("Mesh"));
                //auto meshComponent = m_MeshEntity.GetComponent<MeshRendererComponent>();
                //std::string fullpath = meshComponent.Mesh ? meshComponent.Mesh->GetFilePath() : "None";
                //size_t found = fullpath.find_last_of("/\\");
                //std::string path = found != std::string::npos ? fullpath.substr(found + 1) : fullpath;
                //ImGui::Text(path.c_str()); ImGui::SameLine();
                //if (ImGui::Button("...##Mesh"))
                //{
                //	std::string filename = Prism::Application::Get().OpenFile("");
                //	if (filename != "")
                //	{
                //		auto newMesh = Ref<Mesh>::Create(filename);
                //		// m_MeshMaterial.reset(new MaterialInstance(newMesh->GetMaterial()));
                //		// m_MeshEntity->SetMaterial(m_MeshMaterial);
                //		meshComponent.Mesh = newMesh;
                //	}
                //}
            }
            ImGui::Separator();



            if (ImGui::TreeNode(TR("Shaders")))
            {
                auto& shaders = Prism::PrismShader::s_AllShaders;
                for (auto& shader : shaders)
                {
                    if (ImGui::TreeNode(shader->GetName().c_str()))
                    {
                        std::string buttonName = "Reload##" + shader->GetName();
                        if (ImGui::Button(buttonName.c_str()))
                            shader->Reload();
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
            ImGui::End();

            // ImGui::ShowDemoWindow();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.8f, 0.8f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            ImGui::Begin("Toolbar");
            if (m_SceneState == SceneState::Edit)
            {
                if (ImGui::ImageButton((ImTextureID)(m_PlayButtonTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(0.9f, 0.9f, 0.9f, 1.0f)))
                {
                    OnScenePlay();
                }
            }
            else if (m_SceneState == SceneState::Play)
            {
                if (ImGui::ImageButton((ImTextureID)(m_PlayButtonTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(1.0f, 1.0f, 1.0f, 0.2f)))
                {
                    OnSceneStop();
                }
            }
            ImGui::SameLine();
            if (ImGui::ImageButton((ImTextureID)(m_PlayButtonTex->GetRendererID()), ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.6f)))
            {
                PR_CORE_INFO("PLAY!");
            }
            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();
            ImGui::PopStyleVar();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("Viewport");

            m_ViewportPanelMouseOver = ImGui::IsWindowHovered();
            m_ViewportPanelFocused = ImGui::IsWindowFocused();
            auto viewportOffset = ImGui::GetCursorPos();
            auto viewportSize = ImGui::GetContentRegionAvail();
            //viewportSize.x *= 2;
            //viewportSize.y *= 2;
            if (auto* rs = m_ActiveScene->GetSystem<RenderSystem>())
                rs->SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
            m_EditorCamera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 10000.0f));
            m_EditorCamera.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
            //viewportSize.x *= 0.5;
            //viewportSize.y *= 0.5;

            if (auto* rs = m_ActiveScene->GetSystem<RenderSystem>())
                ImGui::Image((void*)rs->GetFinalColorBufferID(), viewportSize, { 0, 1 }, { 1, 0 });

            if (ImGui::BeginDragDropTarget())
            {
                auto assetData = ImGui::AcceptDragDropPayload("asset_payload");
                if (assetData)
                {
                    int count = assetData->DataSize / sizeof(AssetHandle);

                    for (int i = 0; i < count; i++)
                    {
                        AssetHandle assetHandle = *(((AssetHandle*)assetData->Data) + i);
                        Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);

                        if (count == 1 && asset->Type == AssetType::Scene)
                        {
                            OpenScene(asset->FilePath);
                        }

                        if (asset->Type == AssetType::Mesh)
                        {
                            Entity entity = m_EditorScene->CreateEntity(asset->FileName);
                            auto importResult = ModelImporter::Import(asset->FilePath);
                            entity.AddComponent<MeshRendererComponent>(importResult.Mesh);
                            entity.GetComponent<MeshRendererComponent>().SetMaterials(importResult.Materials);
                            SelectEntity(entity);
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            static int counter = 0;
            auto windowSize = ImGui::GetWindowSize();
            ImVec2 minBound = ImGui::GetWindowPos();
            minBound.x += viewportOffset.x;
            minBound.y += viewportOffset.y;
            ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
            m_ViewportBounds[0] = { minBound.x, minBound.y };
            m_ViewportBounds[1] = { maxBound.x, maxBound.y };
            m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect(minBound, maxBound);
            // Gizmos
            if (m_GizmoType != -1 && m_SelectionContext.size())
            {
                auto& selection = m_SelectionContext[0];
                float rw = (float)ImGui::GetWindowWidth();
                float rh = (float)ImGui::GetWindowHeight();
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);

                bool snap = Input::IsKeyPressed(PR_KEY_LEFT_CONTROL);
                auto& tc = selection.Entity.Transformation();
                glm::mat4 worldMatrix = m_ActiveScene->GetTransformRelativeToParent(selection.Entity);
                float snapValue = GetSnapValue();
                float snapValues[3] = { snapValue, snapValue, snapValue };
                if (m_SelectionMode == SelectionMode::Entity)
                {
                    ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()),
                        glm::value_ptr(m_EditorCamera.GetProjectionMatrix()),
                        (ImGuizmo::OPERATION)m_GizmoType,
                        ImGuizmo::LOCAL,
                        glm::value_ptr(worldMatrix),
                        nullptr,
                        snap ? snapValues : nullptr);

                    // 世界 → 局部：移除父级变换
                    if (selection.Entity.GetParentUUID() != 0)
                    {
                        Entity parent = m_ActiveScene->FindEntityByUUID(selection.Entity.GetParentUUID());
                        if (parent)
                        {
                            glm::mat4 parentWorld = m_ActiveScene->GetTransformRelativeToParent(parent);
                            worldMatrix = glm::inverse(parentWorld) * worldMatrix;
                        }
                    }

                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(worldMatrix, translation, rotation, scale);
                    tc.SetPosition(translation);
                    tc.SetRotation(glm::degrees(rotation));
                    tc.SetScale(scale);
                }
                else
                {
                    glm::mat4 transformBase = worldMatrix * selection.Mesh->Transform;
                    ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()),
                        glm::value_ptr(m_EditorCamera.GetProjectionMatrix()),
                        (ImGuizmo::OPERATION)m_GizmoType,
                        ImGuizmo::LOCAL,
                        glm::value_ptr(transformBase),
                        nullptr,
                        snap ? snapValues : nullptr);

                    selection.Mesh->Transform = glm::inverse(worldMatrix) * transformBase;
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();


            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu(TR("File")))
                {
                    if (ImGui::MenuItem(TR("New Scene"), "Ctrl+N"))
                        NewScene();
                    if (ImGui::MenuItem(TR("Open Scene..."), "Ctrl+O"))
                    {
                        OpenScene();
                    }
                    if (ImGui::MenuItem(TR("Save Scene"), "Ctrl+S"))
                    {
                        SaveScene();
                    }
                    if (ImGui::MenuItem(TR("Save Scene As..."), "Ctrl+Shift+S"))
                    {
                        SaveSceneAs();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem(TR("Exit")))
                        p_open = false;
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(TR("Script")))
                {
                    if (ImGui::MenuItem(TR("Reload Assembly")))
                    {
#ifdef PR_DEBUG
                        CSharpScriptEngine::ReloadAppAssembly("assets/scripts/net9.0/ExampleApp.dll");
#else
                        CSharpScriptEngine::BuildAssembly();
                        CSharpScriptEngine::ReloadAppAssembly("assets/scripts/net9.0/Game.dll");
#endif
                        PythonScriptEngine::ReloadPythonScripts();
                    }
                    ImGui::MenuItem(TR("Reload assembly on play"), nullptr, &m_ReloadScriptOnPlay);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(TR("Edit")))
                {
                    ImGui::MenuItem(TR("Physics Settings"), nullptr, &m_ShowPhysicsSettings);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Language"))
                {
                    if (ImGui::MenuItem("English"))
                        LanguageManager::Get().LoadLanguage("Assets/Lang/en-US.yml");
                    if (ImGui::MenuItem("中文"))
                        LanguageManager::Get().LoadLanguage("Assets/Lang/zh-CN.yml");
                    if (ImGui::MenuItem("日本語"))
                        LanguageManager::Get().LoadLanguage("Assets/Lang/ja-JP.yml");

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            m_SceneHierarchyPanel->OnImGuiRender();

            ImGui::Begin("Materials");

            if (m_SelectionContext.size())
            {
                Entity selectedEntity = m_SelectionContext.front().Entity;
                if (selectedEntity.HasComponent<MeshRendererComponent>())
                {
                    auto& meshComp = selectedEntity.GetComponent<MeshRendererComponent>();
                    Ref<Mesh> mesh = meshComp.Mesh;
                    if (mesh)
                    {
                        auto& materials = meshComp.Materials;
                        static uint32_t selectedMaterialIndex = 0;
                        for (uint32_t i = 0; i < materials.size(); i++)
                        {
                            auto& material = materials[i];
                            if (!material) continue;

                            ImGuiTreeNodeFlags node_flags = (selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
                            bool opened = ImGui::TreeNodeEx((void*)(&material), node_flags, material->GetName().c_str());
                            if (ImGui::IsItemClicked())
                            {
                                selectedMaterialIndex = i;
                            }
                            if (opened)
                                ImGui::TreePop();
                        }

                        ImGui::Separator();

                        // Selected material
                        if (selectedMaterialIndex < materials.size())
                        {
                            auto& material = materials[selectedMaterialIndex];
                            {
                                auto currentShader = material->GetShader();
                                std::string shaderName = currentShader->GetName();
                                if (ImGui::BeginCombo("Shader", shaderName.c_str()))
                                {
                                    auto& allShaders = AssetManager::GetShaderLibrary()->GetAll();
                                    for (auto& [name, shader] : allShaders)
                                    {
                                        bool isSelected = (shader == currentShader);
                                        if (ImGui::Selectable(name.c_str(), isSelected))
                                        {
                                            if (shader != currentShader)
                                                material->SetShader(shader->Handle);
                                        }
                                        if (isSelected)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }
                            }

                            ImGui::Columns(2);
                            auto& uniforms = material->GetShader()->GetUniforms();
                            for (const auto& uni : uniforms)
                            {
                                DrawMaterialProperty(uni, *material);
                            }
                            ImGui::Columns(1);

                            // Keyword toggles
                            auto shader = material->GetShader();
                            const auto& keywords = shader->GetKeywords();
                            if (!keywords.empty())
                            {
                                ImGui::Separator();
                                ImGui::Text(TR("Shader Keywords"));
                                for (const auto& kw : keywords)
                                {
                                    bool enabled = material->IsKeywordEnabled(kw.Name);
                                    if (ImGui::Checkbox(kw.Name.c_str(), &enabled))
                                        material->SetKeyword(kw.Name, enabled);
                                }
                            }
                        }
                    }
                }
            }

            ImGui::End();
            //CSharpScriptEngine::OnImGuiRender();
            //PythonScriptEngine::OnImGuiRender();
            PhysicsSettingsWindow::OnImGuiRender(m_ShowPhysicsSettings);
            m_ActiveScene->OnImGuiRender();
            m_AssetManagerPanel->OnImGuiRender();
            m_ObjectsPanel->OnImGuiRender();
            AssetEditorPanel::OnImGuiRender();
            ImGui::End();
        #endif

            //static bool show_demo_window = true;
            //if (show_demo_window)
            //    ImGui::ShowDemoWindow(&show_demo_window);
            //ImGui::ShowStyleEditor();
        }

        void EditorLayer::OnEvent(Prism::Event& e)
        {
            if (m_SceneState == SceneState::Edit)
            {
                if (m_ViewportPanelMouseOver)
                    m_ActiveScene->OnEvent(e);
            }
            if (m_AllowViewportCameraEvents)
                            m_EditorCamera.OnEvent(e);
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<KeyPressedEvent>(PR_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
            dispatcher.Dispatch<MouseButtonPressedEvent>(PR_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
        }
        bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
        {
            if (m_ViewportPanelFocused)
            {
                switch (e.GetKeyCode())
                {
                case KeyCode::Q:
                    m_GizmoType = -1;
                    break;
                case KeyCode::W:
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                    break;
                case KeyCode::E:
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                    break;
                case KeyCode::R:
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                    break;
                case KeyCode::Delete:
                    if (m_SelectionContext.size())
                    {
                        Entity selectedEntity = m_SelectionContext[0].Entity;
                        m_EditorScene->DestroyEntity(selectedEntity);
                        m_SelectionContext.clear();
                        m_EditorScene->SetSelectedEntity({});
                        m_SceneHierarchyPanel->SetSelected({});
                    }
                    break;
                }
            }

            if (Input::IsKeyPressed(PR_KEY_LEFT_CONTROL))
            {
                switch (e.GetKeyCode())
                {
                case KeyCode::G:
                    // Toggle grid
                    if (auto* rs = m_ActiveScene->GetSystem<RenderSystem>())
                        rs->GetOptions().ShowGrid = !rs->GetOptions().ShowGrid;
                    break;
                case KeyCode::B:
                    // Toggle bounding boxes
                    m_UIShowBoundingBoxes = !m_UIShowBoundingBoxes;
                    ShowBoundingBoxes(m_UIShowBoundingBoxes, m_UIShowBoundingBoxesOnTop);
                    break;
                case KeyCode::D:
                    if (m_SelectionContext.size())
                    {
                        Entity selectedEntity = m_SelectionContext[0].Entity;
                        m_EditorScene->DuplicateEntity(selectedEntity);
                    }
                    break;
                case KeyCode::N:
                    NewScene();
                    break;
                case KeyCode::O:
                    OpenScene();
                    break;
                case KeyCode::S:
                    if (Input::IsKeyPressed(PR_KEY_LEFT_SHIFT))
                        SaveSceneAs();
                    else
                        SaveScene();
                    break;
                }
            }

            return false;
        }

        bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
        {
            auto [mx, my] = Input::GetMousePosition();
            if (e.GetMouseButton() == PR_MOUSE_BUTTON_LEFT && !Input::IsKeyPressed(PR_KEY_LEFT_ALT) &&
                !ImGuizmo::IsOver() && m_SceneState == SceneState::Edit)
            {
                auto [mouseX, mouseY] = GetMouseViewportSpace();
                if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
                {
                    auto [origin, direction] = CastRay(mouseX, mouseY);

                    m_SelectionContext.clear();
                    m_EditorScene->SetSelectedEntity({});
                    auto meshEntities = m_EditorScene->GetAllEntitiesWith<MeshRendererComponent>();
                    for (auto e : meshEntities)
                    {
                        Entity entity = { e, m_EditorScene.Raw() };
                        auto mesh = entity.GetComponent<MeshRendererComponent>().Mesh;
                        if (!mesh)
                            continue;

                        auto& submeshes = mesh->GetSubmeshes();
                        constexpr float lastT = std::numeric_limits<float>::max();
                        for (uint32_t i = 0; i < submeshes.size(); i++)
                        {
                            auto& submesh = submeshes[i];
                            glm::mat4 worldTransform = m_ActiveScene->GetTransformRelativeToParent(entity);
                            Ray ray = {
                                glm::inverse(worldTransform * submesh.Transform) * glm::vec4(origin, 1.0f),
                                glm::inverse(glm::mat3(worldTransform) * glm::mat3(submesh.Transform)) * direction
                            };

                            float t;
                            bool intersects = ray.IntersectsAABB(submesh.BoundingBox, t);
                            if (intersects)
                            {
                                const auto& triangleCache = mesh->GetTriangleCache(i);
                                for (const auto& triangle : triangleCache)
                                {
                                    if (ray.IntersectsTriangle(triangle.V0.Position, triangle.V1.Position, triangle.V2.Position, t))
                                    {
                                        PR_WARN("INTERSECTION: {0}, t={1}", submesh.NodeName, t);
                                        m_SelectionContext.push_back({ entity, &submesh, t });
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    std::sort(m_SelectionContext.begin(), m_SelectionContext.end(), [](auto& a, auto& b) { return a.Distance < b.Distance; });
                    if (m_SelectionContext.size())
                        OnSelected(m_SelectionContext[0]);

                }
            }
            return false;
        }
        std::pair<float, float> EditorLayer::GetMouseViewportSpace()
        {
            auto [mx, my] = ImGui::GetMousePos();
            mx -= m_ViewportBounds[0].x;
            my -= m_ViewportBounds[0].y;
            auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
            auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;

            return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
        }

        std::pair<glm::vec3, glm::vec3> EditorLayer::CastRay(float mx, float my)
        {
            glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

            auto inverseProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
            auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

            glm::vec4 ray = inverseProj * mouseClipPos;
            glm::vec3 rayPos = m_EditorCamera.GetPosition();
            glm::vec3 rayDir = inverseView * glm::vec3(ray);

            return { rayPos, rayDir };
        }

        void EditorLayer::OnSelected(const SelectedSubmesh& selectionContext)
        {
            m_SceneHierarchyPanel->SetSelected(selectionContext.Entity);
            m_EditorScene->SetSelectedEntity(selectionContext.Entity);
        }

        void EditorLayer::OnEntityDeleted(Entity e)
        {
            if (m_SelectionContext[0].Entity == e)
            {
                m_SelectionContext.clear();
                m_EditorScene->SetSelectedEntity({});
            }
        }

        Ray EditorLayer::CastMouseRay()
        {
            auto [mouseX, mouseY] = GetMouseViewportSpace();
            if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
            {
                auto [origin, direction] = CastRay(mouseX, mouseY);
                return Ray(origin, direction);
            }
            return Ray::Zero();
        }

    }
