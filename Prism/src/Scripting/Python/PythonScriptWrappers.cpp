#include "prpch.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Prism/Renderer/Buffer/ShaderStorageBuffer.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/Core/Math/Noise.h"
#include "Prism/Core/Input.h"
#include "Prism/Physics/PXPhysicsWrappers.h"
#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsActor.h"
#include "Prism/Physics/PhysicsUtil.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Prism/Scene/Systems/ScriptSystem.h"
#include "Prism/Scene/Systems/TransformSystem.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Asset/ModelImporter.h"
#include "PythonScriptWrappers.h"
#include "PythonScriptTypeCasters.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <box2d/box2d.h>
#include <PhysX/PxPhysicsAPI.h>

namespace py = pybind11;
using namespace Prism;

namespace Prism
{
    extern std::unordered_map<uint64_t, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    extern std::unordered_map<uint64_t, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;
}
//  Helper
namespace Prism
{
    namespace PythonScript
    {

        static Entity GetEntityFromEntityID(uint64_t entityID)
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto& entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(),
                "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(entityID);
        }

        static TransformSystem* GetTransformSystem(Entity entity)
        {
            return entity.GetScene()->GetSystem<TransformSystem>();
        }
    }
} // namespace Prism::PythonScript

namespace Prism::PythonScript
{
    struct PythonTransform
    { glm::vec3 Position; glm::vec3 Rotation; glm::vec3 Scale; glm::vec3 Up; glm::vec3 Right; glm::vec3 Forward; };

    struct PythonBufferView
    {
        Py_buffer View{};
        bool Valid = false;

        explicit PythonBufferView(const py::object& object)
        {
            if (object.is_none())
                return;
            if (PyObject_GetBuffer(object.ptr(), &View, PyBUF_SIMPLE) == 0)
                Valid = true;
            else
                PyErr_Clear();
        }
        ~PythonBufferView() { if (Valid) PyBuffer_Release(&View); }

        PythonBufferView(const PythonBufferView&) = delete;
        PythonBufferView& operator=(const PythonBufferView&) = delete;

        const void* Data() const { return View.buf; }
        void* MutableData() const { return View.buf; }
        uint32_t Size() const { return Valid ? (uint32_t)View.len : 0; }
    };

    inline uint32_t GetExpectedPixelDataSize(ImageFormat format, uint32_t width, uint32_t height, uint32_t faces = 1)
    {
        // None 与压缩格式无法按 BPP 推算
        if (format == ImageFormat::None || Utils::IsCompressedFormat(format))
            return 0;
        return Utils::GetImageMemorySize(format, width, height) * faces;
    }

    inline void ValidatePixelData(const PythonBufferView& view, ImageFormat format, uint32_t width, uint32_t height, const char* api, uint32_t faces = 1)
    {
        const uint32_t expected = GetExpectedPixelDataSize(format, width, height, faces);
        if (expected != 0 && view.Size() < expected)
            throw std::runtime_error(fmt::format("{}: data needs at least {} bytes for {}x{}, got {}!"
                , api, expected, width, height, view.Size()));
    }

    class PythonNoise
    {
    public:
        static float PerlinNoise(float x, float y) { return Noise::PerlinNoise(x, y); }
    };

    class PythonInput
    {
    public:
        static bool IsKeyPressed(KeyCode key) { return Input::IsKeyPressed(key); }
        static glm::vec2 GetMousePosition() { auto [x, y] = Input::GetMousePosition(); return { x, y }; }
        static void SetCursorMode(CursorMode mode) { Input::SetCursorMode(mode); }
        static CursorMode GetCursorMode() { return Input::GetCursorMode(); }
        static bool IsMouseButtonPressed(MouseButton button) { return Input::IsMouseButtonPressed(button); }
    };

    class PythonTime
    {
    public:
        static float GetDeltaTime(py::object) { return Time::GetDeltaTime(); }
        static float GetUnscaledDeltaTime(py::object) { return Time::GetUnscaledDeltaTime(); }
        static float GetTime(py::object) { return Time::GetTime(); }
        static float GetUnscaledTime(py::object) { return Time::GetUnscaledTime(); }
        static int64_t GetFrameCount(py::object) { return (int64_t)Time::GetFrameCount(); }
        static float GetFixedDeltaTime(py::object) { return Time::GetFixedDeltaTime(); }
        static void SetFixedDeltaTime(py::object, float fixedDeltaTime) { Time::SetFixedDeltaTime(fixedDeltaTime); }
        static float GetTimeScale(py::object) { return Time::GetTimeScale(); }
        static void SetTimeScale(py::object, float scale) { Time::SetTimeScale(scale); }
    };

    class PythonLog
    {
    public:
        static void Trace(const char* message) { PR_CORE_TRACE("[Python] {}", message); }
        static void Debug(const char* message) { PR_CORE_TRACE("[Python] {}", message); }
        static void Info(const char* message) { PR_CORE_INFO("[Python] {}", message); }
        static void Warn(const char* message) { PR_CORE_WARN("[Python] {}", message); }
        static void Error(const char* message) { PR_CORE_ERROR("[Python] {}", message); }
        static void Critical(const char* message) { PR_CORE_FATAL("[Python] {}", message); }
    };
    class PythonRefCounted
    {
    protected:
        Ref<RefCounted> m_Ref;
    public:
        PythonRefCounted() = default;
        PythonRefCounted(Ref<RefCounted> ref) : m_Ref(std::move(ref)) {}
        void SetRef(uint64_t refPtr) { m_Ref = reinterpret_cast<RefCounted*>(refPtr); }
        virtual ~PythonRefCounted() = default;
        virtual std::string __Repr__() { return fmt::format(" <Ref Handle = {}>", (uint64_t)m_Ref.Raw()); }
    };

    class PythonAsset : public PythonRefCounted
    {
    public:
        PythonAsset() = default;
        PythonAsset(Ref<Asset> asset) : PythonRefCounted(std::move(asset)) {}
        void SetAsset(uint64_t assetPtr) { m_Ref = reinterpret_cast<Asset*>(assetPtr); }
        virtual ~PythonAsset() = default;
        virtual std::string __Repr__() { return fmt::format(" <Asset Handle = {}>", (uint64_t)m_Ref.Raw()); }

        AssetType GetType() const { Ref<Asset> asset = m_Ref.As<Asset>(); return asset ? asset->Type : AssetType::None; }
        uint64_t GetHandle() const { Ref<Asset> asset = m_Ref.As<Asset>(); return asset ? (uint64_t)asset->Handle : 0; }
        std::string GetFilePath() const { Ref<Asset> asset = m_Ref.As<Asset>(); return asset ? asset->FilePath : ""; }
        std::string GetFileName() const { Ref<Asset> asset = m_Ref.As<Asset>(); return asset ? asset->FileName : ""; }
        std::string GetExtension() const { Ref<Asset> asset = m_Ref.As<Asset>(); return asset ? asset->Extension : ""; }
    };

    class PythonMesh : public PythonAsset
    {
    public:
        PythonMesh() = default;
        PythonMesh(Ref<Mesh> mesh) : PythonAsset(mesh) {}
        PythonMesh(const char* filepath) : PythonAsset(ModelImporter::Import(filepath).Mesh) {}
        virtual std::string __Repr__() override { return fmt::format(" <Mesh Handle = {}>", (uint64_t)m_Ref.Raw()); }
    public:
        Ref<Mesh> GetMesh() const { return m_Ref.As<Mesh>(); }
    };

    class PythonImage : public PythonRefCounted
    {
    public:
        PythonImage() = default;
        PythonImage(Ref<Image> image) : PythonRefCounted(std::move(image)) {}
        virtual std::string __Repr__() override
        {
            Ref<Image> image = m_Ref.As<Image>();
            if (image)
                return fmt::format(" <Image Handle = {} Width = {} Height = {}>"
                    , (uint64_t)image.Raw(), image->GetWidth(), image->GetHeight());
            return fmt::format(" <Image Handle = {}>", (uint64_t)m_Ref.Raw());
        }
        uint32_t GetWidth() const { Ref<Image> image = m_Ref.As<Image>(); return image ? image->GetWidth() : 0; }
        uint32_t GetHeight() const { Ref<Image> image = m_Ref.As<Image>(); return image ? image->GetHeight() : 0; }
        uint32_t GetSamples() const { Ref<Image> image = m_Ref.As<Image>(); return image ? image->GetSamples() : 0; }
        ImageFormat GetFormat() const { Ref<Image> image = m_Ref.As<Image>(); return image ? image->GetFormat() : ImageFormat::None; }
        ImageUsage GetUsage() const { Ref<Image> image = m_Ref.As<Image>(); return image ? image->GetUsage() : ImageUsage::None; }
    public:
        Ref<Image> GetImage() const { return m_Ref.As<Image>(); }
    };

    class PythonImage2D : public PythonImage
    {
    public:
        PythonImage2D() = default;
        PythonImage2D(Ref<Image2D> image) : PythonImage(image) {}
        static PythonImage2D Create(const ImageSpecification& specification, const py::object& data)
        {
            PythonBufferView view(data);
            if (!data.is_none() && !view.Valid)
                throw std::runtime_error("Image2D.Create: data must support the buffer protocol!");
            ValidatePixelData(view, specification.Format, specification.Width, specification.Height, "Image2D.Create");

            Buffer imageData;
            if (view.Data())
                imageData = Buffer::Copy(view.Data(), Utils::GetImageMemorySize(specification.Format, specification.Width, specification.Height));

            return PythonImage2D(Image2D::Create(specification, std::move(imageData)));
        }
    public:
        Ref<Image2D> GetImage2D() const { return m_Ref.As<Image2D>(); }
    };

    class PythonImageCube : public PythonImage
    {
    public:
        PythonImageCube() = default;
        PythonImageCube(Ref<ImageCube> image) : PythonImage(image) {}
        static PythonImageCube Create(const ImageSpecification& specification, const py::object& data)
        {
            PythonBufferView view(data);
            if (!data.is_none() && !view.Valid)
                throw std::runtime_error("ImageCube.Create: data must support the buffer protocol!");
            ValidatePixelData(view, specification.Format, specification.Width, specification.Height, "ImageCube.Create", 6);

            Buffer imageData;
            if (view.Data())
                imageData = Buffer::Copy(view.Data(), Utils::GetImageMemorySize(specification.Format, specification.Width, specification.Height) * 6);

            return PythonImageCube(ImageCube::Create(specification, std::move(imageData)));
        }
        void GenerateMipMap() { Ref<ImageCube> image = GetImageCube(); if (image) image->GenerateMipMap(); }
        void CopyTo(const PythonImageCube& target) { Ref<ImageCube> image = GetImageCube(); if (image) image->CopyTo(target.GetImageCube()); }
    public:
        Ref<ImageCube> GetImageCube() const { return m_Ref.As<ImageCube>(); }
    };

