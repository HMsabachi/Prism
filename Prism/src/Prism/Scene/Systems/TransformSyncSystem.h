#pragma once

#include "ISystem.h"

namespace Prism {

    class Scene;

    class PRISM_API TransformSyncSystem : public ISystem
    {
    public:
        explicit TransformSyncSystem(Scene* scene);

        void OnFixedUpdate(float dt) override; 
        void OnPreUpdate(float dt) override; 

    private:
        Scene* m_Scene;
    };

}
