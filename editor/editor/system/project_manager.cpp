#include "project_manager.h"
#include "../assets/asset_compiler.h"
#include "../assets/asset_extensions.h"
// #include "../editing/editing_system.h"
#include "../meta/system/project_manager.hpp"

#include <engine/assets/asset_manager.h>
#include <engine/threading/threader.h>
#include <filesystem/watcher.h>
#include <graphics/graphics.h>
#include <logging/logging.h>
#include <serialization/associative_archive.h>
#include <serialization/serialization.h>
// #include <core/system/subsystem.h>
// #include <core/tasks/task_system.h>

// #include <runtime/ecs/ecs.h>
// #include <runtime/system/events.h>

#include <fstream>

namespace ace
{
using namespace std::literals;

static std::vector<fs::path> remove_meta_tag(const std::vector<fs::path>& synced_paths)
{
    std::decay_t<decltype(synced_paths)> reduced;
    reduced.reserve(synced_paths.size());
    for(const auto& synced_path : synced_paths)
    {
        reduced.emplace_back(fs::replace(synced_path, ".meta", ""));
    }
    return reduced;
}

static void unwatch(std::vector<uint64_t>& watchers)
{
    for(const auto& id : watchers)
    {
        fs::watcher::unwatch(id);
    }
    watchers.clear();
}

template<typename T>
static std::uint64_t watch_assets(rtti::context& ctx, const fs::path& dir, const fs::path& wildcard, bool reload_async)
{
    auto& am = ctx.get<asset_manager>();
    auto& ts = ctx.get<threader>();

    fs::path watch_dir = (dir / wildcard).make_preferred();

    return fs::watcher::watch(
        watch_dir,
        true,
        true,
        500ms,
        [&am, &ts](const auto& entries, bool is_initial_list)
        {
            for(const auto& entry : entries)
            {
                auto p = fs::reduce_trailing_extensions(entry.path);
                auto data_key = fs::convert_to_protocol(p);
                auto key = fs::replace(data_key.generic_string(), ":/cache", ":/data").generic_string();

                APPLOG_TRACE("{0}", fs::to_string(entry));

                if(entry.type == fs::file_type::regular)
                {
                    if(entry.status == fs::watcher::entry_status::removed)
                    {
                        itc::invoke(itc::main_thread::get_id(),
                                    [key, &am]()
                                    {
                                        am.unload_asset<T>(key);
                                    });
                    }
                    else if(entry.status == fs::watcher::entry_status::renamed)
                    {
                        auto old_p = fs::reduce_trailing_extensions(entry.last_path);
                        auto old_data_key = fs::convert_to_protocol(old_p);
                        auto old_key = fs::replace(old_data_key.generic_string(), ":/cache", ":/data").generic_string();

                        itc::invoke(itc::main_thread::get_id(),
                                    [old_key, key, &am]()
                                    {
                                        am.rename_asset<T>(old_key, key);
                                    });
                    }
                    else
                    {
                        using namespace runtime;
                        load_flags flags = is_initial_list ? load_flags::standard : load_flags::reload;

                        // created or modified
                        itc::invoke(itc::main_thread::get_id(),
                                    [flags, key, &am]()
                                    {
                                        am.load<T>(key, flags);
                                    });
                    }
                }
            }
        });
}

template<typename T>
static void add_to_syncer(rtti::context& ctx,
                          std::vector<uint64_t>& watchers,
                          fs::syncer& syncer,
                          const fs::path& dir,
                          const fs::syncer::on_entry_removed_t& on_removed,
                          const fs::syncer::on_entry_renamed_t& on_renamed)
{
    auto& ts = ctx.get<threader>();
    auto on_modified = [&ts](const auto& ref_path, const auto& synced_paths, bool is_initial_listing)
    {
        auto task = ts.pool->schedule(
            [ref_path, synced_paths = remove_meta_tag(synced_paths), is_initial_listing]()
            {
                fs::path output = synced_paths.front();
                fs::error_code err;
                if(is_initial_listing && fs::exists(output, err))
                {
                    return;
                }
                asset_compiler::compile<T>(ref_path, output);
            });
    };

    for(const auto& type : ex::get_suported_formats<T>())
    {
//        syncer.set_mapping(type + ".meta", {".asset"}, on_modified, on_modified, on_removed, on_renamed);
        const auto watch_id = watch_assets<T>(ctx, dir, "*" + type, true);
        watchers.push_back(watch_id);
    }
}

//template<>
//void add_to_syncer<gfx::shader>(rtti::context& ctx,
//                                std::vector<uint64_t>& watchers,
//                                fs::syncer& syncer,
//                                const fs::path& dir,
//                                const fs::syncer::on_entry_removed_t& on_removed,
//                                const fs::syncer::on_entry_renamed_t& on_renamed)
//{
//    auto& ts = ctx.get<threader>();

//    auto on_modified = [&ts](const auto& ref_path, const auto& synced_paths, bool is_initial_listing)
//    {
//        auto task = ts.pool->schedule(
//            [ref_path, synced_paths = remove_meta_tag(synced_paths), is_initial_listing]()
//            {
//                const auto& renderer_extension = gfx::get_renderer_filename_extension();
//                auto it = std::find_if(std::begin(synced_paths),
//                                       std::end(synced_paths),
//                                       [&renderer_extension](const auto& key)
//                                       {
//                                           return key.stem().extension() == renderer_extension;
//                                       });

//                if(it == std::end(synced_paths))
//                {
//                    return;
//                }

//                fs::path output = *it;

//                fs::error_code err;
//                if(is_initial_listing && fs::exists(output, err))
//                {
//                    return;
//                }

//                asset_compiler::compile<gfx::shader>(ref_path, output);
//            });
//    };

//    for(const auto& type : ex::get_suported_formats<gfx::shader>())
//    {
//        syncer.set_mapping(type + ".meta",
//                           {".dx11.asset", ".dx12.asset", ".gl.asset", ".vlk.asset"},
//                           on_modified,
//                           on_modified,
//                           on_removed,
//                           on_renamed);

//        const auto watch_id = watch_assets<gfx::shader>(ctx, dir, "*" + type, true);
//        watchers.push_back(watch_id);
//    }
//}

void project_manager::close_project(rtti::context& ctx)
{
    auto& am = ctx.get<asset_manager>();

    //	auto& ecs = core::get_subsystem<runtime::entity_component_system>();
    //	auto& es = core::get_subsystem<editing_system>();
    //	es.close_project();
    //	ecs.dispose();

    am.clear("app:/data");
    unwatch(app_watchers_);
    app_meta_syncer_.unsync();
    app_cache_syncer_.unsync();
    load_config();
}

bool project_manager::open_project(rtti::context& ctx, const fs::path& project_path)
{
    close_project(ctx);

    fs::error_code err;
    if(!fs::exists(project_path, err))
    {
        APPLOG_ERROR("Project directory doesn't exist {0}", project_path.string());
        return false;
    }

    fs::add_path_protocol("app:", project_path);

    set_name(project_path.filename().string());

    save_config();

//    setup_meta_syncer(ctx, app_meta_syncer_, fs::resolve_protocol("app:/data"), fs::resolve_protocol("app:/meta"));
    setup_cache_syncer(ctx,
                       app_watchers_,
                       app_cache_syncer_,
                       fs::resolve_protocol("app:/meta"),
                       fs::resolve_protocol("app:/cache"));

    //	auto& es = core::get_subsystem<editing_system>();
    //	es.load_editor_camera();

    return true;
}

void project_manager::create_project(rtti::context& ctx, const fs::path& project_path)
{
    fs::error_code err;
    fs::add_path_protocol("app:", project_path);
    fs::create_directory(fs::resolve_protocol("app:/data"), err);
    fs::create_directory(fs::resolve_protocol("app:/cache"), err);
    fs::create_directory(fs::resolve_protocol("app:/meta"), err);
    fs::create_directory(fs::resolve_protocol("app:/settings"), err);

    open_project(ctx, project_path);
}

void project_manager::load_config()
{
    fs::error_code err;
    const fs::path project_config_file = fs::resolve_protocol("editor:/config/project.cfg");
    if(!fs::exists(project_config_file, err))
    {
        save_config();
    }
    else
    {
        std::ifstream output(project_config_file.string());
        cereal::iarchive_associative_t ar(output);

        try_load(ar, cereal::make_nvp("options", options_));

        auto& items = options_.recent_project_paths;
        auto iter = std::begin(items);
        while(iter != items.end())
        {
            auto& item = *iter;

            if(!fs::exists(item, err))
            {
                iter = items.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
}

void project_manager::setup_directory(rtti::context& ctx, fs::syncer& syncer)
{
    const auto on_dir_modified = [](const auto& /*ref_path*/, const auto& /*synced_paths*/, bool /*is_initial_listing*/)
    {

    };
    const auto on_dir_removed = [](const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::remove_all(synced_path, err);
        }
    };

    const auto on_dir_renamed = [](const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::rename(synced_path.first, synced_path.second, err);
        }
    };
    syncer.set_directory_mapping(on_dir_modified, on_dir_modified, on_dir_removed, on_dir_renamed);
}

void project_manager::setup_meta_syncer(rtti::context& ctx,
                                        fs::syncer& syncer,
                                        const fs::path& data_dir,
                                        const fs::path& meta_dir)
{
    setup_directory(ctx, syncer);

    const auto on_file_removed = [](const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::remove_all(synced_path, err);
        }
    };

    const auto on_file_renamed = [](const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::rename(synced_path.first, synced_path.second, err);
        }
    };

