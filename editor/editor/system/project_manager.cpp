#include "project_manager.h"
#include <editor/assets/asset_watcher.h>
#include <editor/editing/editing_manager.h>
#include <editor/editing/editor_actions.h>
#include <editor/editing/thumbnail_manager.h>
#include <editor/meta/deploy/deploy.hpp>
#include <editor/meta/system/project_manager.hpp>
#include <editor/meta/settings/settings.hpp>

#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_compiler.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/settings/settings.hpp>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/scripting/ecs/systems/script_system.h>
#include <engine/threading/threader.h>

#include <filesystem/watcher.h>
#include <fstream>
#include <graphics/graphics.h>
#include <logging/logging.h>
#include <serialization/associative_archive.h>
#include <serialization/serialization.h>

namespace ace
{

void project_manager::close_project(rtti::context& ctx)
{
    if(has_open_project())
    {
        save_project_settings();
        save_deploy_settings();
        project_settings_ = {};
        deploy_settings_ = {};
    }

    auto& scr = ctx.get_cached<script_system>();
    scr.unload_app_domain();

    auto& em = ctx.get_cached<editing_manager>();
    em.clear();

    auto& tm = ctx.get_cached<thumbnail_manager>();
    tm.clear_thumbnails();

    auto& ec = ctx.get_cached<ecs>();
    ec.unload_scene();

    set_name({});

    auto& aw = ctx.get_cached<asset_watcher>();
    aw.unwatch_assets(ctx, "app:/");

    load_config();
}

auto project_manager::open_project(rtti::context& ctx, const fs::path& project_path) -> bool
{
    close_project(ctx);

    {
        fs::error_code err;
        fs::create_directories(fs::resolve_protocol("app:/data"), err);
        fs::create_directories(fs::resolve_protocol("app:/compiled"), err);
        fs::create_directories(fs::resolve_protocol("app:/meta"), err);
        fs::create_directories(fs::resolve_protocol("app:/settings"), err);
    }

    fs::error_code err;
    if(!fs::exists(project_path, err))
    {
        APPLOG_ERROR("Project directory doesn't exist {0}", project_path.string());
        return false;
    }

    fs::add_path_protocol("app", project_path);

    set_name(project_path.filename().string());

    save_config();
    save_editor_settings();

    auto& aw = ctx.get_cached<asset_watcher>();
    aw.watch_assets(ctx, "app:/");

    auto& scr = ctx.get_cached<script_system>();
    scr.load_app_domain(ctx, true);

    load_project_settings();
    save_project_settings();

    load_deploy_settings();
    save_deploy_settings();

    editor_actions::generate_script_workspace();

    return true;
}

void project_manager::load_project_settings()
{
    load_from_file(fs::resolve_protocol("app:/settings/settings.cfg").string(), project_settings_);
}

void project_manager::save_project_settings()
{
    save_to_file(fs::resolve_protocol("app:/settings/settings.cfg").string(), project_settings_);
}

void project_manager::load_deploy_settings()
{
    load_from_file(fs::resolve_protocol("app:/settings/deploy.cfg").string(), deploy_settings_);
}

void project_manager::save_deploy_settings()
{
    save_to_file(fs::resolve_protocol("app:/settings/deploy.cfg").string(), deploy_settings_);
}

void project_manager::create_project(rtti::context& ctx, const fs::path& project_path)
{
    fs::error_code err;
    fs::add_path_protocol("app", project_path);

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
        auto ar = ser20::create_iarchive_associative(output);

        try_load(ar, ser20::make_nvp("options", options_));

        auto& items = options_.recent_projects;
        auto iter = std::begin(items);
        while(iter != items.end())
        {
            auto& item = *iter;

            if(!fs::exists(item.path, err))
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

void project_manager::load_editor_settings()
{
    fs::error_code err;
    const fs::path config = fs::resolve_protocol("editor:/config/settings.cfg");
    if(!fs::exists(config, err))
    {
        save_editor_settings();
    }
    else
    {
        load_from_file(config.string(), editor_settings_);
    }
}

void project_manager::save_editor_settings()
{
    fs::error_code err;
    fs::create_directory(fs::resolve_protocol("editor:/config"), err);

    const fs::path config = fs::resolve_protocol("editor:/config/settings.cfg");
    save_to_file(config.string(), editor_settings_);
}

auto project_manager::get_name() const -> const std::string&
{
    return project_name_;
}

void project_manager::set_name(const std::string& name)
{
    project_name_ = name;
}

auto project_manager::get_settings() -> settings&
{
    return project_settings_;
}

auto project_manager::get_deploy_settings() -> deploy_settings&
{
    return deploy_settings_;
}

auto project_manager::get_editor_settings() -> editor_settings&
{
    return editor_settings_;
}

auto project_manager::get_options() -> options&
{
    return options_;
}

auto project_manager::has_open_project() const -> bool
{
    return !get_name().empty();
}

void project_manager::save_config()
{
    if(has_open_project())
    {
        auto& rp = options_.recent_projects;
        auto project_path = fs::resolve_protocol("app:/");
        if(std::find_if(std::begin(rp),
                        std::end(rp),
                        [&](const auto& prj)
                        {
                            return project_path.generic_string() == prj.path;
                        }) == std::end(rp))
        {
            project prj{project_path.generic_string()};
            rp.emplace_back(std::move(prj));
        }

        std::sort(std::begin(rp), std::end(rp), [](const auto& lhs, const auto& rhs)
        {
            fs::error_code ec;
            auto lhs_path = fs::path(lhs.path);
            auto lhs_time = fs::last_write_time(lhs_path / "settings" / "deploy.cfg", ec);

            auto rhs_path = fs::path(rhs.path);
            auto rhs_time = fs::last_write_time(rhs_path / "settings" / "deploy.cfg", ec);

            return lhs_time > rhs_time;
        });
    }

    fs::error_code err;
    fs::create_directory(fs::resolve_protocol("editor:/config"), err);
    const auto project_config_file = fs::resolve_protocol("editor:/config/project.cfg").string();
    std::ofstream output(project_config_file);
    auto ar = ser20::create_oarchive_associative(output);

    try_save(ar, ser20::make_nvp("options", options_));
}

project_manager::project_manager(rtti::context& ctx)
{
    load_config();
    load_editor_settings();

    auto& scripting = ctx.get_cached<script_system>();
    scripting.set_debug_config(editor_settings_.debugger.ip,
                               editor_settings_.debugger.port,
                               editor_settings_.debugger.loglevel);

    auto& ev = ctx.get_cached<events>();
    ev.on_script_recompile.connect(sentinel_,
                                   -1000,
                                   [this](rtti::context& ctx, const std::string& protocol)
                                   {
                                       if(protocol == "app" && has_open_project())
                                       {
                                           editor_actions::generate_script_workspace();
                                       }
                                   });
}

auto project_manager::init(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    auto& aw = ctx.get_cached<asset_watcher>();
    aw.watch_assets(ctx, "editor:/", true);

    return true;
}

auto project_manager::deinit(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    close_project(ctx);

    auto& aw = ctx.get_cached<asset_watcher>();
    aw.unwatch_assets(ctx, "editor:/");

    return true;
}

project_manager::~project_manager()
{
    save_config();
    save_editor_settings();
}
} // namespace ace
