#include "asset_compiler.h"
#include "importers/mesh_importer.h"

#include <bx/error.h>
#include <bx/process.h>
#include <bx/string.h>

#include <graphics/shader.h>
#include <graphics/texture.h>
#include <logging/logging.h>
#include <uuid/uuid.h>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include <engine/meta/animation/animation.hpp>
#include <engine/meta/audio/audio_clip.hpp>
#include <engine/meta/ecs/entity.hpp>
#include <engine/meta/physics/physics_material.hpp>
#include <engine/meta/rendering/material.hpp>
#include <engine/meta/rendering/mesh.hpp>
#include <engine/meta/scripting/script.hpp>

#include <engine/scripting/ecs/systems/script_system.h>

#include <fstream>
#include <monopp/mono_jit.h>
#include <regex>
#include <subprocess/subprocess.hpp>

#include <core/base/platform/config.hpp>

namespace ace::asset_compiler
{

namespace
{

auto resolve_path(const std::string& key) -> fs::path
{
    return fs::absolute(fs::resolve_protocol(key));
}

auto resolve_input_file(const fs::path& key) -> fs::path
{
    fs::path absolute_path = fs::convert_to_protocol(key);
    absolute_path = fs::resolve_protocol(fs::replace(absolute_path, ":/meta", ":/data"));
    if(absolute_path.extension() == ".meta")
    {
        absolute_path.replace_extension();
    }
    return absolute_path;
}

auto escape_str(const std::string& str) -> std::string
{
    return "\"" + str + "\"";
}

auto run_process(const std::string& process,
                 const std::vector<std::string>& args_array,
                 bool chekc_retcode,
                 std::string& err) -> bool
{
    auto result = subprocess::call(process, args_array);
    err = result.out_output;

    if(!result.err_output.empty())
    {
        if(!err.empty())
        {
            err += "\n";
        }

        err += result.err_output;
    }

    if(err.find("error") != std::string::npos)
    {
        return false;
    }

    return result.retcode == 0;
}

} // namespace

template<>
auto compile<gfx::shader>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    bool result = true;

    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    std::string file = absolute_path.stem().string();
    fs::path dir = absolute_path.parent_path();

    fs::path include = fs::resolve_protocol("engine:/data/shaders");
    std::string str_include = include.string();
    fs::path varying = dir / (file + ".io");

    if(!fs::exists(varying, err))
    {
        varying = dir / "varying.def.io";
    }
    std::string str_varying = varying.string();

    std::string str_platform;
    std::string str_profile;
    std::string str_type;
    std::string str_opt = "3";

    bool vs = hpp::string_view(file).starts_with("vs_");
    bool fs = hpp::string_view(file).starts_with("fs_");
    bool cs = hpp::string_view(file).starts_with("cs_");

    auto extension = output.extension();
    auto renderer = gfx::get_renderer_based_on_filename_extension(extension.string());

    if(renderer == gfx::renderer_type::Vulkan)
    {
        str_platform = "windows";
        str_profile = "spirv";
    }

    if(renderer == gfx::renderer_type::Direct3D11 || renderer == gfx::renderer_type::Direct3D12)
    {
        str_platform = "windows";

        if(vs || fs)
        {
            str_profile = "s_5_0";
            str_opt = "3";
        }
        else if(cs)
        {
            str_profile = "s_5_0";
            str_opt = "1";
        }
    }
    else if(renderer == gfx::renderer_type::OpenGLES)
    {
        str_platform = "android";
        str_profile = "100_es";
    }
    else if(renderer == gfx::renderer_type::OpenGL)
    {
        str_platform = "linux";

        if(vs || fs)
            str_profile = "120";
        else if(cs)
            str_profile = "430";
    }
    else if(renderer == gfx::renderer_type::Metal)
    {
        str_platform = "osx";
        str_profile = "metal";
    }

    if(vs)
        str_type = "vertex";
    else if(fs)
        str_type = "fragment";
    else if(cs)
        str_type = "compute";
    else
        str_type = "unknown";

    std::vector<std::string> args_array = {
        "-f",
        str_input,
        "-o",
        str_output,
        "-i",
        str_include,
        "--varyingdef",
        str_varying,
        "--type",
        str_type,
        "--define",
        "BGFX_CONFIG_MAX_BONES=" + std::to_string(gfx::get_max_blend_transforms())
        //        "--Werror"
    };

    if(!str_platform.empty())
    {
        args_array.emplace_back("--platform");
        args_array.emplace_back(str_platform);
    }

    if(!str_profile.empty())
    {
        args_array.emplace_back("-p");
        args_array.emplace_back(str_profile);
    }

    if(!str_opt.empty())
    {
        args_array.emplace_back("-O");
        args_array.emplace_back(str_opt);
    }

    std::string error;

    {
        std::ofstream output_file(str_output);
        (void)output_file;
    }

    auto shaderc = fs::resolve_protocol("binary:/shaderc");