    const auto on_file_modified = [](const auto& /*ref_path*/, const auto& synced_paths, bool is_initial_listing)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            if(is_initial_listing && fs::exists(synced_path, err))
            {
                return;
            }
            std::ofstream output(synced_path.string(), std::ofstream::trunc);
            output.write("metadata", 8);
        }
    };

    static const std::vector<std::vector<std::string>> types = ex::get_all_formats();

    for(const auto& asset_set : types)
    {
        for(const auto& type : asset_set)
        {
            syncer.set_mapping(type, {".meta"}, on_file_modified, on_file_modified, on_file_removed, on_file_renamed);
        }
    }

    syncer.sync(data_dir, meta_dir);
}

void project_manager::setup_cache_syncer(rtti::context& ctx,
                                         std::vector<uint64_t>& watchers,
                                         fs::syncer& syncer,
                                         const fs::path& meta_dir,
                                         const fs::path& cache_dir)
{
    setup_directory(ctx, syncer);

    auto on_removed = [](const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            auto synced_asset = fs::replace(synced_path, ".meta", "");
            fs::error_code err;
            fs::remove_all(synced_asset, err);
        }
    };

    auto on_renamed = [](const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            auto synced_old_asset = fs::replace(synced_path.first, ".meta", "");
            auto synced_new_asset = fs::replace(synced_path.second, ".meta", "");
            fs::error_code err;
            fs::rename(synced_old_asset, synced_new_asset, err);
        }
    };

    add_to_syncer<gfx::texture>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