    class PythonTexture : public PythonAsset
    {
    public:
        PythonTexture() = default;
        PythonTexture(Ref<Texture> texture) : PythonAsset(std::move(texture)) {}
        Ref<Texture> GetTexture() const { return m_Ref.As<Texture>(); }
        uint32_t GetWidth() const { Ref<Texture> texture = m_Ref.As<Texture>(); return texture ? texture->GetWidth() : 0; }
        uint32_t GetHeight() const { Ref<Texture> texture = m_Ref.As<Texture>(); return texture ? texture->GetHeight() : 0; }
        ImageFormat GetFormat() const { Ref<Texture> texture = m_Ref.As<Texture>(); return texture ? texture->GetFormat() : ImageFormat::None; }
    };

    class PythonTexture2D : public PythonTexture
    {
    public:
        PythonTexture2D() = default;
        PythonTexture2D(Ref<Texture2D> texture) : PythonTexture(texture) {}
        static PythonTexture2D Create(uint32_t width, uint32_t height)
        {
            return PythonTexture2D(Texture2D::Create(ImageFormat::RGBA8, width, height));
        }
        virtual std::string __Repr__() override
        {
            Ref<Texture2D> texture = GetTexture();
            if (texture)
                return fmt::format(" <Texture2D Handle = {} Width = {} height = {}>"
                    , (uint64_t)texture.Raw(), texture->GetWidth(), texture->GetHeight());
            return fmt::format(" <Texture2D Handle = {}>", (uint64_t)m_Ref.Raw());
        }
        void SetData(const py::object& data)
        {
            Ref<Texture2D> texture = GetTexture();
            if (!texture)
                throw std::runtime_error("Texture2D.SetData: invalid texture!");
            PythonBufferView view(data);
            if (!view.Valid)
                throw std::runtime_error("Texture2D.SetData: data must support the buffer protocol!");
            const uint32_t expected = texture->GetWidth() * texture->GetHeight() * 4;
            if (view.Size() != expected)
                throw std::runtime_error(fmt::format("Texture2D.SetData: expected {} RGBA8 bytes, got {}!", expected, view.Size()));

            texture->Lock();
            // GetWriteableBuffer() 按值返回，取到的是 Buffer 深拷贝，写入会丢失；必须取 Image2D 的 Buffer 引用
            Ref<Image2D> image = texture->GetImage();
            if (!image)
            {
                texture->Unlock();
                throw std::runtime_error("Texture2D.SetData: texture has no image!");
            }
            Buffer& buffer = image->GetBuffer();
            std::memcpy(buffer.Data, view.Data(), expected);
            texture->Unlock();
        }
        PythonImage2D GetImage() const
        {
            Ref<Texture2D> texture = GetTexture();
            return texture ? PythonImage2D(texture->GetImage()) : PythonImage2D();
        }
    public:
        Ref<Texture2D> GetTexture() const { return m_Ref.As<Texture2D>(); }
    };

    class PythonTextureCube : public PythonTexture
    {
    public:
        PythonTextureCube() = default;
        PythonTextureCube(Ref<TextureCube> texture) : PythonTexture(texture) {}
        static PythonTextureCube Create(ImageFormat format, uint32_t width, uint32_t height, const py::object& data)
        {
            PythonBufferView view(data);
            if (!data.is_none() && !view.Valid)
                throw std::runtime_error("TextureCube.Create: data must support the buffer protocol!");
            ValidatePixelData(view, format, width, height, "TextureCube.Create", 6);
            return PythonTextureCube(TextureCube::Create(format, width, height, view.Data()));
        }
        virtual std::string __Repr__() override
        {
            Ref<TextureCube> texture = GetTextureCube();
            if (texture)
                return fmt::format(" <TextureCube Handle = {} Width = {} height = {}>"
                    , (uint64_t)texture.Raw(), texture->GetWidth(), texture->GetHeight());
            return fmt::format(" <TextureCube Handle = {}>", (uint64_t)m_Ref.Raw());
        }
        PythonImageCube GetImage() const
        {
            Ref<TextureCube> texture = GetTextureCube();
            return texture ? PythonImageCube(texture->GetImage()) : PythonImageCube();
        }
    public:
        Ref<TextureCube> GetTextureCube() const { return m_Ref.As<TextureCube>(); }
    };

    class PythonPrismShader : public PythonAsset
    {
    public:
        PythonPrismShader() = default;
        PythonPrismShader(Ref<PrismShader> shader) : PythonAsset(shader) {}
        static PythonPrismShader GetShader(const char* name)
        {
            return PythonPrismShader(AssetManager::GetShaderLibrary()->Get(name));
        }
        virtual std::string __Repr__() override
        {
            Ref<PrismShader> shader = m_Ref.As<PrismShader>();
            if (shader)
                return fmt::format(" <Shader Handle = {} Name = {}>", (uint64_t)shader.Raw(), shader->GetName());
            return fmt::format(" <Shader Handle = {}>", (uint64_t)m_Ref.Raw());
        }
        std::string GetName() const
        {
            Ref<PrismShader> shader = m_Ref.As<PrismShader>();
            return shader ? shader->GetName() : "";
        }
        uint32_t GetUniformCount() const
        {
            Ref<PrismShader> shader = m_Ref.As<PrismShader>();
            return shader ? (uint32_t)shader->GetUniforms().size() : 0;
        }
        PrismShaderCompiler::PropertyType GetUniformType(uint32_t index) const
        {
            Ref<PrismShader> shader = m_Ref.As<PrismShader>();
            if (!shader) return PrismShaderCompiler::PropertyType::None;
            const auto& uniforms = shader->GetUniforms();
            if (index >= uniforms.size()) return PrismShaderCompiler::PropertyType::None;
            return uniforms[index].Type;
        }
        std::string GetUniformName(uint32_t index) const
        {
            Ref<PrismShader> shader = m_Ref.As<PrismShader>();
            if (!shader) return "";
            const auto& uniforms = shader->GetUniforms();
            if (index >= uniforms.size()) return "";
            return uniforms[index].Name;
        }
        std::string GetUniformDisplayName(uint32_t index) const
        {
            Ref<PrismShader> shader = m_Ref.As<PrismShader>();
            if (!shader) return "";
            const auto& uniforms = shader->GetUniforms();
            if (index >= uniforms.size()) return "";
            return uniforms[index].DisplayName;
        }
        py::object GetUniformDefaultValue(uint32_t index) const
        {
            Ref<PrismShader> shader = m_Ref.As<PrismShader>();
            if (!shader) return py::none();
            const auto& uniforms = shader->GetUniforms();
            if (index >= uniforms.size()) return py::none();
            const auto& uni = uniforms[index];
            const auto& dv = uni.DefaultValue;
            using PT = PrismShaderCompiler::PropertyType;
            switch (uni.Type)
            {
            case PT::Float:
            case PT::Range:
                return dv.empty() ? py::none() : py::cast(dv[0].Float);
            case PT::Int:
            case PT::Enum:
                return dv.empty() ? py::none() : py::cast(dv[0].Int);
            case PT::Bool:
                return dv.empty() ? py::none() : py::cast(dv[0].Bool);
            case PT::Vector2:
                if (dv.size() >= 2) return py::cast(glm::vec2(dv[0].Float, dv[1].Float));
                return py::none();
            case PT::Vector3:
            case PT::Color3:
                if (dv.size() >= 3) return py::cast(glm::vec3(dv[0].Float, dv[1].Float, dv[2].Float));
                return py::none();
            case PT::Vector4:
            case PT::Color:
                if (dv.size() >= 4) return py::cast(glm::vec4(dv[0].Float, dv[1].Float, dv[2].Float, dv[3].Float));
                return py::none();
            case PT::Matrix4:
            {
                if (dv.size() >= 16)
                {
                    glm::mat4 m(1.0f);
                    for (int i = 0; i < 16; ++i) glm::value_ptr(m)[i] = dv[i].Float;
                    return py::cast(m);
                }
                return py::none();
            }
            case PT::Matrix3:
            case PT::Texture2D:
            case PT::Texture2DMS:
            case PT::TextureCube:
            case PT::None:
            default:
                return py::none();
            }
        }
    public:
        Ref<PrismShader> GetShaderRef() const { return m_Ref.As<PrismShader>(); }
    };

    class PythonMaterial : public PythonRefCounted
    {
    public:
        PythonMaterial() = default;
        PythonMaterial(Ref<Material> material) : PythonRefCounted(std::move(material)) {}
        PythonMaterial(const PythonPrismShader& shader)
            : PythonRefCounted(Material::Create(shader.GetShaderRef())) {}
        virtual std::string __Repr__()
        {
            std::string result;
            if (GetMaterial())
            {
                result = fmt::format(" <Material Handle = {} Shader = {}>"
                    , (uint64_t)m_Ref.Raw(), GetMaterial()->GetShader()->GetName());
            }
            else result = fmt::format(" <Material Handle = {}>", (uint64_t)m_Ref.Raw());
            return result;
        }
        void SetFloat(const char* uniform, float value) { GetMaterial()->SetFloat(uniform, value); }
        void SetInt(const char* uniform, int value) { GetMaterial()->SetInt(uniform, value); }
        void SetBool(const char* uniform, bool value) { GetMaterial()->SetBool(uniform, value); }
        void SetVector2(const char* uniform, const glm::vec2& value) { GetMaterial()->SetVec2(uniform, value); }
        void SetVector3(const char* uniform, const glm::vec3& value) { GetMaterial()->SetVec3(uniform, value); }
        void SetVector4(const char* uniform, const glm::vec4& value) { GetMaterial()->SetVec4(uniform, value); }
        void SetColor3(const char* uniform, const glm::vec3& value) { GetMaterial()->SetColor3(uniform, value); }
        void SetColor(const char* uniform, const glm::vec4& value) { GetMaterial()->SetColor(uniform, value); }
        void SetMatrix4(const char* uniform, const glm::mat4& value) { GetMaterial()->SetMatrix4(uniform, value); }
        void SetTexture2D(const char* uniform, const PythonTexture2D& texture) { GetMaterial()->SetTexture(uniform, texture.GetTexture()); }
        void SetKeyword(const char* name, bool enabled) { GetMaterial()->SetKeyword(name, enabled); }
        bool IsKeywordEnabled(const char* name) { return GetMaterial()->IsKeywordEnabled(name); }
    public:
        Ref<Material> GetMaterial() const { return m_Ref.As<Material>(); }
    };

