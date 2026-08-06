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

    renderer.SetAmbientLight(AmbientLight{
        .Intensity = 0.06f,
        .LightColor = {0.65f, 0.75f, 1.0f},
    });

    // Criando o objeto do cubo
    MeshData cube = MeshGenerator::CreateCube(1.0f);
    m_mesh = renderer.CreateMesh(cube);

    TextureCreateInfo checkerCI {
        .ColorSpace = TextureColorSpace::SRGB,
        .Filter = TextureFilter::Linear,
        .MipmapFilter = TextureMipmapFilter::Linear,
        .WrapMode = TextureWrapMode::Repeat,
    };

    m_checker = renderer.CreateTexture("assets/textures/checker.png", checkerCI);

    m_checkerMaterial = renderer.CreateMaterial(MaterialCreateInfo {
        .Albedo = m_checker,
        .UVScale = Vec2{2.0f}
    });

    createPointLight(renderer, {
        .Position = Vec3{0.0f, 1.5f, 0.0f},
        .Range = 5.0f,
        .LightColor = Color{1.0f, 0.2f, 0.05f},
        .Intensity = 3.0f
    });

    createPointLight(renderer, {
        .Position = Vec3{0.0f, -1.5f, 0.0f},
        .Range = 5.0f,
        .LightColor = Color{0.2f, 1.0f, 0.05f},
        .Intensity = 3.0f
    });

    createPointLight(renderer, {
        .Position = Vec3{1.5f, 0.0f, 0.0f},
        .Range = 5.0f,
        .LightColor = Color{0.2f, 0.05f, 1.0f},
        .Intensity = 3.0f
    });

    createPointLight(renderer, {
        .Position = Vec3{0.0f, 0.0f, 1.5f},
        .Range = 5.0f,
        .LightColor = Color{1.0f, 0.05f, 1.0f},
        .Intensity = 3.0f
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
    model = Scale(model, Vec3{0.5f});

    // Aplicando matriz e desenhando
    auto& renderer = GetRenderer();
    renderer.DrawMesh(m_mesh, m_checkerMaterial, model);
}

void SandboxApplication::OnQuit() {
    // Destruindo tudo
    auto& renderer = GetRenderer();
    for (auto& light : m_pointLights) renderer.DestroyPointLight(light);

    renderer.DestroyMaterial(m_checkerMaterial);
    renderer.DestroyTexture(m_checker);
    renderer.DestroyMesh(m_mesh);

    Logger::Info("Sandbox stopped");
}

void SandboxApplication::createPointLight(Renderer& renderer, const PointLight& light) {
    const auto handle = renderer.CreatePointLight(light);

    if (!handle) {
        Logger::Warn("Point Light couldn't be created: limit exceeded.");
        return;
    }

    m_pointLights.push_back(*handle);
}
