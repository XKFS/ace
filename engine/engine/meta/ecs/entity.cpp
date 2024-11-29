#include "entity.hpp"

#include <chrono>
#include <serialization/archives/yaml.hpp>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include "components/all_components.h"

#include "entt/entity/fwd.hpp"
#include "logging/logging.h"

#include <hpp/utility.hpp>
#include <sstream>

namespace ace
{

auto const_handle_cast(entt::const_handle chandle) -> entt::handle
{
    entt::handle handle(*const_cast<entt::handle::registry_type*>(chandle.registry()), chandle.entity());
    return handle;
}

template<typename Entity>
struct entity_components
{
    Entity entity;
};

template<typename Entity>
struct entity_data
{
    entity_components<Entity> components;
};

struct save_context
{
    bool to_prefab{};
    entt::const_handle save_source{};
};

struct load_context
{
    entt::registry* reg{};
    std::map<entt::entity, entt::handle> mapping;
};

thread_local load_context* load_ctx_ptr{};
thread_local save_context* save_ctx_ptr{};

auto push_load_context(entt::registry& registry) -> bool
{
    if(load_ctx_ptr)
    {
        return false;
    }
    load_ctx_ptr = new load_context();
    load_ctx_ptr->reg = &registry;
    return true;
}

void pop_load_context(bool push_result)
{
    if(push_result && load_ctx_ptr)
    {
        delete load_ctx_ptr;
        load_ctx_ptr = {};
    }
}

auto get_load_context() -> load_context&
{
    assert(load_ctx_ptr);
    return *load_ctx_ptr;
}

auto push_save_context() -> bool
{
    if(save_ctx_ptr)
    {
        return false;
    }
    save_ctx_ptr = new save_context();

    return true;
}

void pop_save_context(bool push_result)
{
    if(push_result && save_ctx_ptr)
    {
        delete save_ctx_ptr;
        save_ctx_ptr = {};
    }
}

auto get_save_context() -> save_context&
{
    assert(save_ctx_ptr);
    return *save_ctx_ptr;
}

auto is_parent(entt::const_handle potential_parent, entt::const_handle child) -> bool
{
    if(!potential_parent)
    {
        return false;
    }
    // Traverse up the hierarchy from `child`
    while(true)
    {
        // Access the transform component once per entity
        const auto* transform = child.try_get<transform_component>();
        if(!transform)
        {
            return false; // Reached the root without finding `potential_parent`
        }
        auto parent = transform->get_parent();
        if(!parent)
        {
            return false;
        }

        if(parent == potential_parent)
        {
            return true; // Found the parent relationship
        }

        child = parent; // Move up the hierarchy
    }
}
auto find_root(entt::const_handle e) -> entt::const_handle
{
    // Loop to find the root entity
    while(true)
    {
        // Access the `transform_component` once
        const auto* transform = e.try_get<transform_component>();
        if(!transform || !transform->get_parent())
        {
            break; // If no parent, we are at the root
        }
        e = transform->get_parent(); // Move to the parent entity
    }
    return e; // Root entity
}

auto are_related(entt::const_handle lhs, entt::const_handle rhs) -> bool
{
    return find_root(lhs) == find_root(rhs);
}

enum entity_flags
{
    none,
    resolve_with_existing,
    resolve_with_loaded,
};

} // namespace ace

using namespace ace;

