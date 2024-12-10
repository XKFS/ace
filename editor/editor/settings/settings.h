#pragma once

#include <base/basetypes.hpp>
#include <string>
#include <filesystem/filesystem.h>

namespace ace
{

struct editor_settings
{
    struct debugger_settings
    {
        std::string ip = "127.0.0.1";
        uint32_t port = 55555;
        uint32_t loglevel = 0;
    } debugger;

    struct external_tools_settings
    {
        fs::path vscode_executable;
    } external_tools;

};
} // namespace ace
