#include <sandbox_application.hpp>

const auto CLEAR_COLOR_0 = ParseHexColor32("#1B1B1E").value(); // Carbon Black

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

    MeshData cube = MeshGenerator::CreateCube(1.0f);
    m_quad = renderer.CreateMesh(cube.Vertices, cube.Indices);

    GetWindow().SetRelativeMouseMode(true);
}

void SandboxApplication::OnUpdate(float deltaTime) {
    auto& renderer = GetRenderer();
    const auto& input = GetInput();

    const float rotation_speed = 20.0f;
    m_rotation += rotation_speed * deltaTime;

    m_camera.Update(input, deltaTime);
    renderer.SetCamera(m_camera.GetView(), m_camera.GetProjection(GetWindow().GetAspectRatio()));

    if (input.WasPressed(Key::F8)) Quit();
}

void SandboxApplication::OnRender() {
    Mat4 model{1.0f};
    model = Translate(model, m_position);
    model = RotateX(model, ToRadians(m_rotation * 0.8f));
    model = RotateY(model, ToRadians(m_rotation * 0.9f));
    model = RotateZ(model, ToRadians(m_rotation));
    model = Scale(model, Vec3{0.5f});

    GetRenderer().DrawMesh(m_quad, model);
    GetRenderer().DrawMesh(m_quad, model);
}

void SandboxApplication::OnQuit() {
    Logger::Info("Sandbox stopped");
}
