#pragma once

#include <deoxy/deoxy.hpp>

class SandboxApplication : public deoxy::core::Application {
    public:
        SandboxApplication();
    protected:
        void OnStart() override;
        void OnUpdate(float dt) override;
        void OnRender() override;
        void OnQuit() override;
};
