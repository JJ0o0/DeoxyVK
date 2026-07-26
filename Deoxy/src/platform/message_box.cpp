#include <deoxy/platform/message_box.hpp>
#include <deoxy/platform/logger.hpp>
#include <tinyfiledialogs.h>

namespace deoxy::platform {
    void MessageBox::ShowError(const std::string& title, const std::string& msg) {
        Logger::Error("{}: {}", title, msg);

        tinyfd_messageBox(
            title.c_str(),
            msg.c_str(),
            "ok",
            "error",
            1
        );
    }
}
