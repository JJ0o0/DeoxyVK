#include <camera.hpp>

void Camera::Update(const Input& input, float deltaTime) {
    // CAMERA LOOK
    const Vec2 mouseDelta = input.GetMouseDelta();

    m_yaw += mouseDelta.x * m_properties.Sensitivity;
    m_pitch -= mouseDelta.y * m_properties.Sensitivity;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    const float yaw = ToRadians(m_yaw);
    const float pitch = ToRadians(m_pitch);

    m_forward = Normalize(Vec3 {
        Cos(pitch) * Cos(yaw),
        Sin(pitch),
        Cos(pitch) * Sin(yaw)
    });

    Vec3 flatForward {
        m_forward.x,
        0.0f,
        m_forward.z
    };

    flatForward = Normalize(flatForward);

    const Vec3 right = Normalize(Cross(m_forward, m_worldUp));

    // CAMERA POS
    const float movement_speed = 1.5f;
    Vec3 movement{0.0f};
    if (input.IsPressed(Key::W)) movement += flatForward;
    if (input.IsPressed(Key::S)) movement -= flatForward;
    if (input.IsPressed(Key::D)) movement += right;
    if (input.IsPressed(Key::A)) movement -= right;
    if (input.IsPressed(Key::Space)) movement += m_worldUp;
    if (input.IsPressed(Key::LeftControl)) movement -= m_worldUp;
    if (Length(movement) > 0.0f) {
        movement = Normalize(movement);

        m_position += movement * movement_speed * deltaTime;
    }

    // ZOOM
    const float scrollY = input.GetMouseWheelDelta().y;
    const float zoomStep = 5.0f;

    m_targetFov -= scrollY * zoomStep;
    m_targetFov = std::clamp(m_targetFov, m_properties.MinimumFov, m_properties.MaximumFov);

    const float zoomLerpSpeed = 10.0f;
    const float zoomWeight = std::clamp(zoomLerpSpeed * deltaTime, 0.0f, 1.0f);

    m_fov = Lerp(m_fov, m_targetFov, zoomWeight);
}

Mat4 Camera::GetView() const { return LookAt(m_position, m_position + m_forward, m_worldUp); }

Mat4 Camera::GetProjection(float aspectRatio) const {
    return Perspective(
        ToRadians(m_fov),
        aspectRatio,
        m_properties.NearPlane, m_properties.FarPlane
    );
}
