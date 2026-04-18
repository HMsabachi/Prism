#pragma once

// For use by Prism applications 被用于使用Prism引擎的应用

#include <stdio.h>
#include "Prism/Core/Application.h"
#include "Prism/Core/Core.h"
#include "Prism/Core/Layer.h"
#include "Prism/Core/Log.h"
#include "Prism/Core/Time.h"
#include "Prism/Utilities/Utilities.h"

#include "Prism/Core/Input.h"
#include "Prism/Core/KeyCodes.h"
#include "Prism/Core/MouseButtonCodes.h"

#include "Prism/ImGui/ImGuiLayer.h"
// ---Renderer-
#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Shader/PrismShader.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/Renderer/Shader/GlobalUniforms.h"

#include "imgui/imgui.h"

// --- Prism Render API ------------------------------
#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/RenderPass.h"
#include "Prism/Renderer/VertexArray.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Material.h"

#include "Prism/Renderer/SceneRenderer.h"

#include "Prism/Renderer/Buffer/Framebuffer.h"
#include "Prism/Renderer/Buffer/Buffer.h"
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"

#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Camera/Camera.h"

// Scenes
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Scene.h"

// Editor
#include "Prism/Editor/SceneHierarchyPanel.h"

// ---------------------------------------------------

#include "Prism/Renderer/Camera/OrthographicCamera.h"
#include "Prism/Renderer/Camera/OrthographicCameraController.h"

//// ---Entry Point 入口点 ------------------------
//#include "Prism/Core/EntryPoint.h"


// ---------------------------------------------