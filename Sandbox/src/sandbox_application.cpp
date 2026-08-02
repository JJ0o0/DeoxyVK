#include <sandbox_application.hpp>

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

    // Colocando o clear color
    const auto clearColor = ParseHexColor32("#1B1B1E").value();
    renderer.SetClearColor(clearColor);

    // Criando o objeto do cubo
    MeshData cube = MeshGenerator::CreateCube(1.0f);
    m_mesh = renderer.CreateMesh(cube);
    m_checker = renderer.CreateTexture("assets/textures/checker.png");
    m_checkerMaterial = renderer.CreateMaterial(MaterialCreateInfo {
        .Albedo = m_checker,
        .Tint = Color{0.5f, 0.12f, 0.8f}
    });

    // Setando configurações da janela
    auto& window = GetWindow();
    window.SetIcon("assets/icon.png");
    window.SetRelativeMouseMode(m_mouseLocked);
}

void SandboxApplication::OnUpdate(float deltaTime) {
    auto& window = GetWindow();
    auto& renderer = GetRenderer();
    const auto& input = GetInput();

    // Calculando a rotação do objeto
    const float rotation_speed = 20.0f;
    m_rotation += rotation_speed * deltaTime;

    // Setando a camera
    if (m_mouseLocked) m_camera.Update(input, deltaTime);
    renderer.SetCamera(m_camera.GetView(), m_camera.GetProjection(window.GetAspectRatio()));

    if (input.WasPressed(Key::Escape)) {
        m_mouseLocked = !m_mouseLocked;
        window.SetRelativeMouseMode(m_mouseLocked);
    }

    if (input.WasPressed(Key::F8)) Quit();
    if (input.WasPressed(Key::F11)) window.SetFullscreen(!window.IsFullscreen());
}

void SandboxApplication::OnRender() {
    // Movendo, rotacionando e escalando o objeto
    Mat4 model{1.0f};
    model = Translate(model, Vec3{0.0f});
    model = RotateX(model, ToRadians(m_rotation * 0.8f));
    model = RotateY(model, ToRadians(m_rotation * 0.9f));
    model = RotateZ(model, ToRadians(m_rotation));
    model = Scale(model, Vec3{0.5f});

    // Aplicando matriz e desenhando
    auto& renderer = GetRenderer();
    renderer.DrawMesh(m_mesh, m_checkerMaterial, model);
}

void SandboxApplication::OnQuit() {
    // Destruindo tudo
    auto& renderer = GetRenderer();
    renderer.DestroyMaterial(m_checkerMaterial);
    renderer.DestroyTexture(m_checker);
    renderer.DestroyMesh(m_mesh);

    Logger::Info("Sandbox stopped");
}
