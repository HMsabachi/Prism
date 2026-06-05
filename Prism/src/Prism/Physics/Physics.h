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

    struct RaycastHit
    {
        uint64_t EntityID;
        glm::vec3 Position;
        glm::vec3 Normal;
        float Distance;
    };

    struct OverlapHitData
    {
        uint64_t EntityID;
        uint32_t ColliderType;  // 0=Box, 1=Sphere, 2=Capsule
        uint32_t IsTrigger;
        float ShapeData[6];
        // Box:    [0-2]=Size, [3-5]=Offset
        // Sphere: [0]=Radius
        // Capsule:[0]=Radius, [1]=Height
    };

    class PRISM_API Physics
    {
    public:
        static void Init();
        static void Shutdown();

        static void CreateScene(const SceneParams& params);
        static void CreateActor(Entity e, int entityCount);

        static void Step(float dt);

        static void DestroyScene();

        static void* GetPhysicsScene();

        static void ConnectVisualDebugger();
        static void DisconnectVisualDebugger();

        using CollisionCallback = std::function<void(Entity)>;
        using TriggerCallback = std::function<void(Entity)>;
        static void SetCollisionCallbacks(CollisionCallback begin, CollisionCallback end);
        static void SetTriggerCallbacks(TriggerCallback begin, TriggerCallback end);
    };

}
