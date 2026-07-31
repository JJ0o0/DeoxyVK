#pragma once

#include <deoxy/deoxy.hpp>

class SandboxApplication : public Application {
    public:
        SandboxApplication();
    protected:
        void OnStart() override;
        void OnUpdate(float dt) override;
        void OnRender() override;
        void OnQuit() override;
    private:
        Vec3 m_position{0.0f};
        float m_rotation = 0.0f;

        Color32 m_clearColor{255, 255, 255, 255};

        MeshHandle m_quad;
};
