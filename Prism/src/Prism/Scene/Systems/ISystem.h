#pragma once

namespace Prism {

    class Scene;

    class PRISM_API ISystem
    {
    public:
        virtual ~ISystem() = default;

        // ── 生命周期 ──
        virtual void OnCreate(){}
        virtual void OnDestroy(){}
        virtual void OnRuntimeStart(){}
        virtual void OnRuntimeStop(){}


        virtual void OnEarlyUpdate(float dt){}
        virtual void OnFixedUpdate(float dt){}
        virtual void OnPreUpdate(float dt){}
        virtual void OnUpdate(float dt){}
        virtual void OnPreLateUpdate(float dt){} 
        virtual void OnLateUpdate(float dt){}
        virtual void OnPostLateUpdate(float dt){}
        virtual void OnRender(float dt){}


        virtual void OnImGuiRender(){}
    };

}