    if(!run_process(shaderc.string(), args_array, true, error))
    {
        APPLOG_ERROR("Failed compilation of {0} with error: {1}", str_input, error);
        result = false;
    }
    else
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }
    fs::remove(temp, err);

    return true;
}

template<>
auto compile<gfx::texture>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    bool result = true;
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    const std::vector<std::string> args_array = {
        "-f",
        str_input,
        "-o",
        str_output,
        "--as",
        "ktx",
        "-m",
        "-t",
        "BGRA8",
    };

    std::string error;

    {
        std::ofstream output_file(str_output);
        (void)output_file;
    }
    auto texturec = fs::resolve_protocol("binary:/texturec");

    if(!run_process(texturec.string(), args_array, false, error))
    {
        APPLOG_ERROR("Failed compilation of {0} with error: {1}", str_input, error);
        result = false;
    }
    else
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
    }
    fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    fs::remove(temp, err);

    return true;
}

template<>
auto compile<material>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    std::shared_ptr<material> material;
    {
        load_from_file(str_input, material);
        save_to_file_bin(str_output, material);
    }

    if(material)
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);

    return true;
}

template<>
auto compile<mesh>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    fs::path file = absolute_path.stem();
    fs::path dir = absolute_path.parent_path();

    mesh::load_data data;
    std::vector<animation_clip> animations;
    std::vector<importer::imported_material> materials;
    std::vector<importer::imported_texture> textures;

    if(!importer::load_mesh_data_from_file(am, absolute_path, data, animations, materials, textures))
    {
        APPLOG_ERROR("Failed compilation of {0}", str_input);
        return false;
    }
    if(!data.vertex_data.empty())
    {
        save_to_file_bin(str_output, data);

        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
        fs::remove(temp, err);
    }

    {
        for(const auto& animation : animations)
        {
            temp = fs::temp_directory_path(err);
            temp.append(hpp::to_string(generate_uuid()) + ".buildtemp");
            {
                save_to_file(temp.string(), animation);
            }

            fs::path anim_output;
            if(animation.name.empty())
            {
                anim_output = (dir / file).string() + ".anim";
            }
            else
            {
                anim_output = dir / (animation.name + ".anim");
            }

            fs::copy_file(temp, anim_output, fs::copy_options::overwrite_existing, err);
            fs::remove(temp, err);

            // APPLOG_INFO("Successful compilation of animation {0}", animation.name);
        }

        for(const auto& material : materials)
        {
            temp = fs::temp_directory_path(err);
            temp.append(hpp::to_string(generate_uuid()) + ".buildtemp");
            {
                save_to_file(temp.string(), material.mat);
            }
            fs::path mat_output;

            if(material.name.empty())
            {
                mat_output = (dir / file).string() + ".mat";
            }
            else
            {
                mat_output = dir / (material.name + ".mat");
            }

            fs::copy_file(temp, mat_output, fs::copy_options::overwrite_existing, err);
            fs::remove(temp, err);

            // APPLOG_INFO("Successful compilation of material {0}", material.name);
        }
    }

    return true;
}

template<>
auto compile<animation_clip>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    animation_clip anim;
    {
        load_from_file(str_input, anim);
        save_to_file_bin(str_output, anim);
    }

    if(!anim.channels.empty())
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);

    return true;
}

template<>
auto compile<prefab>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);
    std::string str_input = absolute_path.string();

    fs::error_code er;
    fs::copy_file(absolute_path, output, fs::copy_options::overwrite_existing, er);
    APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
    return true;
}

template<>
auto compile<scene_prefab>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);
    std::string str_input = absolute_path.string();

    // fs::error_code err;
    // fs::path temp = fs::temp_directory_path(err);
    // temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    // std::string str_output = temp.string();

    // {
    //     scene scn;
    //     load_from_file(str_input, scn);
    //     save_to_file_bin(str_output, scn);
    // }

    // fs::error_code er;
    // fs::copy_file(temp, output, fs::copy_options::overwrite_existing, er);
    // APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());

    // fs::remove(temp, err);

    fs::error_code er;
    fs::copy_file(absolute_path, output, fs::copy_options::overwrite_existing, er);
    APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());

    return true;
}

template<>
auto compile<physics_material>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    auto material = std::make_shared<physics_material>();
    {
        load_from_file(str_input, material);
        save_to_file_bin(str_output, material);
    }

    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);

    return true;
}

template<>
auto compile<audio_clip>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    audio::sound_data clip;
    {
        std::string error;
        if(!load_from_file(str_input, clip, error))
        {
            APPLOG_ERROR("Failed compilation of {0} with error: {1}", str_input, error);
            return false;
        }

        clip.convert_to_mono();
        save_to_file_bin(str_output, clip);
    }

    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);

    return true;
}
// Struct to hold the parsed error details
struct script_compilation_entry
{
    std::string file{}; // Path to the file
    int line{};         // Line number of the error
    std::string msg{};  // Full error line
};

