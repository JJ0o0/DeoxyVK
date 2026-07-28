#include <sandbox_application.hpp>

using namespace deoxy::core::math;

SandboxApplication::SandboxApplication()
    : Application({
        .Title = "DeoxyVK Sandbox",
        .Width = 800,
        .Height = 600,
        .Resizable = false
     }) { }

void SandboxApplication::OnStart() {
    deoxy::platform::Logger::Info("Sandbox started");
}

void SandboxApplication::OnUpdate(float deltaTime){
    m_time += deltaTime;

    m_clearColor.R = 0.5f + 0.5f * Sin(m_time);
    m_clearColor.G = 0.5f + 0.5f * Sin(m_time + TWO_PI / 3);
    m_clearColor.B = 0.5f + 0.5f * Sin(m_time + (TWO_PI * 2) / 3);

    GetRenderer().SetClearColor(m_clearColor);
}

void SandboxApplication::OnRender() {
    // Algo como RenderMesh
}

void SandboxApplication::OnQuit() {
    deoxy::platform::Logger::Info("Sandbox stopped");
}
