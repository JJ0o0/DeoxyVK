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

        float m_lightRotation = 0.0f;

        Camera m_camera;

        MeshHandle m_mesh;

        TextureHandle m_checker;
        MaterialHandle m_checkerMaterial;

        std::optional<SpotLightHandle> m_spotLight;
};
