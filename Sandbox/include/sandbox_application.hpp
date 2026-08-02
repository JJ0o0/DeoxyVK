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
        bool m_mouseLocked = true;

        Vec3 m_position{0.0f};
        float m_rotation = 0.0f;

        Camera m_camera;

        MeshHandle m_mesh;
        TextureHandle m_checker;
        MaterialHandle m_checkerMaterial;
};
