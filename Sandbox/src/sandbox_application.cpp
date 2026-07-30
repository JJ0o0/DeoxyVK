#include <sandbox_application.hpp>

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
    GetRenderer().SetClearColor(m_clearColor);
}

void SandboxApplication::OnUpdate(float deltaTime) {
    auto& renderer = GetRenderer();
    const auto& input = GetInput();

    if (input.WasPressed(deoxy::input::Key::T) && !m_quad) {
        m_quad = renderer.CreateMesh(QUAD_VERTICES, QUAD_INDICES);
    }

    if (input.WasPressed(deoxy::input::Key::R) && m_quad) {
        renderer.DestroyMesh(m_quad);
        m_quad = {};
    }

    if (input.WasPressed(Key::Space)) {
        m_clearColor = m_clearColor != CLEAR_COLOR_1 ? CLEAR_COLOR_1 : CLEAR_COLOR_0;
        GetRenderer().SetClearColor(m_clearColor);
    }
}

void SandboxApplication::OnRender() {
    if (m_quad) GetRenderer().DrawMesh(m_quad);
}

void SandboxApplication::OnQuit() {
    deoxy::platform::Logger::Info("Sandbox stopped");
}
