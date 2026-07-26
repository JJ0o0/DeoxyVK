#pragma once

#include <deoxy/platform/window.hpp>
#include <deoxy/graphics/renderer.hpp>

namespace deoxy::core {
    static constexpr int DEOXY_SUCCESS = 0;
    static constexpr int DEOXY_FAILURE = 1;

    class Application {
        public:
            Application(const platform::WindowProperties& windowProperties = {});
            virtual ~Application() = default;

            int Run();
            void Quit();
        protected:
            virtual void OnStart() = 0;
            virtual void OnUpdate(float dt) = 0;
            virtual void OnRender() = 0;
            virtual void OnQuit() = 0;

            platform::Window& GetWindow();
            const platform::Window& GetWindow() const;

            graphics::Renderer& GetRenderer();
            const graphics::Renderer& GetRenderer() const;
        private:
            bool m_running = true;

            platform::WindowProperties m_windowProperties;
            std::unique_ptr<platform::Window> m_window;
            std::unique_ptr<graphics::Renderer> m_renderer;
    };
}
