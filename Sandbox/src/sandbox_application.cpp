#include <sandbox_application.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const auto CLEAR_COLOR_0 = ParseHexColor32("#1B1B1E").value(); // Carbon Black
const auto CLEAR_COLOR_1 = ParseHexColor32("#E4E4E4").value(); // Ash White

const std::array<Vertex, 4> QUAD_VERTICES {
    Vertex {
        .Position = { 0.5f, -0.5f, 0.0f },
        .Color = { 1.0f, 0.0f, 0.0f }
    },
    Vertex {
        .Position = { 0.5f, 0.5f, 0.0f },
        .Color = { 0.0f, 1.0f, 0.0f }
    },
    Vertex {
        .Position = { -0.5f, -0.5f, 0.0f },
        .Color = { 0.0f, 0.0f, 1.0f }
    },
    Vertex {
        .Position = { -0.5f, 0.5f, 0.0f },
        .Color = { 1.0f, 0.0f, 1.0f }
    }
};

const std::array<uint32_t, 6> QUAD_INDICES {
    0, 1, 2,
    1, 2, 3
};

SandboxApplication::SandboxApplication()
    : Application({
        .Title = "DeoxyVK Sandbox",
        .Width = 800,
        .Height = 600,
        .Resizable = false
     }) { }

void SandboxApplication::OnStart() {
    deoxy::platform::Logger::Info("Sandbox started");

    m_clearColor = CLEAR_COLOR_0;

    auto& renderer = GetRenderer();
    renderer.SetClearColor(m_clearColor);

    m_quad = renderer.CreateMesh(QUAD_VERTICES, QUAD_INDICES);
}

void SandboxApplication::OnUpdate(float deltaTime) {
    auto& renderer = GetRenderer();
    const auto& input = GetInput();

    const float movement_speed = 1.5f;
    math::Vec2 direction{0.0f};
    if (input.IsPressed(Key::W)) direction.y -= 1.0f;
    if (input.IsPressed(Key::S)) direction.y += 1.0f;
    if (input.IsPressed(Key::D)) direction.x += 1.0f;
    if (input.IsPressed(Key::A)) direction.x -= 1.0f;

    m_position.x += movement_speed * direction.x * deltaTime;
    m_position.y += movement_speed * direction.y * deltaTime;

    const float rotation_speed = 20.0f;
    if (m_rotation > 360) m_rotation = 0;
    m_rotation += rotation_speed * deltaTime;

    if (input.WasPressed(Key::Space)) {
        m_clearColor = m_clearColor != CLEAR_COLOR_1 ? CLEAR_COLOR_1 : CLEAR_COLOR_0;
        renderer.SetClearColor(m_clearColor);
    }
}

void SandboxApplication::OnRender() {
    math::Mat4 model{1.0f};
    model = glm::translate(model, m_position);
    model = glm::rotate(model, math::ToRadians(m_rotation), math::Vec3(0, 0, 1));
    model = glm::scale(model, math::Vec3{0.5f});

    GetRenderer().DrawMesh(m_quad, model);
}

void SandboxApplication::OnQuit() {
    deoxy::platform::Logger::Info("Sandbox stopped");
}
