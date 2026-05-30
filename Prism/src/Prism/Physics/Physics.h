#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Scene/Entity.h"

namespace Prism {

    enum class ForceMode : uint16_t
    {
        Force = 0,
        Impulse,
        VelocityChange,
        Acceleration
    };

    enum class FilterGroup : uint32_t
    {
        Static   = BIT(0),
        Dynamic  = BIT(1),
        Kinematic = BIT(2),
        All      = Static | Dynamic | Kinematic
    };

    struct SceneParams
    {
        glm::vec3 Gravity = { 0.0F, -9.81F, 0.0F };
    };

    class PRISM_API Physics
    {
    public:
        static void Init();
        static void Shutdown();

        static void CreateScene(const SceneParams& params);
        static void CreateActor(Entity e, int entityCount);

        static void Simulate();

        static void DestroyScene();

        static void ConnectVisualDebugger();
        static void DisconnectVisualDebugger();

        using CollisionCallback = std::function<void(Entity)>;
        using TriggerCallback = std::function<void(Entity)>;
        static void SetCollisionCallbacks(CollisionCallback begin, CollisionCallback end);
        static void SetTriggerCallbacks(TriggerCallback begin, TriggerCallback end);
    };

}