    class PythonUniformBuffer : public PythonRefCounted
    {
    public:
        PythonUniformBuffer() = default;
        PythonUniformBuffer(Ref<UniformBuffer> buffer) : PythonRefCounted(std::move(buffer)) {}
        static PythonUniformBuffer Create(uint32_t size)
        {
            return PythonUniformBuffer(UniformBuffer::Create(size));
        }
        virtual std::string __Repr__() override
        {
            Ref<UniformBuffer> buffer = GetUniformBuffer();
            if (buffer)
                return fmt::format(" <UniformBuffer Handle = {} Size = {}>", (uint64_t)buffer.Raw(), buffer->GetSize());
            return fmt::format(" <UniformBuffer Handle = {}>", (uint64_t)m_Ref.Raw());
        }
        void SetData(const py::object& data, uint32_t offset)
        {
            Ref<UniformBuffer> buffer = GetUniformBuffer();
            if (!buffer)
                throw std::runtime_error("UniformBuffer.SetData: invalid uniform buffer!");
            PythonBufferView view(data);
            if (!view.Valid)
                throw std::runtime_error("UniformBuffer.SetData: data must support the buffer protocol!");
            if ((uint64_t)offset + view.Size() > buffer->GetSize())
                throw std::runtime_error(fmt::format("UniformBuffer.SetData: write range [{}, {}) exceeds buffer size {}!"
                    , offset, offset + view.Size(), buffer->GetSize()));
            buffer->SetData(view.Data(), view.Size(), offset);
        }
    public:
        Ref<UniformBuffer> GetUniformBuffer() const { return m_Ref.As<UniformBuffer>(); }
    };

    class PythonShaderStorageBuffer : public PythonRefCounted
    {
    public:
        PythonShaderStorageBuffer() = default;
        PythonShaderStorageBuffer(Ref<ShaderStorageBuffer> buffer) : PythonRefCounted(std::move(buffer)) {}
        static PythonShaderStorageBuffer Create(uint32_t size, uint32_t usage)
        {
            return PythonShaderStorageBuffer(ShaderStorageBuffer::Create(size, (BufferUsage)usage));
        }
        virtual std::string __Repr__() override
        {
            Ref<ShaderStorageBuffer> buffer = GetShaderStorageBuffer();
            if (buffer)
                return fmt::format(" <ShaderStorageBuffer Handle = {} Size = {}>", (uint64_t)buffer.Raw(), buffer->GetSize());
            return fmt::format(" <ShaderStorageBuffer Handle = {}>", (uint64_t)m_Ref.Raw());
        }
        void SetData(const py::object& data, uint32_t offset)
        {
            Ref<ShaderStorageBuffer> buffer = GetShaderStorageBuffer();
            if (!buffer)
                throw std::runtime_error("ShaderStorageBuffer.SetData: invalid buffer!");
            PythonBufferView view(data);
            if (!view.Valid)
                throw std::runtime_error("ShaderStorageBuffer.SetData: data must support the buffer protocol!");
            if ((uint64_t)offset + view.Size() > buffer->GetSize())
                throw std::runtime_error(fmt::format("ShaderStorageBuffer.SetData: write range [{}, {}) exceeds buffer size {}!"
                    , offset, offset + view.Size(), buffer->GetSize()));
            buffer->SetData(view.Data(), view.Size(), offset);
        }
        void GetData(const py::object& data, uint32_t offset, bool sync)
        {
            Ref<ShaderStorageBuffer> buffer = GetShaderStorageBuffer();
            if (!buffer)
                throw std::runtime_error("ShaderStorageBuffer.GetData: invalid buffer!");
            PythonBufferView view(data);
            if (!view.Valid)
                throw std::runtime_error("ShaderStorageBuffer.GetData: data must support the buffer protocol!");
            if ((uint64_t)offset + view.Size() > buffer->GetSize())
                throw std::runtime_error(fmt::format("ShaderStorageBuffer.GetData: read range [{}, {}) exceeds buffer size {}!"
                    , offset, offset + view.Size(), buffer->GetSize()));
            buffer->GetData(view.MutableData(), view.Size(), offset, sync);
        }
        uint32_t GetSize() const
        {
            Ref<ShaderStorageBuffer> buffer = GetShaderStorageBuffer();
            return buffer ? (uint32_t)buffer->GetSize() : 0;
        }
    public:
        Ref<ShaderStorageBuffer> GetShaderStorageBuffer() const { return m_Ref.As<ShaderStorageBuffer>(); }
    };

    class PythonComputeShader : public PythonAsset
    {
    public:
        PythonComputeShader() = default;
        PythonComputeShader(Ref<ComputeShader> shader) : PythonAsset(std::move(shader)) {}
        static PythonComputeShader Create(const char* filePath)
        {
            AssetHandle handle = AssetManager::GetAssetHandleFromFilePath(filePath);
            if (!AssetManager::IsAssetHandleValid(handle))
                throw std::runtime_error(fmt::format("ComputeShader.Create: '{}' is not a registered asset!", filePath));

            Ref<ComputeShader> shader = AssetManager::GetAsset<ComputeShader>(handle);
            if (!shader)
                throw std::runtime_error(fmt::format("ComputeShader.Create: failed to load '{}'!", filePath));
            return PythonComputeShader(shader);
        }
        virtual std::string __Repr__() override
        {
            Ref<ComputeShader> shader = GetComputeShader();
            if (shader)
                return fmt::format(" <ComputeShader Handle = {} Name = '{}'>", (uint64_t)shader.Raw(), shader->GetName());
            return fmt::format(" <ComputeShader Handle = {}>", (uint64_t)m_Ref.Raw());
        }
        std::string GetName() const
        {
            Ref<ComputeShader> shader = GetComputeShader();
            return shader ? shader->GetName() : "";
        }
        uint32_t GetKernelCount() const
        {
            Ref<ComputeShader> shader = GetComputeShader();
            return shader ? (uint32_t)shader->GetKernelCount() : 0;
        }
        int32_t FindKernel(const char* name) const
        {
            Ref<ComputeShader> shader = GetComputeShader();
            return shader ? shader->FindKernel(name) : -1;
        }
        bool HasKernel(const char* name) const
        {
            Ref<ComputeShader> shader = GetComputeShader();
            return shader ? shader->HasKernel(name) : false;
        }
        py::tuple GetKernelThreadGroupSizes(int32_t kernel) const
        {
            uint32_t x = 1, y = 1, z = 1;
            Ref<ComputeShader> shader = GetComputeShader();
            if (shader)
                shader->GetKernelThreadGroupSizes(kernel, x, y, z);
            return py::make_tuple(x, y, z);
        }
        void SetUniformBuffer(int32_t kernel, const char* name, const PythonUniformBuffer& buffer)
        {
            Ref<ComputeShader> shader = RequireComputeShader("SetUniformBuffer");
            shader->SetUniformBuffer(kernel, name, buffer.GetUniformBuffer());
        }
        void SetBuffer(int32_t kernel, const char* name, const PythonShaderStorageBuffer& buffer)
        {
            Ref<ComputeShader> shader = RequireComputeShader("SetBuffer");
            shader->SetBuffer(kernel, name, buffer.GetShaderStorageBuffer());
        }
        void SetTexture2D(int32_t kernel, const char* name, const PythonImage2D& image)
        {
            Ref<ComputeShader> shader = RequireComputeShader("SetTexture2D");
            shader->SetTexture2D(kernel, name, image.GetImage2D());
        }
        void SetTextureCube(int32_t kernel, const char* name, const PythonImageCube& image)
        {
            Ref<ComputeShader> shader = RequireComputeShader("SetTextureCube");
            shader->SetTextureCube(kernel, name, image.GetImageCube());
        }
        void SetImage2D(int32_t kernel, const char* name, const PythonImage2D& image, uint32_t level)
        {
            Ref<ComputeShader> shader = RequireComputeShader("SetImage2D");
            shader->SetImage2D(kernel, name, image.GetImage2D(), level);
        }
        void SetImageCube(int32_t kernel, const char* name, const PythonImageCube& image, uint32_t level)
        {
            Ref<ComputeShader> shader = RequireComputeShader("SetImageCube");
            shader->SetImageCube(kernel, name, image.GetImageCube(), level);
        }
        void Dispatch(int32_t kernel, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
        {
            Ref<ComputeShader> shader = RequireComputeShader("Dispatch");
            shader->Dispatch(kernel, groupsX, groupsY, groupsZ);
        }
    public:
        Ref<ComputeShader> GetComputeShader() const { return m_Ref.As<ComputeShader>(); }
    private:
        Ref<ComputeShader> RequireComputeShader(const char* method) const
        {
            Ref<ComputeShader> shader = GetComputeShader();
            if (!shader)
                throw std::runtime_error(fmt::format("ComputeShader.{}: invalid compute shader!", method));
            return shader;
        }
    };

    class PythonEntity
    {
        uint64_t m_EntityID = 0;

    public:
        PythonEntity(uint64_t id = 0) : m_EntityID(id) {}
        ~PythonEntity() { /*PR_CORE_TRACE("[Python] Destroyed Entity {0}", m_EntityID);*/ }
        std::string __Repr__() { return fmt::format(" <Entity ID = {}>", m_EntityID); }

        uint64_t GetID() const { return m_EntityID; }
        void SetID(uint64_t id)
        {
            m_EntityID = id;
            // PR_CORE_TRACE("[Python] Created Entity {0}", id);
        }

        bool __Eq__(const PythonEntity& other) const { return m_EntityID == other.m_EntityID; }
        bool __Ne__(const PythonEntity& other) const { return m_EntityID != other.m_EntityID; }
        int64_t __Hash__() const { return (int64_t)m_EntityID; }

        pybind11::object GetComponent(pybind11::object cls)
        {
            py::object behaviourClass = py::module::import("PrismEngine").attr("Behaviour");

            if (PyObject_IsSubclass(cls.ptr(), behaviourClass.ptr()) && cls.ptr() != behaviourClass.ptr())
            {
                // Behaviour lookup
                Entity entity = GetEntityFromEntityID(m_EntityID);
                std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
                UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
                auto& comp = entity.GetComponent<Prism::PythonScriptComponent>();
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (binding.ClassID == classID)
                    {
                        UUID sceneID = PythonScriptEngine::GetCurrentSceneContext()->GetUUID();
                        py::object* obj = PythonScriptEngine::GetScriptObject(sceneID, UUID(bid));
                        if (obj) return *obj;
                        break;
                    }
                }
                return py::none();
            }

            // Component lookup
            Entity entity = GetEntityFromEntityID(m_EntityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            if (s_PythonHasComponentFuncs.count(typeId) && s_PythonHasComponentFuncs.at(typeId)(entity))
            {
                py::object component = cls();
                component.attr("Entity") = py::cast(PythonEntity(m_EntityID));
                return component;
            }
            return py::none();
        }

