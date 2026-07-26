#pragma once

#include <memory>

namespace deoxy::platform {
    class Window;
}

namespace deoxy::graphics {
    class Renderer {
        public:
            Renderer(platform::Window& window);
            ~Renderer();

            void Render();
        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;
    };
}
