#pragma once

namespace Prism {

class Scene;

class PRISM_API ISystem {
public:
    virtual ~ISystem() = default;
    virtual void OnUpdate(float ts) {}
    virtual void OnFixedUpdate(float ts) {}
    virtual void OnRuntimeStart() {}
    virtual void OnRuntimeStop() {}
};

}
