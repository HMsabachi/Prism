#pragma once

#include "ISystem.h"

namespace Prism {

    class Scene;

    class PRISM_API Physics3DSystem : public ISystem {
    public:
        explicit Physics3DSystem(Scene* scene);
        ~Physics3DSystem() override;

        void OnFixedUpdate(float ts) override;
        void OnRuntimeStart() override;
        void OnRuntimeStop() override;

    private:
        Scene* m_Scene;
    };

}
