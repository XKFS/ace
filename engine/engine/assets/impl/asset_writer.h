#pragma once
#include "../asset_handle.h"

#include <filesystem/filesystem.h>

namespace ace
{
namespace asset_writer
{
template<typename T>
void save_to_file(const fs::path& key, const asset_handle<T>& asset)
{
    fs::path absolute_key = fs::absolute(fs::resolve_protocol(key));
    save_to_file(absolute_key.string(), asset.get());
}
}
} // namespace ace
