#pragma once

#include <deoxy/deoxy.hpp>

struct CameraProperties {
    float Sensitivity = 0.05f;
    float MinimumFov = 30.0f;
    float MaximumFov = 90.0f;
    float NearPlane = 0.1f;
    float FarPlane = 100.0f;
};

class Camera {
    public:
        Camera(const CameraProperties& properties = {}) : m_properties(properties) {}
        void Update(const Input& input, float deltatime);

        Mat4 GetView() const;
        Mat4 GetProjection(float aspectRatio) const;

        const CameraProperties& GetProperties() const { return m_properties; }
    private:
        CameraProperties m_properties;

        Vec3 m_position{0.0f, 0.0f, 3.0f};
        Vec3 m_forward{0.0f, 0.0f, -1.0f};
        Vec3 m_worldUp{0.0f, 1.0f, 0.0f};

        float m_fov = 60.0f;
        float m_targetFov = 60.0f;

        float m_yaw = -90.0f;
        float m_pitch = 0.0f;
};
