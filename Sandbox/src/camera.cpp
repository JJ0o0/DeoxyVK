#include <camera.hpp>

void Camera::Update(const Input& input, float deltaTime) {
    // CAMERA LOOK
    const Vec2 mouseDelta = input.GetMouseDelta();

    m_yaw += mouseDelta.x * m_properties.Sensitivity;
    m_pitch -= mouseDelta.y * m_properties.Sensitivity;
    m_pitch = Clamp(m_pitch, -89.0f, 89.0f);

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

    Vec3 targetVelocity{0.0f};
    const bool isMoving = Length(movement) > 0.0f;
    if (isMoving) {
        movement = Normalize(movement);
        targetVelocity += movement * movement_speed;
    }

    if (!isMoving && Length(m_velocity) < 0.001f) {
        m_velocity = Vec3{0.0f};
    }

    const float sharpness = isMoving ? m_properties.Acceleration : m_properties.Deceleration;
    const float weight = ExponentialSmoothing(sharpness, deltaTime);
    m_velocity += (targetVelocity - m_velocity) * weight;
    m_position += m_velocity * deltaTime;

    // ZOOM
    const float scrollY = input.GetMouseWheelDelta().y;
    const float zoomStep = 5.0f;

    m_targetFov -= scrollY * zoomStep;
    m_targetFov = Clamp(m_targetFov, m_properties.MinimumFov, m_properties.MaximumFov);

    const float zoomLerpSpeed = 10.0f;
    const float zoomWeight = Clamp(zoomLerpSpeed * deltaTime, 0.0f, 1.0f);

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
