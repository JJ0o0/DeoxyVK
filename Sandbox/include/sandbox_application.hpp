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
        float m_time = 0.0f;
        deoxy::graphics::Color32 m_clearColor{255, 255, 255, 255};
        deoxy::graphics::MeshHandle m_quad;
};
