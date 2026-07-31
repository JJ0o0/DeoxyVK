#pragma once

#include <deoxy/deoxy.hpp>

class SandboxApplication : public deoxy::core::Application {
    public:
        SandboxApplication();
    protected:
        void OnStart() override;
        void OnUpdate(float dt) override;
        void OnRender() override;
        void OnQuit() override;
    private:
        math::Vec3 m_position{0.0f};
        float m_rotation = 0.0f;

        Color32 m_clearColor{255, 255, 255, 255};

        MeshHandle m_quad;
};