        pybind11::object CreateComponent(pybind11::object cls)
        {
            py::object behaviourClass = py::module::import("PrismEngine").attr("Behaviour");

            if (PyObject_IsSubclass(cls.ptr(), behaviourClass.ptr()) && cls.ptr() != behaviourClass.ptr())
            {
                Entity entity = GetEntityFromEntityID(m_EntityID);
                std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
                UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
                auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
                uint64_t bid = (uint64_t)ss->AddPythonBehaviour(entity, classID);
                UUID sceneID = PythonScriptEngine::GetCurrentSceneContext()->GetUUID();
                py::object* obj = PythonScriptEngine::GetScriptObject(sceneID, UUID(bid));
                if (obj) return *obj;
                return py::none();
            }

            Entity entity = GetEntityFromEntityID(m_EntityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            s_PythonCreateComponentFuncs.at(typeId)(entity);
            py::object component = cls();
            component.attr("Entity") = py::cast(PythonEntity(m_EntityID));
            return component;
        }

        bool HasComponent(pybind11::object cls)
        {
            py::object behaviourClass = py::module::import("PrismEngine").attr("Behaviour");

            if (PyObject_IsSubclass(cls.ptr(), behaviourClass.ptr()) && cls.ptr() != behaviourClass.ptr())
            {
                Entity entity = GetEntityFromEntityID(m_EntityID);
                std::string className = cls.attr("__module__").cast<std::string>() + "." + cls.attr("__qualname__").cast<std::string>();
                UUID classID = PythonScriptMetaRegistry::GenerateClassID(className);
                auto& comp = entity.GetComponent<Prism::PythonScriptComponent>();
                for (auto& [bid, binding] : comp.Behaviours)
                    if (binding.ClassID == classID) return true;
                return false;
            }

            Entity entity = GetEntityFromEntityID(m_EntityID);
            uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
            return s_PythonHasComponentFuncs.count(typeId) && s_PythonHasComponentFuncs.at(typeId)(entity);
        }

        pybind11::object GetTransform()
        {
            py::object transformCompClass = py::module::import("PrismEngine").attr("TransformComponent");
            return GetComponent(transformCompClass);
        }

        static pybind11::object FindEntityByTag(const char* tag)
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            std::string tagStr(tag);
            const auto& entityMap = scene->GetEntityMap();
            for (const auto& [id, entity] : entityMap)
            {
                if (entity.HasComponent<TagComponent>() &&
                    entity.GetComponent<TagComponent>().Tag == tagStr)
                    return py::cast(PythonEntity(id));
            }
            return py::none();
        }

        static pybind11::object FindEntityByID(uint64_t id)
        {
            return py::cast(PythonEntity(id));
        }
    };

    class PythonComponent
    {
    protected:
        pybind11::object m_Entity;
        uint64_t m_EntityID = 0;

    public:
        virtual ~PythonComponent() = default;
        virtual std::string __Repr__() { return fmt::format(" <Component EntityID = {}>", m_EntityID); }
        pybind11::object GetEntity() const { return m_Entity; }

        void SetEntity(pybind11::object entity)
        {
            m_Entity = entity;
            m_EntityID = entity.attr("ID").cast<uint64_t>();
        }
        
    protected:
        Entity GetEntityImpt() const
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto& entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(m_EntityID) != entityMap.end(),
                "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(m_EntityID);
        }
    };

    class PythonTagComponent : public PythonComponent
    {
        public:
        std::string GetTag() const { return GetEntityImpt().GetComponent<TagComponent>().Tag; }
        void SetTag(const std::string& tag) { GetEntityImpt().GetComponent<TagComponent>().Tag = tag; }
        virtual std::string __Repr__() override { return fmt::format(" <TagComponent Tag = {}>", GetTag()); }
    };

    class PythonScriptComponent : public PythonComponent
    {
    };

    class PythonCameraComponent : public PythonComponent
    {
    public:
        virtual std::string __Repr__() override { return fmt::format(" <CameraComponent> {}", m_EntityID); }
    };

    class PythonSpriteRendererComponent : public PythonComponent
    {
    };

    class PythonBoxCollider2DComponent : public PythonComponent
    {
    };

    class PythonCircleCollider2DComponent : public PythonComponent
    {
    };

    class PythonRigidBody2DComponent : public PythonComponent
    {
    public:
        virtual std::string __Repr__() override { return fmt::format(" <RigidBody2DComponent> {}", m_EntityID); }
        void ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& offset, bool wake)
        {
            Entity entity = GetEntityImpt();
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), b2Vec2(offset.x, offset.y), wake);
        }
        glm::vec2 GetLinearVelocity() const
        {
            Entity entity = GetEntityImpt();
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            const b2Vec2& v = body->GetLinearVelocity();
            return { v.x, v.y };
        }
        void SetLinearVelocity(const glm::vec2& velocity)
        {
            Entity entity = GetEntityImpt();
            auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
        }
    };

    class PythonRigidBodyComponent : public PythonComponent
    {
    public:
        virtual std::string __Repr__() override { return fmt::format(" <RigidBodyComponent> {}", m_EntityID); }
        void AddForce(const glm::vec3& force, ForceMode mode)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
            actor->AddForce(force, (ForceMode)mode);
        }
        void AddTorque(const glm::vec3& torque, ForceMode mode)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
            actor->AddTorque(torque, (ForceMode)mode);
        }
        glm::vec3 GetLinearVelocity() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetLinearVelocity();
        }
        void SetLinearVelocity(const glm::vec3& velocity)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetLinearVelocity(velocity);
        }
        void Rotate(const glm::vec3& rotation)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->Rotate(rotation);
        }
        float GetMass() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetMass();
        }
        void SetMass(float mass)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetMass(mass);
        }
        glm::vec3 GetAngularVelocity() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return Physics::GetActorForEntity(entity)->GetAngularVelocity();
        }
        void SetAngularVelocity(const glm::vec3& velocity)
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            Physics::GetActorForEntity(entity)->SetAngularVelocity(velocity);
        }
        uint32_t GetLayer() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return entity.GetComponent<RigidBodyComponent>().Layer;
        }
        uint32_t GetBodyType() const
        {
            Entity entity = GetEntityImpt();
            PR_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "No RigidBodyComponent!");
            return (uint32_t)entity.GetComponent<RigidBodyComponent>().BodyType;
        }

    };

    class PythonBoxColliderComponent : public PythonComponent
    {
    };

    class PythonSphereColliderComponent : public PythonComponent
    {
    };

    class PythonCapsuleColliderComponent : public PythonComponent
    {
    };

    class PythonTransformComponent : public PythonComponent
    {
    public:
        glm::vec3 GetPosition() const { return GetEntityImpt().Transformation().GetPosition(); }
        void SetPosition(const glm::vec3& v) { GetEntityImpt().Transformation().SetPosition(v); }
        glm::vec3 GetRotation() const { return GetEntityImpt().Transformation().GetRotation(); }
        void SetRotation(const glm::vec3& v) { GetEntityImpt().Transformation().SetRotation(v); }
        glm::vec3 GetScale() const { return GetEntityImpt().Transformation().GetScale(); }
        void SetScale(const glm::vec3& v) { GetEntityImpt().Transformation().SetScale(v); }
        glm::vec3 GetLocalPosition() const { return GetEntityImpt().Transformation().GetPosition(); }
        void SetLocalPosition(const glm::vec3& v) { GetEntityImpt().Transformation().SetPosition(v); }
        glm::vec3 GetLocalRotation() const { return GetEntityImpt().Transformation().GetRotation(); }
        void SetLocalRotation(const glm::vec3& v) { GetEntityImpt().Transformation().SetRotation(v); }
        glm::vec3 GetLocalScale() const { return GetEntityImpt().Transformation().GetScale(); }
        void SetLocalScale(const glm::vec3& v) { GetEntityImpt().Transformation().SetScale(v); }

        glm::vec3 GetForward() const { return GetEntityImpt().Transformation().Forward; }
        glm::vec3 GetRight() const { return GetEntityImpt().Transformation().Right; }
        glm::vec3 GetUp() const { return GetEntityImpt().Transformation().Up; }

        PythonTransform GetTransform() const {
            Entity e = GetEntityImpt();
            auto world = GetTransformSystem(e)->GetWorldDecomposed(e);
            auto& tc = e.Transformation();
            return { world.Position, world.Rotation, world.Scale, tc.Up, tc.Right, tc.Forward };
        }
        void SetTransform(const PythonTransform& t) {
            Entity e = GetEntityImpt();
            auto* ts = GetTransformSystem(e);
            ts->SetWorldPosition(e, t.Position);
            ts->SetWorldRotation(e, t.Rotation);
            ts->SetWorldScale(e, t.Scale);
        }
    };

    class PythonMeshRendererComponent : public PythonComponent
    {
    public:
        PythonMesh GetMesh() const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            return PythonMesh(mc.Mesh);
        }
        void SetMesh(const PythonMesh& mesh)
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            mc.Mesh = mesh.GetMesh();
        }
        PythonMaterial GetMaterial(uint32_t index = 0) const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            if (!mc.Materials.empty() && mc.Materials[index])
                return PythonMaterial(mc.Materials[index]);
            return PythonMaterial();
        }
        void SetMaterial(const PythonMaterial& material, uint32_t index = 0)
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            if (!mc.Materials.empty())
            {
                if (material.GetMaterial())
                    mc.Materials[index] = material.GetMaterial();
                else PR_CORE_WARN("[Python] Attempted to set null material on MeshRendererComponent!");
            }
        }
        std::vector<PythonMaterial> GetMaterials() const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            std::vector<PythonMaterial> result;
            result.reserve(mc.Materials.size());
            for (auto& mat : mc.Materials)
                result.emplace_back(mat);
            return result;
        }
        void SetMaterials(const std::vector<PythonMaterial>& materials)
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            mc.Materials.resize(materials.size());
            for (size_t i = 0; i < materials.size(); ++i)
                mc.Materials[i] = materials[i].GetMaterial();
        }
        uint32_t GetMaterialCount() const
        {
            Entity e = GetEntityImpt();
            auto& mc = e.GetComponent<MeshRendererComponent>();
            return (uint32_t)mc.Materials.size();
        }
    };

    class PythonBehaviour : public PythonComponent
    {
        uint64_t m_BehaviourID = 0;

    public:
        uint64_t GetID() const { return m_BehaviourID; }
        void SetID(uint64_t id) { m_BehaviourID = id; }

        bool GetEnabled() {
            auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            return ss->GetEnabled(UUID(m_BehaviourID));
        }
        void SetEnabled(bool enabled) {
            auto* ss = PythonScriptEngine::GetCurrentSceneContext()->GetSystem<ScriptSystem>();
            ss->SetEnabled(UUID(m_BehaviourID), enabled);
        }

        pybind11::object GetTransform()
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("Transform");
            return pybind11::none();
        }

        pybind11::object GetComponent(pybind11::object cls)
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("GetComponent")(cls);
            return pybind11::none();
        }

        bool HasComponent(pybind11::object cls)
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("HasComponent")(cls).cast<bool>();
            return false;
        }

        pybind11::object CreateComponent(pybind11::object cls)
        {
            if (!m_Entity.is_none())
                return m_Entity.attr("CreateComponent")(cls);
            return pybind11::none();
        }
    };

    class PythonCollider
    {
    private:
        PythonEntity m_Entity;
        bool m_IsTrigger = false;
    public:
        PythonCollider(const PythonEntity& entity, bool isTrigger)
            : m_Entity(entity), m_IsTrigger(isTrigger) {}
        virtual ~PythonCollider() = default;
        virtual std::string __Repr__() { return fmt::format(" <Collider>"); }
        virtual std::string __Str__() { return fmt::format("Collider({}, {}, {})", GetColliderType(), m_Entity.GetID(), m_IsTrigger); }
        PythonEntity GetEntity() const { return m_Entity; }
        PythonRigidBodyComponent GetRigidBody() const
        {
            Entity entity = GetEntityImpt();
            if (entity.HasComponent<RigidBodyComponent>())
            {
                PythonRigidBodyComponent rb;
                rb.SetEntity(py::cast(m_Entity));
                return rb;
            }
            throw std::runtime_error("Entity does not have a RigidBodyComponent!");
        }
        bool IsTrigger() const { return m_IsTrigger; }
    protected:
        Entity GetEntityImpt() const
        {
            WeakRef<Scene> scene = PythonScriptEngine::GetCurrentSceneContext();
            PR_CORE_ASSERT(scene, "No active scene!");
            const auto& entityMap = scene->GetEntityMap();
            PR_CORE_ASSERT(entityMap.find(m_Entity.GetID()) != entityMap.end(),
                "Invalid entity ID or entity doesn't exist in scene!");
            return entityMap.at(m_Entity.GetID());
        }
        virtual std::string GetColliderType() const { return "Collder"; }
    };

    class PythonBoxCollider : public PythonCollider
    {
    private:
        glm::vec3 m_Size;
        glm::vec3 m_Offset;
    public:
        PythonBoxCollider(const PythonEntity& entity, bool isTrigger, const glm::vec3& size, const glm::vec3& offset)
            : PythonCollider(entity, isTrigger), m_Size(size), m_Offset(offset) {}
        glm::vec3 GetSize() const { return m_Size; }
        glm::vec3 GetOffset() const { return m_Offset; }
    protected:
        virtual std::string GetColliderType() const override { return "BoxCollider"; }
    };

    class PythonSphereCollider : public PythonCollider
    {
    private:
        float m_Radius;
    public:
        PythonSphereCollider(const PythonEntity& entity, bool isTrigger, float radius)
            : PythonCollider(entity, isTrigger), m_Radius(radius) {}
        float GetRadius() const { return m_Radius; }
    protected:
        virtual std::string GetColliderType() const override { return "SphereCollider"; }
    };

    class PythonCapsuleCollider : public PythonCollider
    {
    private:
        float m_Radius;
        float m_Height;
    public:
        PythonCapsuleCollider(const PythonEntity& entity, bool isTrigger, float radius, float height)
            : PythonCollider(entity, isTrigger), m_Radius(radius), m_Height(height) {}
        float GetRadius() const { return m_Radius; }
        float GetHeight() const { return m_Height; }
    protected:
        virtual std::string GetColliderType() const override { return "CapsuleCollider"; }
    };

    class PythonMeshCollider : public PythonCollider
    {
    private:
        PythonMesh m_Mesh;
    public:
        PythonMeshCollider(const PythonEntity& entity, bool isTrigger, const PythonMesh& mesh )
            : PythonCollider(entity, isTrigger), m_Mesh(mesh) {}
        PythonMesh GetMesh() const { return m_Mesh; }
    protected:
        virtual std::string GetColliderType() const override { return "MeshCollider"; }
    };

    class PythonPhysics
    {
        using ColliderList = std::vector<std::shared_ptr<PythonCollider>>;
    public:
        static float GetGravity() { return Physics::GetGravity(); }
        static void SetGravity(float gravity) { Physics::SetGravity(gravity); }
        static bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* raycastHit)
        {
            RaycastHit hit;
            if (PXPhysicsWrappers::Raycast(origin, direction, maxDistance, &hit))
            {
                if (raycastHit) *raycastHit = hit;
                return true;
            }
            return false;
        }
        static ColliderList OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            ColliderList results;
            if (PXPhysicsWrappers::OverlapBox(origin, halfSize, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }
        static ColliderList OverlapSphere(const glm::vec3& origin, float radius)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            ColliderList results;
            if (PXPhysicsWrappers::OverlapSphere(origin, radius, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }
        static ColliderList OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight)
        {
            std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> buffer;
            uint32_t count;
            ColliderList results;
            if (PXPhysicsWrappers::OverlapCapsule(origin, radius, halfHeight, buffer, &count))
            {
                results.reserve(count);
                for (uint32_t i = 0; i < count; ++i)
                    results.push_back(FillOverlapHit(buffer[i]));
            }
            return results;
        }
    private:
        static std::shared_ptr<PythonCollider> FillOverlapHit(physx::PxOverlapHit& pxHit)
        {
            Entity& entity = *(Entity*)pxHit.actor->userData;
            if (entity.HasComponent<BoxColliderComponent>())
            {
                auto& bc = entity.GetComponent<BoxColliderComponent>();
                auto boxCollider = std::make_shared<PythonBoxCollider>(
                    PythonEntity(entity.GetUUID()), bc.IsTrigger, bc.Size, bc.Offset
                );
                return boxCollider;
            }
            else if (entity.HasComponent<SphereColliderComponent>())
            {
                auto& sc = entity.GetComponent<SphereColliderComponent>();
                auto sphereCollider = std::make_shared<PythonSphereCollider>(
                    PythonEntity(entity.GetUUID()), sc.IsTrigger, sc.Radius
                );
                return sphereCollider;
            }
            else if (entity.HasComponent<CapsuleColliderComponent>())
            {
                auto& cc = entity.GetComponent<CapsuleColliderComponent>();
                auto capsuleCollider = std::make_shared<PythonCapsuleCollider>(
                    PythonEntity(entity.GetUUID()), cc.IsTrigger, cc.Radius, cc.Height
                );
                return capsuleCollider;
            }
            else if (entity.HasComponent<MeshColliderComponent>())
            {
                auto& mc = entity.GetComponent<MeshColliderComponent>();
                auto meshCollider = std::make_shared<PythonMeshCollider>(
                    PythonEntity(entity.GetUUID()), mc.IsTrigger, PythonMesh(mc.CollisionMesh)
                );
                return meshCollider;
            }
            return nullptr;
        }
    };

    class PythonMeshFactory
    {
    public:
        static PythonMesh CreatePlane(float width, float height)
        {
            return PythonMesh(ModelImporter::Import("assets/models/Plane1m.obj").Mesh);
        }
    };

} // namespace Prism::PythonScript