namespace ser20
{

template<typename Archive>
void save_entity(Archive& ar, const entt::const_handle& obj, entity_flags flags)
{
    entt::handle::entity_type id = obj.valid() ? obj.entity() : entt::null;
    try_save(ar, ser20::make_nvp("id", id));
}

template<typename Archive>
void load_entity(Archive& ar, entt::handle& obj, entity_flags flags)
{
    entt::handle::entity_type id{};
    try_load(ar, ser20::make_nvp("id", id));

    bool valid = id != entt::null && id != entt::handle::entity_type(0);
    if(valid)
    {
        auto& load_ctx = get_load_context();
        auto it = load_ctx.mapping.find(id);
        if(it != load_ctx.mapping.end())
        {
            obj = it->second;
        }
        else if(obj)
        {
            load_ctx.mapping[id] = obj;
        }
        else
        {
            if(flags == entity_flags::resolve_with_existing)
            {
                entt::handle check_entity(*load_ctx.reg, id);
                if(check_entity)
                {
                    obj = check_entity;
                    load_ctx.mapping[id] = obj;
                }
                else
                {
                    obj = {};
                }
            }
            else
            {
                obj = entt::handle(*load_ctx.reg, load_ctx.reg->create());
                load_ctx.mapping[id] = obj;
            }
        }
    }
    else
    {
        obj = {};
    }
}

SAVE(entt::const_handle)
{
    save_entity(ar, obj, entity_flags::none);
}
SAVE_INSTANTIATE(entt::const_handle, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(entt::const_handle, ser20::oarchive_binary_t);

LOAD(entt::handle)
{
    load_entity(ar, obj, entity_flags::none);
}

LOAD_INSTANTIATE(entt::handle, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(entt::handle, ser20::iarchive_binary_t);

SAVE(const_entity_handle_link)
{
    // Saving entity links is a little more complex than just entities
    // The rule is as follows.
    // If we are saving as single entity hierarch :
    // If the entity link is not part of it :
    // -> if we are saving to prefab, break the link
    // -> if we are duplicating resolve the link on load with exsisting scene.
    entity_flags flags = entity_flags::resolve_with_loaded;
    entt::const_handle to_save = obj.handle;

    auto& save_ctx = get_save_context();

    bool is_saving_single = save_ctx.save_source.valid();
    if(is_saving_single)
    {
        // is the entity a child of the hierarchy that we are saving?
        bool save_source_is_parent = is_parent(save_ctx.save_source, obj.handle);

        // if it is an external entity
        if(!save_source_is_parent)
        {
            if(save_ctx.to_prefab)
            {
                // when saving prefabs, external entities
                // should not be saved
                to_save = {};
            }
            else
            {
                // when saving entities for duplication purpose, external entities
                // should not be resolved from the existing scene
                flags = entity_flags::resolve_with_existing;
            }
        }
    }

    try_save(ar, ser20::make_nvp("flags", flags));
    save_entity(ar, to_save, flags);
}
SAVE_INSTANTIATE(const_entity_handle_link, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(const_entity_handle_link, ser20::oarchive_binary_t);

LOAD(entity_handle_link)
{
    entity_flags flags{};
    try_load(ar, ser20::make_nvp("flags", flags));

    load_entity(ar, obj.handle, flags);
}

LOAD_INSTANTIATE(entity_handle_link, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(entity_handle_link, ser20::iarchive_binary_t);

SAVE(entity_components<entt::const_handle>)
{
    hpp::for_each_tuple_type<ace::all_serializeable_components>(
        [&](auto index)
        {
            using ctype = std::tuple_element_t<decltype(index)::value, ace::all_serializeable_components>;
            auto component = obj.entity.try_get<ctype>();

            auto name = rttr::get_pretty_name(rttr::type::get<ctype>());

            auto has_name = "Has" + name;
            try_save(ar, ser20::make_nvp(has_name, component != nullptr));

            if(component)
            {
                try_save(ar, ser20::make_nvp(name, *component));
            }
        });
}
SAVE_INSTANTIATE(entity_components<entt::const_handle>, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(entity_components<entt::const_handle>, ser20::oarchive_binary_t);

LOAD(entity_components<entt::handle>)
{
    hpp::for_each_tuple_type<ace::all_serializeable_components>(
        [&](auto index)
        {
            using ctype = std::tuple_element_t<decltype(index)::value, ace::all_serializeable_components>;

            auto component_type = rttr::type::get<ctype>();
            std::string name = component_type.get_name().data();
            auto meta_id = component_type.get_metadata("pretty_name");
            if(meta_id)
            {
                name = meta_id.to_string();
            }

            auto has_name = "Has" + name;
            bool has_component = false;
            try_load(ar, ser20::make_nvp(has_name, has_component));

            if(has_component)
            {
                auto& component = obj.entity.emplace_or_replace<ctype>();
                try_load(ar, ser20::make_nvp(name, component));
            }
        });
}
LOAD_INSTANTIATE(entity_components<entt::handle>, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(entity_components<entt::handle>, ser20::iarchive_binary_t);

SAVE(entity_data<entt::const_handle>)
{
    SAVE_FUNCTION_NAME(ar, obj.components.entity);
    try_save(ar, ser20::make_nvp("components", obj.components));
}
SAVE_INSTANTIATE(entity_data<entt::const_handle>, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(entity_data<entt::const_handle>, ser20::oarchive_binary_t);

LOAD(entity_data<entt::handle>)
{
    entt::handle e;
    LOAD_FUNCTION_NAME(ar, e);

    obj.components.entity = e;
    try_load(ar, ser20::make_nvp("components", obj.components));
}
LOAD_INSTANTIATE(entity_data<entt::handle>, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(entity_data<entt::handle>, ser20::iarchive_binary_t);

} // namespace ser20

namespace ace
{
namespace
{

void flatten_hierarchy(entt::const_handle obj, std::vector<entity_data<entt::const_handle>>& entities)
{
    auto& trans_comp = obj.get<transform_component>();
    const auto& children = trans_comp.get_children();

    entity_data<entt::const_handle> data;
    data.components.entity = obj;

    entities.emplace_back(std::move(data));

    entities.reserve(entities.size() + children.size());
    for(const auto& child : children)
    {
        flatten_hierarchy(child, entities);
    }
}

template<typename Archive>
void save_to_archive(Archive& ar, entt::const_handle obj)
{
    bool pushed = push_save_context();

    bool is_root = obj.all_of<root_component>();
    if(!is_root)
    {
        const_handle_cast(obj).emplace<root_component>();
    }

    auto& trans_comp = obj.get<transform_component>();

    std::vector<entity_data<entt::const_handle>> entities;
    flatten_hierarchy(obj, entities);

    try_save(ar, ser20::make_nvp("entities", entities));

    static const std::string version = "1.0.0";
    try_save(ar, ser20::make_nvp("version", version));

    if(!is_root)
    {
        const_handle_cast(obj).erase<root_component>();
    }

    pop_save_context(pushed);
}

template<typename Archive>
auto load_from_archive_impl(Archive& ar, entt::registry& registry, const std::function<void(entt::handle)>& on_create)
    -> entt::handle
{
    std::vector<entity_data<entt::handle>> entities;
    try_load(ar, ser20::make_nvp("entities", entities));

    std::string version;
    try_load(ar, ser20::make_nvp("version", version));

    entt::handle result{};
    if(!entities.empty())
    {
        result = entities.front().components.entity;
    }

    if(on_create)
    {
        for(const auto& e : entities)
        {
            on_create(e.components.entity);
        }
    }

    return result;
}

template<typename Archive>
auto load_from_archive_start(Archive& ar,
                             entt::registry& registry,
                             const std::function<void(entt::handle)>& on_create = {}) -> entt::handle
{
    bool pushed = push_load_context(registry);

    auto obj = load_from_archive_impl(ar, registry, on_create);

    pop_load_context(pushed);

    return obj;
}

template<typename Archive>
void load_from_archive(Archive& ar, entt::handle& obj, const std::function<void(entt::handle)>& on_create = {})
{
    obj = load_from_archive_start(ar, *obj.registry(), on_create);
}

template<typename Archive>
void save_to_archive(Archive& ar, const entt::registry& reg)
{
    bool pushed = push_save_context();

    size_t count = 0;
    reg.view<transform_component, root_component>().each(
        [&](auto e, auto&& comp1, auto&& comp2)
        {
            count++;
        });

    try_save(ar, ser20::make_nvp("entities_count", count));
    reg.view<transform_component, root_component>().each(
        [&](auto e, auto&& comp1, auto&& comp2)
        {
            save_to_archive(ar, entt::const_handle(reg, e));
        });

    pop_save_context(pushed);
}

template<typename Archive>
void load_from_archive(Archive& ar, entt::registry& reg)
{
    reg.clear();
    size_t count = 0;
    try_load(ar, ser20::make_nvp("entities_count", count));

    bool pushed = push_load_context(reg);

    for(size_t i = 0; i < count; ++i)
    {
        entt::handle e(reg, reg.create());
        load_from_archive(ar, e);
    }

    pop_load_context(pushed);
}

} // namespace

void save_to_stream(std::ostream& stream, entt::const_handle obj)
{
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_oarchive_associative(stream);
        save_to_archive(ar, obj);
    }
}

void save_to_file(const std::string& absolute_path, entt::const_handle obj)
{
    std::ofstream stream(absolute_path);

    bool pushed = push_save_context();
    auto& save_ctx = get_save_context();
    save_ctx.save_source = obj;
    save_ctx.to_prefab = true;

    save_to_stream(stream, obj);

    save_ctx.to_prefab = false;
    save_ctx.save_source = {};
    pop_save_context(pushed);
}

void save_to_stream_bin(std::ostream& stream, entt::const_handle obj)
{
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);

        save_to_archive(ar, obj);
    }
}

void save_to_file_bin(const std::string& absolute_path, entt::const_handle obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);

    bool pushed = push_save_context();
    auto& save_ctx = get_save_context();
    save_ctx.save_source = obj;
    save_ctx.to_prefab = true;

    save_to_stream_bin(stream, obj);
    save_ctx.to_prefab = false;
    save_ctx.save_source = {};
    pop_save_context(pushed);
}

void load_from_view(std::string_view view, entt::handle& obj)
{
    if(!view.empty())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(view.data(), view.size());
        load_from_archive(ar, obj);
    }
}

void load_from_stream(std::istream& stream, entt::handle& obj)
{
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(stream);
        load_from_archive(ar, obj);
    }
}

void load_from_file(const std::string& absolute_path, entt::handle& obj)
{
    std::ifstream stream(absolute_path);
    load_from_stream(stream, obj);
}

void load_from_stream_bin(std::istream& stream, entt::handle& obj)
{
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        ser20::iarchive_binary_t ar(stream);
        load_from_archive(ar, obj);
    }
}

void load_from_file_bin(const std::string& absolute_path, entt::handle& obj)
{
    // APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ifstream stream(absolute_path, std::ios::binary);
    load_from_stream_bin(stream, obj);
}

auto load_from_prefab(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle
{
    entt::handle obj;

    const auto& prefab = pfb.get();
    const auto& buffer = prefab->buffer.data;

    if(!buffer.empty())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(buffer.data(), buffer.size());

        auto on_create = [&pfb](entt::handle obj)
        {
            if(obj)
            {
                auto& pfb_comp = obj.get_or_emplace<prefab_component>();
                pfb_comp.source = pfb;
            }
        };

        obj = load_from_archive_start(ar, registry, on_create);
    }

    return obj;
}
auto load_from_prefab_bin(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle
{
    entt::handle obj;

    const auto& prefab = pfb.get();
    auto buffer = prefab->buffer.get_stream_buf();
    std::istream stream(&buffer);
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        ser20::iarchive_binary_t ar(stream);

        auto on_create = [&pfb](entt::handle obj)
        {
            if(obj)
            {
                auto& pfb_comp = obj.get_or_emplace<prefab_component>();
                pfb_comp.source = pfb;
            }
        };

        obj = load_from_archive_start(ar, registry, on_create);
    }

    return obj;
}

void clone_entity_from_stream(entt::const_handle src_obj, entt::handle& dst_obj)
{
    // APPLOG_INFO_PERF(std::chrono::microseconds);

    std::stringstream ss;
    save_to_stream(ss, src_obj);

    load_from(ss, dst_obj);
}

void save_to_stream(std::ostream& stream, const scene& scn)
{
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_oarchive_associative(stream);
        save_to_archive(ar, *scn.registry);
    }
}
void save_to_file(const std::string& absolute_path, const scene& scn)
{
    // APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ofstream stream(absolute_path);
    save_to_stream(stream, scn);
}
void save_to_stream_bin(std::ostream& stream, const scene& scn)
{
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        ser20::oarchive_binary_t ar(stream);
        save_to_archive(ar, *scn.registry);
    }
}
void save_to_file_bin(const std::string& absolute_path, const scene& scn)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    save_to_stream_bin(stream, scn);
}

