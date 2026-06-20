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

    enum class BroadphaseType
    {
        SweepAndPrune,
        MultiBoxPrune,
        AutomaticBoxPrune
    };

    struct PhysicsSettings
    {
        float FixedTimestep = 0.02F;
        glm::vec3 Gravity = { 0.0F, -9.81F, 0.0F };
        BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
        glm::vec3 WorldBoundsMin = glm::vec3(0.0F);
        glm::vec3 WorldBoundsMax = glm::vec3(1.0F);
        uint32_t WorldBoundsSubdivisions = 2;
        uint32_t SolverIterations = 6;
        uint32_t SolverVelocityIterations = 1;
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
        uint32_t ColliderType;  // 0=Box, 1=Sphere, 2=Capsule, 3=Mesh
        uint32_t IsTrigger;
        float ShapeData[6];
        // Box:    [0-2]=Size, [3-5]=Offset
        // Sphere: [0]=Radius
        // Capsule:[0]=Radius, [1]=Height
        // Mesh:   MeshHandle
        void* MeshHandle;
    };

    class PRISM_API Physics
    {
    public:
        static void Init();
        static void Shutdown();

        static void ExpandEntityBuffer(uint32_t entityCount);
        static void CreateScene();
        static void CreateActor(Entity e);

        static PhysicsSettings& GetSettings();

        static void SetGravity(float gravity);
        static float GetGravity();

        static void Step(float dt);

        static void DestroyScene();

        static void* GetPhysicsScene();

        using CollisionCallback = std::function<void(Entity)>;
        using TriggerCallback = std::function<void(Entity)>;
        static void SetCollisionCallbacks(CollisionCallback begin, CollisionCallback end);
        static void SetTriggerCallbacks(TriggerCallback begin, TriggerCallback end);
    };

}
