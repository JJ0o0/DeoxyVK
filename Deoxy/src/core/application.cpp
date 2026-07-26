#include <deoxy/core/application.hpp>
#include <deoxy/platform/logger.hpp>
#include <deoxy/platform/message_box.hpp>

#include <SDL3/SDL_timer.h>
#include <algorithm>

namespace deoxy::core {
    Application::Application(const platform::WindowProperties& properties)
        : m_windowProperties(properties) {
        platform::Logger::Initialize();
    }

    int Application::Run() {
        try {
            platform::Logger::Info("Initializing...");
            m_window = std::make_unique<platform::Window>(m_windowProperties);

            OnStart();

            constexpr float NanosecondsToSeconds = 1.0f / 1'000'000'000.0f;
            std::uint64_t previousTime = SDL_GetTicksNS();
            while (!m_window->ShouldClose() && m_running) {
                const std::uint64_t currentTime = SDL_GetTicksNS();
                const std::uint64_t elapsedNanoseconds = currentTime - previousTime;

                previousTime = currentTime;

                const float deltaTime = std::min(
                    static_cast<float>(elapsedNanoseconds) * NanosecondsToSeconds,
                    0.1f
                );

                m_window->PollEvents();

                OnUpdate(deltaTime);
                OnRender();
            }

            OnQuit();
            m_window.reset();
            platform::Logger::Info("Quitting...");

            return DEOXY_SUCCESS;
        } catch (const std::exception& e) {
            platform::MessageBox::ShowError("DeoxyVK Fatal Error", "{}", e.what());

            m_window.reset();
            return DEOXY_FAILURE;
        }
    }
}
