#include "editor_actions.h"

#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/assets/impl/asset_reader.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/ecs/entity.hpp>

#include <editor/editing/editing_manager.h>
#include <editor/system/project_manager.h>

#include <filedialog/filedialog.h>
#include <filesystem/filesystem.h>
#include <subprocess/subprocess.hpp>

namespace ace
{

namespace
{

auto parse_dependencies(const std::vector<char>& input_buffer, const fs::path& fs_parent_path)
    -> std::vector<std::string>
{
    std::string input(input_buffer.begin(), input_buffer.end());

    std::vector<std::string> dependencies;
    std::stringstream ss(input);
    std::string line;

    auto trim_line = [](std::string& line)
    {
        // Trim trailing spaces and \r
        line.erase(std::find_if(line.rbegin(),
                                line.rend(),
                                [](char ch)
                                {
                                    return !std::isspace(int(ch));
                                })
                       .base(),
                   line.end());
    };

    while(std::getline(ss, line))
    {
#ifdef _WIN32
        // parse dependencies output
        if(line.find("[ApplicationDirectory]") != std::string::npos)
        {
            std::size_t pos = line.find(":");
            if(pos != std::string::npos)
            {
                std::string dependency = line.substr(pos + 2); // +2 to skip ": "
                trim_line(dependency);

                dependencies.push_back(dependency);
            }
#else
        // parse ldd output
        size_t pos = line.find("=> ");
        if(pos != std::string::npos)
        {
            std::string dependency = line.substr(pos + 3); // +3 to remove '=> '
            size_t address_pos = dependency.find(" (0x");
            if(address_pos != std::string::npos)
            {
                dependency = dependency.substr(0, address_pos); // remove the address
            }

            trim_line(dependency);

            fs::path fs_path(dependency);

            if(fs::exists(fs_path) && fs::exists(fs_parent_path))
            {
                if(fs::equivalent(fs_path.parent_path(), fs_parent_path))
                {
                    dependencies.push_back(fs_path);
                }
            }
        }

#endif

    }
    return dependencies;
}

auto get_dependencies(const fs::path& file) -> std::vector<std::string>
{
    std::vector<std::string> params;

#ifdef _WIN32
    params.emplace_back(fs::resolve_protocol("editor:/programs/dependencies/Dependencies.exe"));
    params.emplace_back("-modules");
    params.emplace_back(file);

#else

    params.emplace_back("ldd");
    params.emplace_back(file);
#endif

    auto parent_path = file.parent_path();

    auto obuf = subprocess::check_output(params);
    return parse_dependencies(obuf.buf, parent_path);
}

auto save_scene_impl(rtti::context& ctx, const fs::path& path) -> bool
{
    auto& ec = ctx.get<ecs>();
    save_to_file(path, ec.get_scene());
    return true;
}

auto save_scene_as_impl(rtti::context& ctx, fs::path& path) -> bool
{
    std::string picked;
    if(native::save_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Save scene as",
                                fs::resolve_protocol("app:/data").string()))
    {
        auto& em = ctx.get<editing_manager>();

        path = picked;

        if(!ex::is_format<scene_prefab>(path.extension().generic_string()))
        {
            path.replace_extension(ex::get_format<scene_prefab>(false));
        }

        return save_scene_impl(ctx, path);
    }

    return false;
}

} // namespace

auto editor_actions::new_scene(rtti::context& ctx) -> bool
{
    auto& em = ctx.get<editing_manager>();
    em.close_project();

    auto& ec = ctx.get<ecs>();
    ec.unload_scene();

    auto& def = ctx.get<defaults>();
    def.create_default_3d_scene(ctx, ec.get_scene());
    return true;
}
auto editor_actions::open_scene(rtti::context& ctx) -> bool
{
    std::string picked;
    if(native::open_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Open scene",
                                fs::resolve_protocol("app:/data").string()))
    {
        auto path = fs::convert_to_protocol(picked);
        if(ex::is_format<scene_prefab>(path.extension().generic_string()))
        {
            auto& em = ctx.get<editing_manager>();
            em.close_project();

            auto& am = ctx.get<asset_manager>();
            auto asset = am.load<scene_prefab>(path.string());

            auto& ec = ctx.get<ecs>();
            ec.unload_scene();

            auto& scene = ec.get_scene();
            return scene.load_from(asset);
        }
    }
    return false;
}
auto editor_actions::save_scene(rtti::context& ctx) -> bool
{
    auto& ec = ctx.get<ecs>();
    auto& scene = ec.get_scene();

    if(!scene.source)
    {
        fs::path picked;
        if(save_scene_as_impl(ctx, picked))
        {
            auto path = fs::convert_to_protocol(picked);

            auto& am = ctx.get<asset_manager>();
            scene.source = am.load<scene_prefab>(path.string());
            return true;
        }
    }
    else
    {
        auto path = fs::resolve_protocol(scene.source.id());
        save_scene_impl(ctx, path);
    }

    return true;
}
auto editor_actions::save_scene_as(rtti::context& ctx) -> bool
{
    fs::path p;
    return save_scene_as_impl(ctx, p);
}
auto editor_actions::close_project(rtti::context& ctx) -> bool
{
    auto& pm = ctx.get<project_manager>();
    pm.close_project(ctx);
    return true;
}

