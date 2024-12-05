#include "hierarchy_panel.h"
#include "../panel.h"
#include "../panels_defs.h"
#include <imgui/imgui_internal.h>

#include <editor/editing/editing_manager.h>
#include <editor/events.h>

#include <engine/assets/impl/asset_extensions.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/components/id_component.h>
#include <engine/ecs/components/prefab_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/rendering/ecs/components/model_component.h>

#include <filesystem/filesystem.h>

namespace ace
{

namespace
{

struct graph_context
{
    graph_context(rtti::context& context) : ctx(context), em(context.get_cached<editing_manager>()), ec(context.get_cached<ecs>())

    {
    }

    rtti::context& ctx;
    editing_manager& em;
    ecs& ec;
    imgui_panels* panels;
};

bool prev_edit_label{};
bool edit_label_{};

auto update_editing()
{
    prev_edit_label = edit_label_;
}

auto is_just_started_editing_label() -> bool
{
    return edit_label_ && edit_label_ != prev_edit_label;
}

auto is_editing_label() -> bool
{
    return edit_label_;
}

void start_editing_label(graph_context& ctx, entt::handle entity)
{
    ctx.em.select(entity);
    edit_label_ = true;
}

void stop_editing_label(graph_context& ctx, entt::handle entity)
{
    edit_label_ = false;
}

auto get_entity_tag(entt::handle entity) -> const std::string&
{
    auto& tag = entity.get_or_emplace<tag_component>();
    return tag.tag;
}

void set_entity_tag(entt::handle entity, const std::string& name)
{
    auto& tag = entity.get_or_emplace<tag_component>();
    tag.tag = name;
}

bool process_drag_drop_source(graph_context& ctx, entt::handle entity)
{
    if(entity && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::TextUnformatted(get_entity_tag(entity).c_str());
        ImGui::SetDragDropPayload("entity", &entity, sizeof(entity));
        ImGui::EndDragDropSource();
        return true;
    }

    return false;
}

void process_drag_drop_target(graph_context& ctx, entt::handle entity)
{
    if(ImGui::BeginDragDropTarget())
    {
        if(ImGui::IsDragDropPayloadBeingAccepted())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        else
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
        }

        {
            auto payload = ImGui::AcceptDragDropPayload("entity");
            if(payload != nullptr)
            {
                entt::handle dropped{};
                std::memcpy(&dropped, payload->Data, size_t(payload->DataSize));
                if(dropped)
                {
                    auto trans_comp = dropped.try_get<transform_component>();
                    if(trans_comp)
                    {
                        trans_comp->set_parent(entity);
                    }
                }
            }
        }

        for(const auto& type : ex::get_suported_formats<mesh>())
        {
            auto payload = ImGui::AcceptDragDropPayload(type.c_str());
            if(payload != nullptr)
            {
                std::string absolute_path(reinterpret_cast<const char*>(payload->Data), std::size_t(payload->DataSize));

                std::string key = fs::convert_to_protocol(fs::path(absolute_path)).generic_string();

                auto& em = ctx.em;
                auto& ec = ctx.ec;

                auto object = defaults::create_mesh_entity_at(ctx.ctx, ec.get_scene(), key);

                em.select(object);
            }
        }

        for(const auto& type : ex::get_suported_formats<prefab>())
        {
            auto payload = ImGui::AcceptDragDropPayload(type.c_str());
            if(payload != nullptr)
            {
                std::string absolute_path(reinterpret_cast<const char*>(payload->Data), std::size_t(payload->DataSize));

                std::string key = fs::convert_to_protocol(fs::path(absolute_path)).generic_string();

                auto& em = ctx.em;
                auto& ec = ctx.ec;

                auto object = defaults::create_prefab_at(ctx.ctx, ec.get_scene(), key);

                em.select(object);
            }
        }

        ImGui::EndDragDropTarget();
    }
}

void check_drag(graph_context& ctx, entt::handle entity)
{
    if(!process_drag_drop_source(ctx, entity))
    {
        process_drag_drop_target(ctx, entity);
    }
}

void check_context_menu(graph_context& ctx, entt::handle entity)
{
    ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetStyleColorVec4(ImGuiCol_Text));