void load_from_view(std::string_view view, scene& scn)
{
    if(!view.empty())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(view.data(), view.size());
        load_from_archive(ar, *scn.registry);
    }
}

void load_from_stream(std::istream& stream, scene& scn)
{
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        stream.seekg(0);

        auto ar = ser20::create_iarchive_associative(stream);
        load_from_archive(ar, *scn.registry);
    }
}
void load_from_file(const std::string& absolute_path, scene& scn)
{
    // APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ifstream stream(absolute_path);
    load_from_stream(stream, scn);
}
void load_from_stream_bin(std::istream& stream, scene& scn)
{
    if(stream.good())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);

        stream.seekg(0);

        ser20::iarchive_binary_t ar(stream);
        load_from_archive(ar, *scn.registry);
    }
}
void load_from_file_bin(const std::string& absolute_path, scene& scn)
{
    // APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ifstream stream(absolute_path, std::ios::binary);
    load_from_stream_bin(stream, scn);
}

auto load_from_prefab(const asset_handle<scene_prefab>& pfb, scene& scn) -> bool
{
    const auto& prefab = pfb.get();
    const auto& buffer = prefab->buffer.data;

    if(!buffer.empty())
    {
        // APPLOG_INFO_PERF(std::chrono::microseconds);
        auto ar = ser20::create_iarchive_associative(buffer.data(), buffer.size());
        load_from_archive(ar, *scn.registry);
    }

    return true;
}
auto load_from_prefab_bin(const asset_handle<scene_prefab>& pfb, scene& scn) -> bool
{
    const auto& prefab = pfb.get();
    auto buffer = prefab->buffer.get_stream_buf();

    // APPLOG_INFO_PERF(std::chrono::microseconds);
    std::istream stream(&buffer);
    if(!stream.good())
    {
        return false;
    }

    load_from_stream_bin(stream, scn);

    return true;
}

void clone_scene_from_stream(const scene& src_scene, scene& dst_scene)
{
    dst_scene.unload();

    auto& src = src_scene.registry;
    auto& dst = dst_scene.registry;

    // APPLOG_INFO_PERF(std::chrono::microseconds);

    src->view<transform_component, root_component>().each(
        [&](auto e, auto&& comp1, auto&& comp2)
        {
            std::stringstream ss;
            save_to_stream(ss, src_scene.create_entity(e));

            auto e_clone = dst_scene.registry->create();
            auto e_clone_obj = dst_scene.create_entity(e_clone);

            load_from(ss, e_clone_obj);
        });
}
} // namespace ace
