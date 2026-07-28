#include <sandbox_application.hpp>

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
    // Atualiza jogo, câmera, input etc.
}

void SandboxApplication::OnRender() {
    // TEMPORÁRIO SÓ PRA TESTAR O RENDERER
    GetRenderer().Render();
}

void SandboxApplication::OnQuit() {
    deoxy::platform::Logger::Info("Sandbox stopped");
}
