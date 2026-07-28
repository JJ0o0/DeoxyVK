#pragma once

#include <deoxy/graphics/color.hpp>
#include <memory>

namespace deoxy::platform {
    class Window;
}

namespace deoxy::graphics {
    class Renderer {
        public:
            Renderer(platform::Window& window);
            ~Renderer();

            bool BeginFrame();
            void EndFrame();

            void SetClearColor(Color color);
            void SetClearColor(Color32 color32);
        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    };
}
