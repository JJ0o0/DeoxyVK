#include <sandbox_application.hpp>

const auto CLEAR_COLOR_0 = ParseHexColor32("#1B1B1E").value(); // Carbon Black

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
    1, 3, 2
};

SandboxApplication::SandboxApplication()
    : Application({
        .Title = "DeoxyVK Sandbox",
        .Width = 800,
        .Height = 600,
        .Resizable = false
     }) { }

void SandboxApplication::OnStart() {
    Logger::Info("Sandbox started");

    auto& renderer = GetRenderer();
    renderer.SetClearColor(CLEAR_COLOR_0);

    m_quad = renderer.CreateMesh(QUAD_VERTICES, QUAD_INDICES);

    GetWindow().SetRelativeMouseMode(true);
}

void SandboxApplication::OnUpdate(float deltaTime) {
    auto& renderer = GetRenderer();
    const auto& input = GetInput();

    const float rotation_speed = 20.0f;
    if (m_rotation > 360) m_rotation = 0;
    m_rotation += rotation_speed * deltaTime;

    m_camera.Update(input, deltaTime);
    renderer.SetCamera(m_camera.GetView(), m_camera.GetProjection(GetWindow().GetAspectRatio()));

    if (input.WasPressed(Key::F8)) Quit();
}

void SandboxApplication::OnRender() {
    Mat4 model{1.0f};
    model = Translate(model, m_position);
    model = RotateZ(model, ToRadians(m_rotation));
    model = Scale(model, Vec3{0.5f});

    GetRenderer().DrawMesh(m_quad, model);
}

void SandboxApplication::OnQuit() {
    Logger::Info("Sandbox stopped");
}
