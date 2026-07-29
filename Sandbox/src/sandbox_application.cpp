#include <sandbox_application.hpp>

constexpr auto CLEAR_COLOR_0 = ParseHexColor32("#1B1B1E").value(); // Carbon Black
constexpr auto CLEAR_COLOR_1 = ParseHexColor32("#E4E4E4").value(); // Ash White

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
    auto& input = GetInput();

    if (input.WasPressed(Key::Space)) {
        m_clearColor = m_clearColor != CLEAR_COLOR_1 ? CLEAR_COLOR_1 : CLEAR_COLOR_0;
        GetRenderer().SetClearColor(m_clearColor);
    }
}

void SandboxApplication::OnRender() {
    // Algo como RenderMesh
}

void SandboxApplication::OnQuit() {
    deoxy::platform::Logger::Info("Sandbox stopped");
}