// PrismEngine Module Registe

PYBIND11_MODULE(PrismEngine, m)
{
    using namespace Prism::PythonScript;

    py::class_<PythonNoise>(m, "Noise")
        .def_static("PerlinNoise", &PythonNoise::PerlinNoise);

    py::class_<PythonInput>(m, "Input")
        .def_static("IsKeyPressed", &PythonInput::IsKeyPressed)
        .def_static("GetMousePosition", &PythonInput::GetMousePosition)
        .def_static("SetCursorMode", &PythonInput::SetCursorMode)
        .def_static("GetCursorMode", &PythonInput::GetCursorMode)
        .def_static("IsMouseButtonPressed", &PythonInput::IsMouseButtonPressed);

    py::class_<PythonTime>(m, "Time")
        .def_property_readonly_static("DeltaTime", &PythonTime::GetDeltaTime)
        .def_property_readonly_static("UnscaledDeltaTime", &PythonTime::GetUnscaledDeltaTime)
        .def_property_readonly_static("Time", &PythonTime::GetTime)
        .def_property_readonly_static("UnscaledTime", &PythonTime::GetUnscaledTime)
        .def_property_readonly_static("FrameCount", &PythonTime::GetFrameCount)
        .def_property_static("TimeScale", &PythonTime::GetTimeScale, &PythonTime::SetTimeScale)
        .def_property_static("FixedDeltaTime", &PythonTime::GetFixedDeltaTime, &PythonTime::SetFixedDeltaTime);

    py::class_<PythonLog>(m, "Log")
        .def_static("Trace", &PythonLog::Trace)
        .def_static("Debug", &PythonLog::Debug)
        .def_static("Info", &PythonLog::Info)
        .def_static("Warn", &PythonLog::Warn)
        .def_static("Error", &PythonLog::Error)
        .def_static("Critical", &PythonLog::Critical);

    py::class_<PythonRefCounted>(m, "Ref")
        .def(py::init<>())
        .def("SetRef", &PythonRefCounted::SetRef)
        .def("__repr__", &PythonRefCounted::__Repr__);
    py::enum_<AssetType>(m, "AssetType")
        .value("Scene", AssetType::Scene)
        .value("Mesh", AssetType::Mesh)
        .value("Texture", AssetType::Texture)
        .value("EnvMap", AssetType::EnvMap)
        .value("Audio", AssetType::Audio)
        .value("Script", AssetType::Script)
        .value("PhysicsMat", AssetType::PhysicsMat)
        .value("Shader", AssetType::Shader)
        .value("ComputeShader", AssetType::ComputeShader)
        .value("Directory", AssetType::Directory)
        .value("Other", AssetType::Other)
        .value("None", AssetType::None)
        .value("Missing", AssetType::Missing);
    py::class_<PythonAsset, PythonRefCounted>(m, "Asset")
        .def(py::init<>())
        .def("__repr__", &PythonAsset::__Repr__)
        .def_property_readonly("Type", &PythonAsset::GetType)
        .def_property_readonly("Handle", &PythonAsset::GetHandle)
        .def_property_readonly("FilePath", &PythonAsset::GetFilePath)
        .def_property_readonly("FileName", &PythonAsset::GetFileName)
        .def_property_readonly("Extension", &PythonAsset::GetExtension);
    py::class_<PythonMesh, PythonAsset>(m, "Mesh")
        .def(py::init<>())
        .def(py::init<const char*>())
        .def("__repr__", &PythonMesh::__Repr__);
    py::enum_<ImageFormat>(m, "ImageFormat")
        .value("None", ImageFormat::None)
        .value("R8", ImageFormat::R8)
        .value("RG8", ImageFormat::RG8)
        .value("RGB8", ImageFormat::RGB8)
        .value("RGBA8", ImageFormat::RGBA8)
        .value("R8_SRGB", ImageFormat::R8_SRGB)
        .value("RG8_SRGB", ImageFormat::RG8_SRGB)
        .value("RGB8_SRGB", ImageFormat::RGB8_SRGB)
        .value("RGBA8_SRGB", ImageFormat::RGBA8_SRGB)
        .value("R8_SNORM", ImageFormat::R8_SNORM)
        .value("RG8_SNORM", ImageFormat::RG8_SNORM)
        .value("RGB8_SNORM", ImageFormat::RGB8_SNORM)
        .value("RGBA8_SNORM", ImageFormat::RGBA8_SNORM)
        .value("R16F", ImageFormat::R16F)
        .value("RG16F", ImageFormat::RG16F)
        .value("RGB16F", ImageFormat::RGB16F)
        .value("RGBA16F", ImageFormat::RGBA16F)
        .value("R32F", ImageFormat::R32F)
        .value("RG32F", ImageFormat::RG32F)
        .value("RGB32F", ImageFormat::RGB32F)
        .value("RGBA32F", ImageFormat::RGBA32F)
        .value("R16_UINT", ImageFormat::R16_UINT)
        .value("RG16_UINT", ImageFormat::RG16_UINT)
        .value("RGBA16_UINT", ImageFormat::RGBA16_UINT)
        .value("R32_UINT", ImageFormat::R32_UINT)
        .value("RG32_UINT", ImageFormat::RG32_UINT)
        .value("RGBA32_UINT", ImageFormat::RGBA32_UINT)
        .value("R16_SINT", ImageFormat::R16_SINT)
        .value("RG16_SINT", ImageFormat::RG16_SINT)
        .value("RGBA16_SINT", ImageFormat::RGBA16_SINT)
        .value("R32_SINT", ImageFormat::R32_SINT)
        .value("RG32_SINT", ImageFormat::RG32_SINT)
        .value("RGBA32_SINT", ImageFormat::RGBA32_SINT)
        .value("RGB565", ImageFormat::RGB565)
        .value("RGBA4", ImageFormat::RGBA4)
        .value("RGB5A1", ImageFormat::RGB5A1)
        .value("RGB10A2", ImageFormat::RGB10A2)
        .value("RG11B10F", ImageFormat::RG11B10F)
        .value("RGB9E5", ImageFormat::RGB9E5)
        .value("DEPTH16", ImageFormat::DEPTH16)
        .value("DEPTH24STENCIL8", ImageFormat::DEPTH24STENCIL8)
        .value("DEPTH32F", ImageFormat::DEPTH32F)
        .value("DEPTH32FSTENCIL8", ImageFormat::DEPTH32FSTENCIL8)
        .value("BC1", ImageFormat::BC1)
        .value("BC1_SRGB", ImageFormat::BC1_SRGB)
        .value("BC2", ImageFormat::BC2)
        .value("BC2_SRGB", ImageFormat::BC2_SRGB)
        .value("BC3", ImageFormat::BC3)
        .value("BC3_SRGB", ImageFormat::BC3_SRGB)
        .value("BC4", ImageFormat::BC4)
        .value("BC5", ImageFormat::BC5)
        .value("BC6H_UF16", ImageFormat::BC6H_UF16)
        .value("BC6H_SF16", ImageFormat::BC6H_SF16)
        .value("BC7", ImageFormat::BC7)
        .value("BC7_SRGB", ImageFormat::BC7_SRGB)
        .value("ETC2_RGB8", ImageFormat::ETC2_RGB8)
        .value("ETC2_RGB8_SRGB", ImageFormat::ETC2_RGB8_SRGB)
        .value("ETC2_RGBA8", ImageFormat::ETC2_RGBA8)
        .value("ETC2_RGBA8_SRGB", ImageFormat::ETC2_RGBA8_SRGB)
        .value("ASTC_4x4", ImageFormat::ASTC_4x4)
        .value("ASTC_5x5", ImageFormat::ASTC_5x5)
        .value("ASTC_6x6", ImageFormat::ASTC_6x6)
        .value("ASTC_8x8", ImageFormat::ASTC_8x8)
        .value("Depth", ImageFormat::Depth);
    py::enum_<ImageUsage>(m, "ImageUsage")
        .value("None", ImageUsage::None)
        .value("Texture", ImageUsage::Texture)
        .value("Attachment", ImageUsage::Attachment)
        .value("Storage", ImageUsage::Storage);
    py::class_<ImageSpecification>(m, "ImageSpecification")
        .def(py::init<>())
        .def_readwrite("Format", &ImageSpecification::Format)
        .def_readwrite("Usage", &ImageSpecification::Usage)
        .def_readwrite("Width", &ImageSpecification::Width)
        .def_readwrite("Height", &ImageSpecification::Height)
        .def_readwrite("Samples", &ImageSpecification::Samples);
    py::class_<PythonImage, PythonRefCounted>(m, "Image")
        .def(py::init<>())
        .def("__repr__", &PythonImage::__Repr__)
        .def_property_readonly("Width", &PythonImage::GetWidth)
        .def_property_readonly("Height", &PythonImage::GetHeight)
        .def_property_readonly("Samples", &PythonImage::GetSamples)
        .def_property_readonly("Format", &PythonImage::GetFormat)
        .def_property_readonly("Usage", &PythonImage::GetUsage)
        .def("GetWidth", &PythonImage::GetWidth)
        .def("GetHeight", &PythonImage::GetHeight)
        .def("GetSamples", &PythonImage::GetSamples)
        .def("GetFormat", &PythonImage::GetFormat)
        .def("GetUsage", &PythonImage::GetUsage);
    py::class_<PythonImage2D, PythonImage>(m, "Image2D")
        .def(py::init<>())
        .def_static("Create", &PythonImage2D::Create,
            py::arg("specification"), py::arg("data") = py::none())
        .def("__repr__", &PythonImage2D::__Repr__);
    py::class_<PythonImageCube, PythonImage>(m, "ImageCube")
        .def(py::init<>())
        .def_static("Create", &PythonImageCube::Create,
            py::arg("specification"), py::arg("data") = py::none())
        .def("__repr__", &PythonImageCube::__Repr__)
        .def("GenerateMipMap", &PythonImageCube::GenerateMipMap)
        .def("CopyTo", &PythonImageCube::CopyTo);
    py::class_<PythonTexture, PythonAsset>(m, "Texture")
        .def(py::init<>())
        .def_property_readonly("Width", &PythonTexture::GetWidth)
        .def_property_readonly("Height", &PythonTexture::GetHeight)
        .def_property_readonly("Format", &PythonTexture::GetFormat)
        .def("GetWidth", &PythonTexture::GetWidth)
        .def("GetHeight", &PythonTexture::GetHeight)
        .def("GetFormat", &PythonTexture::GetFormat);
    py::class_<PythonTexture2D, PythonTexture>(m, "Texture2D")
        .def(py::init<>())
        .def_static("Create", &PythonTexture2D::Create, py::arg("width"), py::arg("height"))
        .def("__repr__", &PythonTexture2D::__Repr__)
        .def("SetData", &PythonTexture2D::SetData)
        .def("GetImage", &PythonTexture2D::GetImage);
    py::class_<PythonTextureCube, PythonTexture>(m, "TextureCube")
        .def(py::init<>())
        .def_static("Create", &PythonTextureCube::Create,
            py::arg("format"), py::arg("width"), py::arg("height"), py::arg("data") = py::none())
        .def("__repr__", &PythonTextureCube::__Repr__)
        .def("GetImage", &PythonTextureCube::GetImage);
    py::enum_<PrismShaderCompiler::PropertyType>(m, "UniformType")
        .value("None", PrismShaderCompiler::PropertyType::None)
        .value("Bool", PrismShaderCompiler::PropertyType::Bool)
        .value("Color", PrismShaderCompiler::PropertyType::Color)
        .value("Color3", PrismShaderCompiler::PropertyType::Color3)
        .value("Float", PrismShaderCompiler::PropertyType::Float)
        .value("Int", PrismShaderCompiler::PropertyType::Int)
        .value("Vector2", PrismShaderCompiler::PropertyType::Vector2)
        .value("Vector3", PrismShaderCompiler::PropertyType::Vector3)
        .value("Vector4", PrismShaderCompiler::PropertyType::Vector4)
        .value("Range", PrismShaderCompiler::PropertyType::Range)
        .value("Matrix3", PrismShaderCompiler::PropertyType::Matrix3)
        .value("Matrix4", PrismShaderCompiler::PropertyType::Matrix4)
        .value("Texture2D", PrismShaderCompiler::PropertyType::Texture2D)
        .value("Texture2DMS", PrismShaderCompiler::PropertyType::Texture2DMS)
        .value("TextureCube", PrismShaderCompiler::PropertyType::TextureCube)
        .value("Enum", PrismShaderCompiler::PropertyType::Enum);
    py::class_<PythonPrismShader, PythonAsset>(m, "PrismShader")
        .def(py::init<>())
        .def_static("GetShader", &PythonPrismShader::GetShader)
        .def("__repr__", &PythonPrismShader::__Repr__)
        .def_property_readonly("Name", &PythonPrismShader::GetName)
        .def("GetName", &PythonPrismShader::GetName)
        .def("GetUniformCount", &PythonPrismShader::GetUniformCount)
        .def("GetUniformType", &PythonPrismShader::GetUniformType)
        .def("GetUniformName", &PythonPrismShader::GetUniformName)
        .def("GetUniformDisplayName", &PythonPrismShader::GetUniformDisplayName)
        .def("GetUniformDefaultValue", &PythonPrismShader::GetUniformDefaultValue);
    py::class_<PythonMaterial, PythonRefCounted>(m, "Material")
        .def(py::init<>())
        .def(py::init<const PythonPrismShader&>())
        .def("__repr__", &PythonMaterial::__Repr__)
        .def("SetFloat", &PythonMaterial::SetFloat)
        .def("SetInt", &PythonMaterial::SetInt)
        .def("SetBool", &PythonMaterial::SetBool)
        .def("SetVector2", &PythonMaterial::SetVector2)
        .def("SetVector3", &PythonMaterial::SetVector3)
        .def("SetVector4", &PythonMaterial::SetVector4)
        .def("SetColor3", &PythonMaterial::SetColor3)
        .def("SetColor", &PythonMaterial::SetColor)
        .def("SetMatrix4", &PythonMaterial::SetMatrix4)
        .def("SetTexture2D", &PythonMaterial::SetTexture2D)
        .def("SetKeyword", &PythonMaterial::SetKeyword)
        .def("IsKeywordEnabled", &PythonMaterial::IsKeywordEnabled);
    py::class_<PythonUniformBuffer, PythonRefCounted>(m, "UniformBuffer")
        .def(py::init<>())
        .def_static("Create", &PythonUniformBuffer::Create)
        .def("__repr__", &PythonUniformBuffer::__Repr__)
        .def("SetData", &PythonUniformBuffer::SetData, py::arg("data"), py::arg("offset") = 0);
    py::class_<PythonShaderStorageBuffer, PythonRefCounted>(m, "ShaderStorageBuffer")
        .def(py::init<>())
        .def_static("Create", &PythonShaderStorageBuffer::Create, py::arg("size"), py::arg("usage") = (uint32_t)BufferUsage::Dynamic)
        .def("__repr__", &PythonShaderStorageBuffer::__Repr__)
        .def("SetData", &PythonShaderStorageBuffer::SetData, py::arg("data"), py::arg("offset") = 0)
        .def("GetData", &PythonShaderStorageBuffer::GetData, py::arg("data"), py::arg("offset") = 0, py::arg("sync") = false)
        .def("GetSize", &PythonShaderStorageBuffer::GetSize);
    py::class_<PythonComputeShader, PythonAsset>(m, "ComputeShader")
        .def(py::init<>())
        .def_static("Create", &PythonComputeShader::Create, py::arg("filePath"))
        .def("__repr__", &PythonComputeShader::__Repr__)
        .def("GetName", &PythonComputeShader::GetName)
        .def("GetKernelCount", &PythonComputeShader::GetKernelCount)
        .def("FindKernel", &PythonComputeShader::FindKernel, py::arg("name"))
        .def("HasKernel", &PythonComputeShader::HasKernel, py::arg("name"))
        .def("GetKernelThreadGroupSizes", &PythonComputeShader::GetKernelThreadGroupSizes, py::arg("kernel"))
        .def("SetUniformBuffer", &PythonComputeShader::SetUniformBuffer, py::arg("kernel"), py::arg("name"), py::arg("buffer"))
        .def("SetBuffer", &PythonComputeShader::SetBuffer, py::arg("kernel"), py::arg("name"), py::arg("buffer"))
        .def("SetTexture2D", &PythonComputeShader::SetTexture2D, py::arg("kernel"), py::arg("name"), py::arg("image"))
        .def("SetTextureCube", &PythonComputeShader::SetTextureCube, py::arg("kernel"), py::arg("name"), py::arg("image"))
        .def("SetImage2D", &PythonComputeShader::SetImage2D, py::arg("kernel"), py::arg("name"), py::arg("image"), py::arg("level") = 0)
        .def("SetImageCube", &PythonComputeShader::SetImageCube, py::arg("kernel"), py::arg("name"), py::arg("image"), py::arg("level") = 0)
        .def("Dispatch", &PythonComputeShader::Dispatch, py::arg("kernel"), py::arg("groupsX"), py::arg("groupsY"), py::arg("groupsZ"));
    py::class_<PythonMeshFactory>(m, "MeshFactory")
        .def_static("CreatePlane", &PythonMeshFactory::CreatePlane);

    py::class_<PythonEntity>(m, "Entity")
        .def(py::init<uint64_t>(), py::arg("id") = 0)
        .def("__repr__", &PythonEntity::__Repr__)
        .def("__eq__", &PythonEntity::__Eq__)
        .def("__ne__", &PythonEntity::__Ne__)
        .def("__hash__", &PythonEntity::__Hash__)
        .def_property("ID", &PythonEntity::GetID, &PythonEntity::SetID)
        .def_property_readonly("_id", &PythonEntity::GetID)
        .def("GetComponent", &PythonEntity::GetComponent)
        .def("CreateComponent", &PythonEntity::CreateComponent)
        .def("HasComponent", &PythonEntity::HasComponent)
        .def_property_readonly("Transform", &PythonEntity::GetTransform)
        .def_static("FindEntityByTag", &PythonEntity::FindEntityByTag)
        .def_static("FindEntityByID", &PythonEntity::FindEntityByID);

    py::class_<PythonComponent>(m, "Component")
        .def(py::init<>())
        .def_property("Entity", &PythonComponent::GetEntity, &PythonComponent::SetEntity);
    py::class_<PythonTagComponent, PythonComponent>(m, "TagComponent")
        .def(py::init<>())
        .def("__repr__", &PythonTagComponent::__Repr__)
        .def_property("Tag", &PythonTagComponent::GetTag, &PythonTagComponent::SetTag);
    py::class_<PythonCameraComponent, PythonComponent>(m, "CameraComponent")
        .def(py::init<>())
        .def("__repr__", &PythonCameraComponent::__Repr__);
    py::class_<::Prism::PythonScript::PythonScriptComponent, PythonComponent>(m, "ScriptComponent")
        .def(py::init<>());
    py::class_<PythonSpriteRendererComponent, PythonComponent>(m, "SpriteRendererComponent")
        .def(py::init<>());
    py::class_<PythonRigidBody2DComponent, PythonComponent>(m, "RigidBody2DComponent")
        .def(py::init<>())
        .def("__repr__", &PythonRigidBody2DComponent::__Repr__)
        .def("ApplyLinearImpulse", &PythonRigidBody2DComponent::ApplyLinearImpulse)
        .def("GetLinearVelocity", &PythonRigidBody2DComponent::GetLinearVelocity)
        .def("SetLinearVelocity", &PythonRigidBody2DComponent::SetLinearVelocity)
        .def_property("LinearVelocity", &PythonRigidBody2DComponent::GetLinearVelocity, &PythonRigidBody2DComponent::SetLinearVelocity);
    py::class_<PythonBoxCollider2DComponent, PythonComponent>(m, "BoxCollider2DComponent")
        .def(py::init<>());
    py::class_<PythonCircleCollider2DComponent, PythonComponent>(m, "CircleCollider2DComponent")
        .def(py::init<>());

    py::class_<PythonRigidBodyComponent, PythonComponent>(m, "RigidBodyComponent")
        .def(py::init<>())
        .def("__repr__", &PythonRigidBodyComponent::__Repr__)
        .def("AddForce", &PythonRigidBodyComponent::AddForce)
        .def("AddTorque", &PythonRigidBodyComponent::AddTorque)
        .def_property("LinearVelocity", &PythonRigidBodyComponent::GetLinearVelocity, &PythonRigidBodyComponent::SetLinearVelocity)
        .def("GetLinearVelocity", &PythonRigidBodyComponent::GetLinearVelocity)
        .def("SetLinearVelocity", &PythonRigidBodyComponent::SetLinearVelocity)
        .def("Rotate", &PythonRigidBodyComponent::Rotate)
        .def_property("Mass", &PythonRigidBodyComponent::GetMass, &PythonRigidBodyComponent::SetMass)
        .def("GetMass", &PythonRigidBodyComponent::GetMass)
        .def("SetMass", &PythonRigidBodyComponent::SetMass)
        .def_property("AngularVelocity", &PythonRigidBodyComponent::GetAngularVelocity, &PythonRigidBodyComponent::SetAngularVelocity)
        .def("GetAngularVelocity", &PythonRigidBodyComponent::GetAngularVelocity)
        .def("SetAngularVelocity", &PythonRigidBodyComponent::SetAngularVelocity)
        .def_property_readonly("Layer", &PythonRigidBodyComponent::GetLayer)
        .def_property_readonly("BodyType", &PythonRigidBodyComponent::GetBodyType);

    py::class_<PythonBoxColliderComponent, PythonComponent>(m, "BoxColliderComponent")
        .def(py::init<>());
    py::class_<PythonSphereColliderComponent, PythonComponent>(m, "SphereColliderComponent")
        .def(py::init<>());
    py::class_<PythonCapsuleColliderComponent, PythonComponent>(m, "CapsuleColliderComponent")
        .def(py::init<>());

    py::class_<PythonTransformComponent, PythonComponent>(m, "TransformComponent")
        .def(py::init<>())
        .def_property("Position", &PythonTransformComponent::GetPosition, &PythonTransformComponent::SetPosition)
        .def_property("Rotation", &PythonTransformComponent::GetRotation, &PythonTransformComponent::SetRotation)
        .def_property("Scale", &PythonTransformComponent::GetScale, &PythonTransformComponent::SetScale)
        .def_property("LocalPosition", &PythonTransformComponent::GetLocalPosition, &PythonTransformComponent::SetLocalPosition)
        .def_property("LocalRotation", &PythonTransformComponent::GetLocalRotation, &PythonTransformComponent::SetLocalRotation)
        .def_property("LocalScale", &PythonTransformComponent::GetLocalScale, &PythonTransformComponent::SetLocalScale)
        .def_property_readonly("Forward", &PythonTransformComponent::GetForward)
        .def_property_readonly("Right", &PythonTransformComponent::GetRight)
        .def_property_readonly("Up", &PythonTransformComponent::GetUp)
        .def_property("Transform", &PythonTransformComponent::GetTransform, &PythonTransformComponent::SetTransform);
    py::class_<PythonMeshRendererComponent, PythonComponent>(m, "MeshRendererComponent")
        .def(py::init<>())
        .def_property("Mesh", &PythonMeshRendererComponent::GetMesh, &PythonMeshRendererComponent::SetMesh)
        .def_property("Material",
            [](const PythonMeshRendererComponent& self) { return self.GetMaterial(0); },
            [](PythonMeshRendererComponent& self, const PythonMaterial& mat) { self.SetMaterial(mat, 0); })
        .def_property("Materials", &PythonMeshRendererComponent::GetMaterials, &PythonMeshRendererComponent::SetMaterials)
        .def_property_readonly("MaterialCount", &PythonMeshRendererComponent::GetMaterialCount)
        .def("GetMaterial", &PythonMeshRendererComponent::GetMaterial)
        .def("SetMaterial", &PythonMeshRendererComponent::SetMaterial);
    py::class_<PythonBehaviour, PythonComponent>(m, "Behaviour")
        .def(py::init<>())
        .def_property("ID", &PythonBehaviour::GetID, &PythonBehaviour::SetID)
        .def_property("Enabled", &PythonBehaviour::GetEnabled, &PythonBehaviour::SetEnabled)
        .def_property_readonly("Transform", &PythonBehaviour::GetTransform)
        .def("GetComponent", &PythonBehaviour::GetComponent)
        .def("HasComponent", &PythonBehaviour::HasComponent)
        .def("CreateComponent", &PythonBehaviour::CreateComponent);

    py::class_<PythonCollider>(m, "Collider")
        .def("__repr__", &PythonCollider::__Repr__)
        .def("__str__", &PythonCollider::__Str__)
        .def_property_readonly("Entity", &PythonCollider::GetEntity)
        .def_property_readonly("RigidBody", &PythonCollider::GetRigidBody)
        .def_property_readonly("IsTrigger", &PythonCollider::IsTrigger);
    py::class_<PythonBoxCollider, PythonCollider>(m, "BoxCollider")
        .def_property_readonly("Size", &PythonBoxCollider::GetSize)
        .def_property_readonly("Offset", &PythonBoxCollider::GetOffset);
    py::class_<PythonSphereCollider, PythonCollider>(m, "SphereCollider")
        .def_property_readonly("Radius", &PythonSphereCollider::GetRadius);
    py::class_<PythonCapsuleCollider, PythonCollider>(m, "CapsuleCollider")
        .def_property_readonly("Radius", &PythonCapsuleCollider::GetRadius)
        .def_property_readonly("Height", &PythonCapsuleCollider::GetHeight);
    py::class_<PythonMeshCollider, PythonCollider>(m, "MeshCollider")
        .def_property_readonly("Mesh", &PythonMeshCollider::GetMesh);

    py::class_<PythonPhysics>(m, "Physics")
        .def_property_static("Gravity", &PythonPhysics::GetGravity, &PythonPhysics::SetGravity)
        .def_static("Raycast", &PythonPhysics::Raycast,
            py::arg("origin"), py::arg("direction"), py::arg("maxDistance") = 100.0f, py::arg("raycastHit") = py::none())
        .def_static("OverlapBox", &PythonPhysics::OverlapBox)
        .def_static("OverlapSphere", &PythonPhysics::OverlapSphere)
        .def_static("OverlapCapsule", &PythonPhysics::OverlapCapsule);


#pragma region EnumClass
    py::class_<RaycastHit>(m, "RaycastHit")
        .def(py::init<>())
        .def_readwrite("EntityID", &RaycastHit::EntityID)
        .def_readwrite("Position", &RaycastHit::Position)
        .def_readwrite("Normal", &RaycastHit::Normal)
        .def_readwrite("Distance", &RaycastHit::Distance);

    py::class_<PythonTransform>(m, "ScriptTransform")
        .def(py::init<>())
        .def_readwrite("Position", &PythonTransform::Position)
        .def_readwrite("Rotation", &PythonTransform::Rotation)
        .def_readwrite("Scale", &PythonTransform::Scale)
        .def_readwrite("Up", &PythonTransform::Up)
        .def_readwrite("Right", &PythonTransform::Right)
        .def_readwrite("Forward", &PythonTransform::Forward);

    py::enum_<ForceMode>(m, "ForceMode")
        .value("Force", ForceMode::Force)
        .value("Impulse", ForceMode::Impulse)
        .value("VelocityChange", ForceMode::VelocityChange)
        .value("Acceleration", ForceMode::Acceleration);
    py::enum_<CursorMode>(m, "CursorMode")
        .value("Normal", CursorMode::Normal)
        .value("Hidden", CursorMode::Hidden)
        .value("Locked", CursorMode::Locked);
    py::enum_<MouseButton>(m, "MouseButton")
        .value("Left", MouseButton::Left)
        .value("Right", MouseButton::Right)
        .value("Middle", MouseButton::Middle)
        .value("Button0", MouseButton::Button0)
        .value("Button1", MouseButton::Button1)
        .value("Button2", MouseButton::Button2)
        .value("Button3", MouseButton::Button3)
        .value("Button4", MouseButton::Button4)
        .value("Button5", MouseButton::Button5);
    py::enum_<KeyCode>(m, "KeyCode")
        .value("Space", KeyCode::Space)
        .value("Apostrophe", KeyCode::Apostrophe)
        .value("Comma", KeyCode::Comma)
        .value("Minus", KeyCode::Minus)
        .value("Period", KeyCode::Period)
        .value("Slash", KeyCode::Slash)
        .value("D0", KeyCode::D0)
        .value("D1", KeyCode::D1)
        .value("D2", KeyCode::D2)
        .value("D3", KeyCode::D3)
        .value("D4", KeyCode::D4)
        .value("D5", KeyCode::D5)
        .value("D6", KeyCode::D6)
        .value("D7", KeyCode::D7)
        .value("D8", KeyCode::D8)
        .value("D9", KeyCode::D9)
        .value("Semicolon", KeyCode::Semicolon)
        .value("Equal", KeyCode::Equal)
        .value("A", KeyCode::A)
        .value("B", KeyCode::B)
        .value("C", KeyCode::C)
        .value("D", KeyCode::D)
        .value("E", KeyCode::E)
        .value("F", KeyCode::F)
        .value("G", KeyCode::G)
        .value("H", KeyCode::H)
        .value("I", KeyCode::I)
        .value("J", KeyCode::J)
        .value("K", KeyCode::K)
        .value("L", KeyCode::L)
        .value("M", KeyCode::M)
        .value("N", KeyCode::N)
        .value("O", KeyCode::O)
        .value("P", KeyCode::P)
        .value("Q", KeyCode::Q)
        .value("R", KeyCode::R)
        .value("S", KeyCode::S)
        .value("T", KeyCode::T)
        .value("U", KeyCode::U)
        .value("V", KeyCode::V)
        .value("W", KeyCode::W)
        .value("X", KeyCode::X)
        .value("Y", KeyCode::Y)
        .value("Z", KeyCode::Z)
        .value("LeftBracket", KeyCode::LeftBracket)
        .value("Backslash", KeyCode::Backslash)
        .value("RightBracket", KeyCode::RightBracket)
        .value("GraveAccent", KeyCode::GraveAccent)
        .value("World1", KeyCode::World1)
        .value("World2", KeyCode::World2)
        .value("Escape", KeyCode::Escape)
        .value("Enter", KeyCode::Enter)
        .value("Tab", KeyCode::Tab)
        .value("Backspace", KeyCode::Backspace)
        .value("Insert", KeyCode::Insert)
        .value("Delete", KeyCode::Delete)
        .value("Right", KeyCode::Right)
        .value("Left", KeyCode::Left)
        .value("Down", KeyCode::Down)
        .value("Up", KeyCode::Up)
        .value("PageUp", KeyCode::PageUp)
        .value("PageDown", KeyCode::PageDown)
        .value("Home", KeyCode::Home)
        .value("End", KeyCode::End)
        .value("CapsLock", KeyCode::CapsLock)
        .value("ScrollLock", KeyCode::ScrollLock)
        .value("NumLock", KeyCode::NumLock)
        .value("PrintScreen", KeyCode::PrintScreen)
        .value("Pause", KeyCode::Pause)
        .value("F1", KeyCode::F1)
        .value("F2", KeyCode::F2)
        .value("F3", KeyCode::F3)
        .value("F4", KeyCode::F4)
        .value("F5", KeyCode::F5)
        .value("F6", KeyCode::F6)
        .value("F7", KeyCode::F7)
        .value("F8", KeyCode::F8)
        .value("F9", KeyCode::F9)
        .value("F10", KeyCode::F10)
        .value("F11", KeyCode::F11)
        .value("F12", KeyCode::F12)
        .value("F13", KeyCode::F13)
        .value("F14", KeyCode::F14)
        .value("F15", KeyCode::F15)
        .value("F16", KeyCode::F16)
        .value("F17", KeyCode::F17)
        .value("F18", KeyCode::F18)
        .value("F19", KeyCode::F19)
        .value("F20", KeyCode::F20)
        .value("F21", KeyCode::F21)
        .value("F22", KeyCode::F22)
        .value("F23", KeyCode::F23)
        .value("F24", KeyCode::F24)
        .value("F25", KeyCode::F25)
        .value("KP0", KeyCode::KP0)
        .value("KP1", KeyCode::KP1)
        .value("KP2", KeyCode::KP2)
        .value("KP3", KeyCode::KP3)
        .value("KP4", KeyCode::KP4)
        .value("KP5", KeyCode::KP5)
        .value("KP6", KeyCode::KP6)
        .value("KP7", KeyCode::KP7)
        .value("KP8", KeyCode::KP8)
        .value("KP9", KeyCode::KP9)
        .value("KPDecimal", KeyCode::KPDecimal)
        .value("KPDivide", KeyCode::KPDivide)
        .value("KPMultiply", KeyCode::KPMultiply)
        .value("KPSubtract", KeyCode::KPSubtract)
        .value("KPAdd", KeyCode::KPAdd)
        .value("KPEnter", KeyCode::KPEnter)
        .value("KPEqual", KeyCode::KPEqual)
        .value("LeftShift", KeyCode::LeftShift)
        .value("LeftControl", KeyCode::LeftControl)
        .value("LeftAlt", KeyCode::LeftAlt)
        .value("LeftSuper", KeyCode::LeftSuper)
        .value("RightShift", KeyCode::RightShift)
        .value("RightControl", KeyCode::RightControl)
        .value("RightAlt", KeyCode::RightAlt)
        .value("RightSuper", KeyCode::RightSuper)
        .value("Menu", KeyCode::Menu);
        
#pragma endregion

}