    auto common_menu_items = [&]()
    {
        if(ImGui::MenuItem("Create Empty"))
        {
            ctx.panels->get_scene_panel().add_action(
                [ctx, entity]() mutable
                {
                    auto new_entity = ctx.ec.get_scene().create_entity({}, entity);
                    start_editing_label(ctx, new_entity);
                });
        }

        if(ImGui::BeginMenu("3D Objects"))
        {
            static const std::vector<std::pair<std::string, std::vector<std::string>>> menu_objects = {

                {"Cube", {"Cube"}},
                {"Sphere", {"Sphere"}},
                {"Plane", {"Plane"}},
                {"Cylinder", {"Cylinder"}},
                {"Capsule", {"Capsule"}},
                {"Cone", {"Cone"}},
                {"Torus", {"Torus"}},
                {"Teapot", {"Teapot"}},
                {"Separator", {}},
                {"Polygon", {"Icosahedron", "Dodecahedron"}},
                {"Icosphere", {"Icosphere0",  "Icosphere1",  "Icosphere2",  "Icosphere3",  "Icosphere4",
                               "Icosphere5",  "Icosphere6",  "Icosphere7",  "Icosphere8",  "Icosphere9",
                               "Icosphere10", "Icosphere11", "Icosphere12", "Icosphere13", "Icosphere14",
                               "Icosphere15", "Icosphere16", "Icosphere17", "Icosphere18", "Icosphere19"}}};

            for(const auto& p : menu_objects)
            {
                const auto& name = p.first;
                const auto& objects_name = p.second;

                if(name == "Separator")
                {
                    ImGui::Separator();
                }
                else if(name == "New Line")
                {
                    ImGui::NextLine();
                }
                else if(objects_name.size() == 1)
                {
                    if(ImGui::MenuItem(name.c_str()))
                    {
                        auto object = defaults::create_embedded_mesh_entity(ctx.ctx, ctx.ec.get_scene(), name);

                        if(object)
                        {
                            object.get<transform_component>().set_parent(entity, false);
                        }
                        ctx.em.select(object);

                        start_editing_label(ctx, object);

                    }
                }
                else
                {
                    if(ImGui::BeginMenu(name.c_str()))
                    {
                        for(const auto& n : objects_name)
                        {
                            if(ImGui::MenuItem(n.c_str()))
                            {
                                auto object = defaults::create_embedded_mesh_entity(ctx.ctx, ctx.ec.get_scene(), n);
                                if(object)
                                {
                                    object.get<transform_component>().set_parent(entity, false);
                                }

                                ctx.em.select(object);

                                start_editing_label(ctx, object);
                            }
                        }
                        ImGui::EndMenu();
                    }
                }
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Lighting"))
        {
            if(ImGui::BeginMenu("Light"))
            {
                static const std::vector<std::pair<std::string, light_type>> light_objects = {
                    {"Directional", light_type::directional},
                    {"Spot", light_type::spot},
                    {"Point", light_type::point}};

                for(const auto& p : light_objects)
                {
                    const auto& name = p.first;
                    const auto& type = p.second;
                    if(ImGui::MenuItem(name.c_str()))
                    {
                        auto object = defaults::create_light_entity(ctx.ctx, ctx.ec.get_scene(), type, name);
                        if(object)
                        {
                            object.get<transform_component>().set_parent(entity, false);
                        }
                        ctx.em.select(object);

                        start_editing_label(ctx, object);
                    }
                }
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Reflection Probes"))
            {
                static const std::vector<std::pair<std::string, probe_type>> reflection_probes = {
                    {"Sphere", probe_type::sphere},
                    {"Box", probe_type::box}};
                for(const auto& p : reflection_probes)
                {
                    const auto& name = p.first;
                    const auto& type = p.second;

                    if(ImGui::MenuItem(name.c_str()))
                    {
                        auto object = defaults::create_reflection_probe_entity(ctx.ctx, ctx.ec.get_scene(), type, name);
                        if(object)
                        {
                            object.get<transform_component>().set_parent(entity, false);
                        }
                        ctx.em.select(object);

                        start_editing_label(ctx, object);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        //            if(ImGui::BeginMenu("AUDIO"))
        //            {
        //                if(ImGui::MenuItem("SOURCE"))
        //                {
        //                    auto object = ecs.create();
        //                    object.set_name("AUDIO SOURCE");
        //                    object.assign<transform_component>();
        //                    object.assign<audio_source_component>();
        //                    es.select(object);
        //                }
        //                ImGui::EndMenu();
        //            }

        if(ImGui::MenuItem("Camera"))
        {
            auto object = defaults::create_camera_entity(ctx.ctx, ctx.ec.get_scene(), "Camera");
            ctx.em.select(object);

            start_editing_label(ctx, object);
        }
    };

    if(entity)
    {
        if(ImGui::BeginPopupContextItem("Entity Context Menu"))
        {
            if(ImGui::MenuItem("Create Empty Parent"))
            {
                ctx.panels->get_scene_panel().add_action(
                    [ctx, entity]() mutable
                    {
                        auto current_parent = entity.get<transform_component>().get_parent();

                        auto new_entity = ctx.ec.get_scene().create_entity({}, current_parent);
                        entity.get<transform_component>().set_parent(new_entity);

                        start_editing_label(ctx, new_entity);
                    });
            }

            common_menu_items();

            ImGui::Separator();

            if(ImGui::MenuItem("Rename", ImGui::GetKeyName(ctx.panels->get_scene_panel().edit_key)))
            {
                ctx.panels->get_scene_panel().add_action(
                    [ctx, entity]() mutable
                    {
                        start_editing_label(ctx, entity);
                    });
            }

            if(ImGui::MenuItem(
                   "Duplicate",
                   ImGui::GetKeyCombinationName(ctx.panels->get_scene_panel().duplicate_combination).c_str()))
            {
                ctx.panels->get_scene_panel().duplicate_entity(entity);
            }

            if(ImGui::MenuItem("Delete", ImGui::GetKeyName(ctx.panels->get_scene_panel().delete_key)))
            {
                ctx.panels->get_scene_panel().delete_entity(entity);
            }

            if(ImGui::MenuItem("Focus", ImGui::GetKeyName(ctx.panels->get_scene_panel().focus_key)))
            {
                ctx.panels->get_scene_panel().focus_entity(ctx.panels->get_scene_panel().get_camera(), entity);
            }

            ImGui::EndPopup();
        }
    }
    else
    {
        if(ImGui::BeginPopupContextWindowEx())
        {
            common_menu_items();
            ImGui::EndPopup();
        }


    }

    ImGui::PopStyleColor();
}

void draw_entity(graph_context& ctx, entt::handle entity)
{
    if(!entity)
    {
        return;
    }

    const auto& name = get_entity_tag(entity);
    ImGui::PushID(static_cast<int>(entity.entity()));

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_OpenOnArrow;

    if(ctx.em.is_selected(entity))
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    auto& trans_comp = entity.get<transform_component>();
    bool no_children = trans_comp.get_children().empty();

    if(no_children)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    auto pos = ImGui::GetCursorScreenPos() + ImVec2(ImGui::GetTextLineHeightWithSpacing(), 0.0f);
    ImGui::AlignTextToFramePadding();

    bool is_bone = entity.all_of<bone_component>();
    bool is_submesh = entity.all_of<submesh_component>();

    bool has_source = entity.all_of<prefab_component>();
    auto icon = has_source ? ICON_MDI_CUBE " " : ICON_MDI_CUBE_OUTLINE " ";
    if(is_bone)
    {
        icon = ICON_MDI_BONE " ";
    }
    auto label = icon + name + "###" + std::to_string(static_cast<int>(entity.entity()));

    auto col = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    col = ImLerp(col, ImVec4(0.5f, 0.85f, 1.0f, 1.0f), float(has_source) * 0.5f);
    col = ImLerp(col, ImVec4(0.5f, 0.85f, 1.0f, 1.0f), float(is_bone) * 0.5f);
    col = ImLerp(col, ImVec4(0.8f, 0.4f, 0.4f, 1.0f), float(is_submesh) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Text, col);
    bool opened = ImGui::TreeNodeEx(label.c_str(), flags);
    ImGui::PopStyleColor();


    if(ctx.em.is_focused(entity))
    {
        ImGui::SetItemFocusFrame(ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.0f, 1.0f)));
    }

    if(ImGui::IsItemReleased(ImGuiMouseButton_Left))
    {
        ctx.panels->get_scene_panel().add_action(
            [ctx, entity]() mutable
            {
                stop_editing_label(ctx, entity);
                ctx.em.select(entity);
            });
    }

    if(ctx.em.is_selected(entity))
    {
        if(ImGui::IsItemClicked(ImGuiMouseButton_Middle))
        {
            ctx.panels->get_scene_panel().focus_entity(ctx.panels->get_scene_panel().get_camera(), entity);
        }

        if(ImGui::IsItemDoubleClicked(ImGuiMouseButton_Left))
        {
            ctx.panels->get_scene_panel().add_action(
                [ctx, entity]() mutable
                {
                    start_editing_label(ctx, entity);
                });
        }

        if(ImGui::IsItemKeyPressed(ctx.panels->get_scene_panel().edit_key))
        {
            ctx.panels->get_scene_panel().add_action(
                [ctx, entity]() mutable
                {
                    start_editing_label(ctx, entity);
                });
        }

        if(ImGui::IsItemKeyPressed(ctx.panels->get_scene_panel().delete_key))
        {
            ctx.panels->get_scene_panel().delete_entity(entity);
        }

        if(ImGui::IsItemKeyPressed(ctx.panels->get_scene_panel().focus_key))
        {
            ctx.panels->get_scene_panel().focus_entity(ctx.panels->get_scene_panel().get_camera(), entity);
        }

        if(ImGui::IsItemCombinationKeyPressed(ctx.panels->get_scene_panel().duplicate_combination))
        {
            ctx.panels->get_scene_panel().duplicate_entity(entity);
        }
    }

    if(!is_editing_label())
    {
        check_drag(ctx, entity);

        check_context_menu(ctx, entity);
    }

    if(ctx.em.is_selected(entity) && is_editing_label())
    {
        if(is_just_started_editing_label())
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

        auto edit_name = name;
        ImGui::InputTextWidget("##rename", edit_name, false, ImGuiInputTextFlags_AutoSelectAll);
        if(ImGui::IsItemDeactivatedAfterEdit())
        {
            set_entity_tag(entity, edit_name);
            stop_editing_label(ctx, entity);
        }

        ImGui::PopItemWidth();

        if(ImGui::IsItemDeactivated())
        {
            stop_editing_label(ctx, entity);
        }
    }

    if(opened)
    {
        if(!no_children)
        {
            const auto& children = trans_comp.get_children();
            for(auto& child : children)
            {
                if(child)
                {
                    draw_entity(ctx, child);
                }
            }
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}
} // namespace

hierarchy_panel::hierarchy_panel(imgui_panels* parent) : entity_panel(parent)
{
}

void hierarchy_panel::init(rtti::context& ctx)
{
}

void hierarchy_panel::on_frame_ui_render(rtti::context& ctx, const char* name)
{
    entity_panel::on_frame_ui_render();

    if(ImGui::Begin(name))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));

        execute_actions();

        graph_context gctx(ctx);
        gctx.panels = parent_;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoSavedSettings;

        if(ImGui::BeginChild("hierarchy_content", ImGui::GetContentRegionAvail(), 0, flags))
        {
            check_context_menu(gctx, {});

            auto& scene = gctx.ec.get_scene();

            std::string name = fs::path(scene.source.id()).stem().string();

            if(name.empty())
            {
                name = "Unnamed";
            }
            ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
            if(ImGui::CollapsingHeader(name.c_str()))
            {
                scene.registry->view<transform_component, root_component>().each(
                    [&](auto e, auto&& comp, auto&& tag)
                    {
                        draw_entity(gctx, comp.get_owner());
                    });
            }


        }
        ImGui::EndChild();
        check_drag(gctx, {});
    }
    ImGui::End();

    update_editing();
}

} // namespace ace
