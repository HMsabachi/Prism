#pragma once

// Renderer2D 暂时禁用（基本未用，回头跟进 Hazel 设计：DrawCircle/DrawLine/Stats）
#if 0
#include <glm/glm.hpp>

namespace Prism
{
    class Texture2D;
}

namespace Prism
{
    class PRISM_API Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const glm::mat4& viewProj, bool depthTest = true);
        static void EndScene();
        static void Flush();

        // Primitives
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
        static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));

        static void DrawRotatedRect(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedRect(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);

        static void DrawCircle(const glm::vec2& position, float radius, const glm::vec4& color, float thickness = 0.05f);
        static void DrawCircle(const glm::vec3& position, float radius, const glm::vec4& color, float thickness = 0.05f);
        // Stats
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;
            uint32_t LineCount = 0;
            uint32_t CircleCount = 0;

            uint32_t GetTotalVertexCount() { return QuadCount * 4 + LineCount * 2 + CircleCount * 4; }
            uint32_t GetTotalIndexCount() { return QuadCount * 6 + LineCount * 2 + CircleCount * 6; }
        };
        static void ResetStats();
        static Statistics GetStats();
    private:
        static void FlushAndReset();
        static void FlushAndResetLines();
    };

}
#endif