// Function to parse all compilation errors
auto parse_compilation_errors(const std::string& log) -> std::vector<script_compilation_entry>
{
    // Regular expression to extract the warning details
    std::regex warning_regex(R"((.*)\((\d+),\d+\): error .*)");
    std::vector<script_compilation_entry> entries;

    // Use std::sregex_iterator to find all matches
    auto begin = std::sregex_iterator(log.begin(), log.end(), warning_regex);
    auto end = std::sregex_iterator();

    for(auto it = begin; it != end; ++it)
    {
        const std::smatch& match = *it;
        if(match.size() >= 3)
        {
            script_compilation_entry entry;
            entry.file = match[1].str();            // Extract file path
            entry.line = std::stoi(match[2].str()); // Extract line number
            entry.msg = match[0].str();             // Extract full warning line
            entries.emplace_back(std::move(entry));
        }
    }

    return entries;
}

// Function to parse all compilation warnings
auto parse_compilation_warnings(const std::string& log) -> std::vector<script_compilation_entry>
{
    // Regular expression to extract the warning details
    std::regex warning_regex(R"((.*)\((\d+),\d+\): error .*)");
    std::vector<script_compilation_entry> entries;

    // Use std::sregex_iterator to find all matches
    auto begin = std::sregex_iterator(log.begin(), log.end(), warning_regex);
    auto end = std::sregex_iterator();

    for(auto it = begin; it != end; ++it)
    {
        const std::smatch& match = *it;
        if(match.size() >= 3)
        {
            script_compilation_entry entry;
            entry.file = match[1].str();            // Extract file path
            entry.line = std::stoi(match[2].str()); // Extract line number
            entry.msg = match[0].str();             // Extract full warning line
            entries.emplace_back(std::move(entry));
        }
    }

    return entries;
}

template<>
auto compile<script_library>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    bool result = true;
    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);

    mono::compiler_params params;

    auto protocol = fs::extract_protocol(key).generic_string();

    if(protocol != "engine")
    {
        auto lib_compiled_key = fs::resolve_protocol(script_system::get_lib_compiled_key("engine"));

        params.references.emplace_back(lib_compiled_key.filename().string());

        params.references_locations.emplace_back(lib_compiled_key.parent_path().string());
    }

    auto assets = am.get_assets<script>(protocol);
    for(const auto& asset : assets)
    {
        if(asset)
        {
            params.files.emplace_back(fs::resolve_protocol(asset.id()).string());
        }
    }

    temp /= script_system::get_lib_name(protocol);

    auto temp_xml = temp;
    temp_xml.replace_extension(".xml");
    auto output_xml = output;
    output_xml.replace_extension(".xml");

    auto temp_mdb = temp;
    temp_mdb.concat(".mdb");
    auto output_mdb = output;
    output_mdb.concat(".mdb");

    std::string str_output = temp.string();

    params.output_name = str_output;
    params.output_doc_name = temp_xml.string();
    if(params.files.empty())
    {
        fs::remove(output, err);
        fs::remove(output_mdb, err);

        if(protocol == "engine")
        {
            return false;
        }

        return result;
    }

    params.debug = flags & script_library::compilation_flags::debug;

    std::string error;
    auto cmd = mono::create_compile_command_detailed(params);

    APPLOG_TRACE("Script Compile : \n {0} {1}", cmd.cmd, cmd.args);

    fs::remove(temp, err);
    fs::remove(temp_mdb, err);
    fs::remove(temp_xml, err);

    if(!run_process(cmd.cmd, cmd.args, true, error))
    {
        auto parsed_errors = parse_compilation_errors(error);

        if(!parsed_errors.empty())
        {
            for(const auto& error : parsed_errors)
            {
                APPLOG_ERROR_LOC(error.file.c_str(), error.line, "", error.msg);
            }
        }
        else
        {
            APPLOG_ERROR("Failed compilation of {0} with error: {1}", output.string(), error);
        }
        result = false;
    }
    else
    {
        if(!params.debug)
        {
            fs::remove(output_mdb, err);
        }

        fs::create_directories(output.parent_path(), err);

        if(protocol != "engine")
        {
            auto parsed_warnings = parse_compilation_warnings(error);

            for(const auto& warning : parsed_warnings)
            {
                APPLOG_WARNING_LOC(warning.file.c_str(), warning.line, "", warning.msg);
            }
        }

        APPLOG_INFO("Successful compilation of {0}", output.string());

        script_system::copy_compiled_lib(temp, output);
    }

    return result;
}

template<>
auto compile<script>(asset_manager& am, const fs::path& key, const fs::path& output, uint32_t flags) -> bool
{
    auto absolute_path = resolve_input_file(key);

    fs::error_code er;
    fs::copy_file(absolute_path, output, fs::copy_options::overwrite_existing, er);

    // APPLOG_INFO("Successful copy to {0}", output.string());

    script_system::set_needs_recompile(fs::extract_protocol(fs::convert_to_protocol(key)).string());

    return true;
}

} // namespace ace::asset_compiler
