#pragma once

#include <deoxy/deoxy.hpp>
#include <camera.hpp>

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

        Camera m_camera;

        MeshHandle m_quad;
};