auto editor_actions::deploy_project(rtti::context& ctx, const deploy_params& params)
    -> std::map<std::string, itc::job_shared_future<void>>
{
    auto& th = ctx.get<threader>();

    std::map<std::string, itc::job_shared_future<void>> jobs;
    std::vector<itc::shared_future<void>> jobs_seq;

    auto deploy_location = params.deploy_location;

    auto startup = asset_reader::resolve_compiled_path(params.startup_scene.id());

    if(params.deploy_dependencies)
    {
        auto job = th.pool
                       ->schedule(
                           [deploy_location]()
                           {
                               fs::error_code ec;

                               fs::remove_all(deploy_location, ec);
                               fs::create_directories(deploy_location, ec);

                               fs::path app_executable =
                                   fs::resolve_protocol("binary:/game" + fs::executable_extension());
                               auto deps = get_dependencies(app_executable);

                               for(const auto& dep : deps)
                               {
                                   fs::copy(dep, deploy_location, fs::copy_options::overwrite_existing, ec);
                               }

                               fs::copy(app_executable, deploy_location, fs::copy_options::overwrite_existing, ec);
                           })
                       .share();
        jobs["Deploying Dependencies"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [deploy_location, startup]()
                           {
                               auto startup_path = fs::resolve_protocol("app:/cache/__startup__" +
                                                                        ex::get_format<scene_prefab>() + ".asset");
                               fs::error_code ec;
                               fs::copy(startup, startup_path, fs::copy_options::overwrite_existing, ec);

                               fs::path cached_data = deploy_location / "data" / "app" / "cache";
                               auto data = fs::resolve_protocol("app:/cache");

                               fs::remove_all(cached_data, ec);
                               fs::create_directories(cached_data, ec);
                               fs::copy(data, cached_data, fs::copy_options::recursive, ec);

                               fs::remove(startup_path, ec);
                           })
                       .share();

        jobs["Deploying Project Data"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [deploy_location]()
                           {
                               fs::path cached_data = deploy_location / "data" / "engine" / "cache";
                               auto data = fs::resolve_protocol("engine:/cache");

                               fs::error_code ec;
                               fs::remove_all(cached_data, ec);
                               fs::create_directories(cached_data, ec);
                               fs::copy(data, cached_data, fs::copy_options::recursive, ec);
                           })
                       .share();
        jobs["Deploying Engine Data..."] = job;
        jobs_seq.emplace_back(job);
    }

    itc::when_all(std::begin(jobs_seq), std::end(jobs_seq))
        .then(itc::this_thread::get_id(),
              [deploy_location, params](auto f)
              {
                  if(params.deploy_and_run)
                  {
                      subprocess::check_output(deploy_location / ("game" + fs::executable_extension()));
                  }
                  else
                  {
                      fs::show_in_graphical_env(deploy_location);
                  }
              });

    return jobs;
}

} // namespace ace