//    add_to_syncer<gfx::shader>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    //	add_to_syncer<mesh>(watchers, syncer, cache_dir, on_removed, on_renamed);
    //	add_to_syncer<audio::sound>(watchers, syncer, cache_dir, on_removed, on_renamed);
    //	add_to_syncer<material>(watchers, syncer, cache_dir, on_removed, on_renamed);
    //	add_to_syncer<runtime::animation>(watchers, syncer, cache_dir, on_removed, on_renamed);
    //	add_to_syncer<prefab>(watchers, syncer, cache_dir, on_removed, on_renamed);
    //	add_to_syncer<scene>(watchers, syncer, cache_dir, on_removed, on_renamed);

    syncer.sync(meta_dir, cache_dir);
}

void project_manager::save_config()
{
    auto& rp = options_.recent_project_paths;
    auto project_path = fs::resolve_protocol("app:/");
    if(project_path != "" && std::find(std::begin(rp), std::end(rp), project_path.generic_string()) == std::end(rp))
    {
        rp.push_back(project_path.generic_string());
    }

    fs::error_code err;
    fs::create_directory(fs::resolve_protocol("editor:/config"), err);
    const auto project_config_file = fs::resolve_protocol("editor:/config/project.cfg").string();
    std::ofstream output(project_config_file);
    cereal::oarchive_associative_t ar(output);

    try_save(ar, cereal::make_nvp("options", options_));
}

project_manager::project_manager(rtti::context& ctx)
{
    load_config();
//    setup_meta_syncer(ctx,
//                      engine_meta_syncer_,
//                      fs::resolve_protocol("engine:/data"),
//                      fs::resolve_protocol("engine:/meta"));

//    setup_cache_syncer(ctx,
//                       engine_watchers_,
//                       engine_cache_syncer_,
//                       fs::resolve_protocol("engine:/meta"),
//                       fs::resolve_protocol("engine:/cache"));
//    setup_meta_syncer(ctx,
//                      editor_meta_syncer_,
//                      fs::resolve_protocol("editor:/data"),
//                      fs::resolve_protocol("editor:/meta"));
//    setup_cache_syncer(ctx,
//                       editor_watchers_,
//                       editor_cache_syncer_,
//                       fs::resolve_protocol("editor:/meta"),
//                       fs::resolve_protocol("editor:/cache"));
}

project_manager::~project_manager()
{
    save_config();

    unwatch(app_watchers_);

    app_meta_syncer_.unsync();
    app_cache_syncer_.unsync();

    unwatch(editor_watchers_);

    editor_meta_syncer_.unsync();
    editor_cache_syncer_.unsync();

    unwatch(engine_watchers_);

    engine_meta_syncer_.unsync();
    engine_cache_syncer_.unsync();
}
} // namespace ace
