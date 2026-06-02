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

    struct PhysicsLayer
    {
        uint32_t LayerID;
        std::string Name;
        uint32_t BitValue;
        int32_t CollidesWith = 0;
    };

    class PRISM_API PhysicsLayerManager
    {
    public:
        static uint32_t AddLayer(const std::string& name, bool setCollisions = true);
        static void RemoveLayer(uint32_t layerId);

        static void SetLayerCollision(uint32_t layerId, uint32_t otherLayer, bool collides);
        static const std::vector<PhysicsLayer>& GetLayerCollisions(uint32_t layerId);

        static const std::vector<PhysicsLayer>& GetLayers() { return s_Layers; }

        static PhysicsLayer& GetLayer(uint32_t layerId);
        static PhysicsLayer& GetLayer(const std::string& layerName);
        static uint32_t GetLayerCount() { return s_Layers.size(); }

        static bool ShouldCollide(uint32_t layer1, uint32_t layer2);
        static bool IsLayerValid(uint32_t layerId);

        static void ClearLayers();

    private:
        static uint32_t GetNextLayerID();

    private:
        static std::vector<PhysicsLayer> s_Layers;
        static std::unordered_map<uint32_t, std::vector<PhysicsLayer>> s_LayerCollisions;
        static PhysicsLayer s_NullLayer;
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

        static void ConnectVisualDebugger();
        static void DisconnectVisualDebugger();

        using CollisionCallback = std::function<void(Entity)>;
        using TriggerCallback = std::function<void(Entity)>;
        static void SetCollisionCallbacks(CollisionCallback begin, CollisionCallback end);
        static void SetTriggerCallbacks(TriggerCallback begin, TriggerCallback end);
    };

}
