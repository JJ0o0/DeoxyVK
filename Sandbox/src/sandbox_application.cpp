#include <sandbox_application.hpp>

using namespace deoxy::core::math;
using namespace deoxy::graphics;

constexpr auto CLEAR_COLOR = ParseHexColor32("#1B1B1E").value(); // Carbon Black

SandboxApplication::SandboxApplication()
    : Application({
        .Title = "DeoxyVK Sandbox",
        .Width = 800,
        .Height = 600,
        .Resizable = false
     }) { }

void SandboxApplication::OnStart() {
    deoxy::platform::Logger::Info("Sandbox started");

    GetRenderer().SetClearColor(CLEAR_COLOR);
}

void SandboxApplication::OnUpdate(float deltaTime) {

}

void SandboxApplication::OnRender() {
    // Algo como RenderMesh
}

void SandboxApplication::OnQuit() {
    deoxy::platform::Logger::Info("Sandbox stopped");
}
