#pragma once
#include <engine/assets/asset_manager.h>
#include <filesystem/filesystem.h>

namespace ace
{
namespace asset_compiler
{

template<typename T>
auto compile(asset_manager& am, const fs::path& key, const fs::path& output_key, uint32_t flags = 0) -> bool;

} // namespace asset_compiler
} // namespace ace