// Python Type Registration

namespace Prism
{
    static std::unordered_map<UUID, pybind11::object> s_PythonTypeCache;

    static constexpr uint64_t GenerateTypeID(std::string_view name)
    {
        return Hash::GenerateFNVHash64(name.data());
    }

    void RegisterAllPythonTypes()
    {
        s_PythonTypeCache.clear();
        try
        {
            py::module_ builtins = py::module_::import("builtins");
            py::module_ math = py::module_::import("Prism.Math");
            py::module_ engine = py::module_::import("PrismEngine");
            using namespace Prism::PythonScript;
            s_PythonTypeCache[PYTHON_TYPE_NONE] = py::type::of(py::none());
            s_PythonTypeCache[PYTHON_TYPE_FLOAT] = builtins.attr("float");
            s_PythonTypeCache[PYTHON_TYPE_DOUBLE] = builtins.attr("float");
            s_PythonTypeCache[PYTHON_TYPE_BOOL] = builtins.attr("bool");
            s_PythonTypeCache[PYTHON_TYPE_INT8] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_INT16] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_INT32] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_INT64] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT8] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT16] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT32] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_UINT64] = builtins.attr("int");
            s_PythonTypeCache[PYTHON_TYPE_VECTOR2] = math.attr("Vector2");
            s_PythonTypeCache[PYTHON_TYPE_VECTOR3] = math.attr("Vector3");
            s_PythonTypeCache[PYTHON_TYPE_VECTOR4] = math.attr("Vector4");
            s_PythonTypeCache[PYTHON_TYPE_OBJECT] = builtins.attr("object");
            s_PythonTypeCache[PYTHON_TYPE_MESHREF] = py::type::of<PythonMesh>();
            s_PythonTypeCache[PYTHON_TYPE_MATERIALREF] = py::type::of<PythonMaterial>();
            s_PythonTypeCache[PYTHON_TYPE_TEXTURE2DREF] = py::type::of<PythonTexture2D>();
            s_PythonTypeCache[PYTHON_TYPE_PRISMSHADERREF] = py::type::of<PythonPrismShader>();
            s_PythonTypeCache[PYTHON_TYPE_COMPUTESHADERREF] = py::type::of<PythonComputeShader>();
            s_PythonTypeCache[PYTHON_TYPE_ASSET] = py::type::of<PythonAsset>();
            s_PythonTypeCache[PYTHON_TYPE_REF] = py::type::of<PythonRefCounted>();

            for (const auto& [id, type] : s_PythonTypeCache)
                PR_CORE_INFO("[Python Meta] 注册类型: {} -> {}", id, (std::string)pybind11::str(type));
            
        }
        catch (py::error_already_set& e)
        {
            PR_CORE_WARN("[Python Meta] 注册类型异常: {}", e.what());
            PyErr_Clear();
        }
    }

    void ClearAllPythonTypes()
    {
        s_PythonTypeCache.clear();
    }

    pybind11::object* GetPythonType(const UUID id)
    {
        return s_PythonTypeCache.count(id) ? &s_PythonTypeCache.at(id) : nullptr;
    }

} // namespace Prism::PythonScript
