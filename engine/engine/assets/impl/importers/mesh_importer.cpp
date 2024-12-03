#include "mesh_importer.h"

#include <graphics/graphics.h>
#include <logging/logging.h>
#include <math/math.h>
#include <string_utils/utils.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/GltfMaterial.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <bx/file.h>
#include <graphics/utils/bgfx_utils.h>

#include <algorithm>
#include <execution>
#include <filesystem/filesystem.h>

namespace ace
{
namespace importer
{
namespace
{

// Helper function to interpolate between two keyframes for position
auto interpolate_position(float animation_time, const aiNodeAnim* node_anim) -> aiVector3D
{
    if(node_anim->mNumPositionKeys == 1)
    {
        return node_anim->mPositionKeys[0].mValue;
    }

    for(unsigned int i = 0; i < node_anim->mNumPositionKeys - 1; ++i)
    {
        if(animation_time < (float)node_anim->mPositionKeys[i + 1].mTime)
        {
            float time1 = (float)node_anim->mPositionKeys[i].mTime;
            float time2 = (float)node_anim->mPositionKeys[i + 1].mTime;
            float factor = (animation_time - time1) / (time2 - time1);
            const aiVector3D& start = node_anim->mPositionKeys[i].mValue;
            const aiVector3D& end = node_anim->mPositionKeys[i + 1].mValue;
            aiVector3D delta = end - start;
            return start + factor * delta;
        }
    }
    return node_anim->mPositionKeys[0].mValue; // Default to first position
}

// Helper function to interpolate between two keyframes for rotation
auto interpolate_rotation(float animation_time, const aiNodeAnim* node_anim) -> aiQuaternion
{
    if(node_anim->mNumRotationKeys == 1)
    {
        return node_anim->mRotationKeys[0].mValue;
    }

    for(unsigned int i = 0; i < node_anim->mNumRotationKeys - 1; ++i)
    {
        if(animation_time < (float)node_anim->mRotationKeys[i + 1].mTime)
        {
            float time1 = (float)node_anim->mRotationKeys[i].mTime;
            float time2 = (float)node_anim->mRotationKeys[i + 1].mTime;
            float factor = (animation_time - time1) / (time2 - time1);
            const aiQuaternion& start = node_anim->mRotationKeys[i].mValue;
            const aiQuaternion& end = node_anim->mRotationKeys[i + 1].mValue;
            aiQuaternion result;
            aiQuaternion::Interpolate(result, start, end, factor);
            return result.Normalize();
        }
    }
    return node_anim->mRotationKeys[0].mValue; // Default to first rotation
}

// Helper function to interpolate between two keyframes for scaling
auto interpolate_scaling(float animation_time, const aiNodeAnim* node_anim) -> aiVector3D
{
    if(node_anim->mNumScalingKeys == 1)
    {
        return node_anim->mScalingKeys[0].mValue;
    }

    for(unsigned int i = 0; i < node_anim->mNumScalingKeys - 1; ++i)
    {
        if(animation_time < (float)node_anim->mScalingKeys[i + 1].mTime)
        {
            float time1 = (float)node_anim->mScalingKeys[i].mTime;
            float time2 = (float)node_anim->mScalingKeys[i + 1].mTime;
            float factor = (animation_time - time1) / (time2 - time1);
            const aiVector3D& start = node_anim->mScalingKeys[i].mValue;
            const aiVector3D& end = node_anim->mScalingKeys[i + 1].mValue;
            aiVector3D delta = end - start;
            return start + factor * delta;
        }
    }
    return node_anim->mScalingKeys[0].mValue; // Default to first scaling
}

// Find the animation channel that matches the node name (bone)
auto find_node_anim(const aiAnimation* animation, const aiString& node_name) -> const aiNodeAnim*
{
    for(unsigned int i = 0; i < animation->mNumChannels; ++i)
    {
        const aiNodeAnim* node_anim = animation->mChannels[i];
        if(std::string(node_anim->mNodeName.C_Str()) == node_name.C_Str())
        {
            return node_anim;
        }
    }
    return nullptr;
}

// Recursively calculate the bone transform for the current node (bone)
auto calculate_bone_transform(const aiNode* node,
                              const aiString& bone_name,
                              const aiAnimation* animation,
                              float animation_time,
                              const aiMatrix4x4& parent_transform) -> aiMatrix4x4
{
    std::string node_name(node->mName.C_Str());

    // Find the corresponding animation channel for this bone/node
    const aiNodeAnim* node_anim = find_node_anim(animation, node->mName);

    // Local transformation matrix
    aiMatrix4x4 local_transform = node->mTransformation;

    // If we have animation data for this node, interpolate the transformation
    if(node_anim)
    {
        // Interpolate translation, rotation, and scaling
        aiVector3D interpolated_position = interpolate_position(animation_time, node_anim);
        aiQuaternion interpolated_rotation = interpolate_rotation(animation_time, node_anim);
        aiVector3D interpolated_scaling = interpolate_scaling(animation_time, node_anim);

        // Build the transformation matrix from interpolated values
        aiMatrix4x4 position_matrix;
        aiMatrix4x4::Translation(interpolated_position, position_matrix);

        aiMatrix4x4 rotation_matrix = aiMatrix4x4(interpolated_rotation.GetMatrix());

        aiMatrix4x4 scaling_matrix;
        aiMatrix4x4::Scaling(interpolated_scaling, scaling_matrix);

        // Combine them into a single local transformation matrix
        local_transform = position_matrix * rotation_matrix * scaling_matrix;
    }

    // Combine with parent transformation
    aiMatrix4x4 global_transform = parent_transform * local_transform;

    // If this node is the bone we're looking for, return the global transformation
    if(node_name == bone_name.C_Str())
    {
        return global_transform;
    }

    // Recursively calculate the bone transform for all child nodes
    for(unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        auto child_transform =
            calculate_bone_transform(node->mChildren[i], bone_name, animation, animation_time, global_transform);
        if(child_transform != aiMatrix4x4())
        {
            return child_transform;
        }
    }

    // If not found, return identity matrix
    return aiMatrix4x4();
}

using animation_bounding_box_map = std::unordered_map<const aiAnimation*, std::vector<math::bbox>>;

auto transform_point(const aiMatrix4x4& transform, const aiVector3D& point) -> math::vec3
{
    aiVector3D transformed_point = transform * point;
    return math::vec3(transformed_point.x, transformed_point.y, transformed_point.z);
}

auto get_transformed_vertices(const aiMesh* mesh,
                              const aiScene* scene,
                              float time_in_seconds,
                              const aiAnimation* animation) -> std::vector<math::vec3>
{
    std::vector<math::vec3> transformed_vertices(mesh->mNumVertices, math::vec3(0.0f));

    // Iterate over bones in the mesh using parallel execution
    std::for_each(
        // std::execution::par,
        mesh->mBones,
        mesh->mBones + mesh->mNumBones,
        [&](const aiBone* bone)
        {
            aiMatrix4x4 bone_offset = bone->mOffsetMatrix;

            // Calculate or retrieve the cached bone transformation for this frame
            aiMatrix4x4 bone_transform =
                calculate_bone_transform(scene->mRootNode, bone->mName, animation, time_in_seconds, aiMatrix4x4());

            // Apply the bone transformation to vertices influenced by this bone
            std::for_each(bone->mWeights,
                          bone->mWeights + bone->mNumWeights,
                          [&](const aiVertexWeight& weight)
                          {
                              unsigned int vertex_id = weight.mVertexId;
                              float weight_value = weight.mWeight;

                              aiVector3D position = mesh->mVertices[vertex_id];
                              math::vec3 transformed_pos = transform_point(bone_transform * bone_offset, position);

                              // Accumulate the influence of this bone for each vertex
                              transformed_vertices[vertex_id] += transformed_pos * weight_value;
                          });
        });

    return transformed_vertices;
}

// Calculate the bounding box in parallel
auto calculate_bounding_box(const std::vector<math::vec3>& vertices) -> math::bbox
{
    math::bbox box;

    // Use parallel execution to find the min/max extents of the bounding box
    std::for_each(vertices.begin(),
                  vertices.end(),
                  [&](const math::vec3& vertex)
                  {
                      box.add_point(vertex);
                  });

    return box;
}

// Recursive function to propagate bone influence to child nodes
void propagate_bone_influence(const aiNode* node, std::unordered_set<std::string>& affected_bones)
{
    // Mark this node as affected
    affected_bones.insert(node->mName.C_Str());

    // Recursively propagate to all child nodes
    for(unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        propagate_bone_influence(node->mChildren[i], affected_bones);
    }
}

// Helper function to collect directly and indirectly affected bones (nodes) by the animation
auto get_affected_bones_and_children(const aiScene* scene, const aiAnimation* animation)
    -> std::unordered_set<std::string>
{
    std::unordered_set<std::string> affected_bones;

    // Step 1: Collect directly affected bones (from animation channels)
    for(unsigned int i = 0; i < animation->mNumChannels; ++i)
    {
        const aiNodeAnim* node_anim = animation->mChannels[i];
        affected_bones.insert(node_anim->mNodeName.C_Str());

        // Step 2: Find the corresponding node in the scene and propagate influence to its children
        const aiNode* affected_node = scene->mRootNode->FindNode(node_anim->mNodeName);
        if(affected_node)
        {
            propagate_bone_influence(affected_node, affected_bones); // Recursively mark all children
        }
    }

    return affected_bones;
}

// Function to check if a mesh is affected by the animation (directly or indirectly)
auto is_mesh_affected_by_animation(const aiMesh* mesh, const std::unordered_set<std::string>& affected_bones) -> bool
{
    for(unsigned int i = 0; i < mesh->mNumBones; ++i)
    {
        if(affected_bones.find(mesh->mBones[i]->mName.C_Str()) != affected_bones.end())
        {
            return true; // This mesh is influenced by at least one bone affected by the animation
        }
    }
    return false; // No bones from this mesh are affected by the animation
}

auto get_affected_meshes(const aiScene* scene,
                         const aiAnimation* animation,
                         const std::unordered_set<std::string>& affected_bones)
{
    std::vector<const aiMesh*> affected_meshes;
    for(unsigned int mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index)
    {
        const aiMesh* mesh = scene->mMeshes[mesh_index];

        // Skip the mesh if it is not affected by the animation
        if(is_mesh_affected_by_animation(mesh, affected_bones))
        {
            affected_meshes.emplace_back(mesh);
        }
    }

    return affected_meshes;
}

// Main function to compute bounding boxes for animations, skipping unaffected meshes
auto compute_bounding_boxes_for_animations(const aiScene* scene, float sample_interval = 0.2f)
    -> animation_bounding_box_map
{
    APPLOG_TRACE_PERF(std::chrono::seconds);

    animation_bounding_box_map animation_bounding_boxes;

    if(!scene->HasAnimations())
    {
        return animation_bounding_boxes;
    }

    float total_steps = 0;
    for(unsigned int anim_index = 0; anim_index < scene->mNumAnimations; ++anim_index)
    {
        const aiAnimation* animation = scene->mAnimations[anim_index];

        animation_bounding_boxes[animation].clear();

        float animation_duration = (float)animation->mDuration;
        float ticks_per_second = (animation->mTicksPerSecond != 0.0f) ? (float)animation->mTicksPerSecond : 25.0f;
        float steps = animation_duration / (sample_interval * ticks_per_second);
        total_steps += steps;
    }

    std::atomic<size_t> current_steps = 0;

    std::for_each(
        // std::execution::par,
        scene->mAnimations,
        scene->mAnimations + scene->mNumAnimations,
        [&](const aiAnimation* animation)
        {
            float animation_duration = (float)animation->mDuration;
            float ticks_per_second = (animation->mTicksPerSecond != 0.0f) ? (float)animation->mTicksPerSecond : 25.0f;
            float steps = animation_duration / (sample_interval * ticks_per_second);

            auto& boxes = animation_bounding_boxes[animation];
            boxes.reserve(size_t(steps));

            // Collect the bones affected by the animation (both direct and indirect)
            auto affected_bones = get_affected_bones_and_children(scene, animation);
            auto affected_meshes = get_affected_meshes(scene, animation, affected_bones);
            // For each keyframe (or sample the animation at regular intervals)
            for(float time = 0.0f; time <= animation_duration; time += (sample_interval * ticks_per_second))
            {
                float percent = (float(current_steps) / total_steps) * 100.0f;

                for(const auto& mesh : affected_meshes)
                {
                    // Get transformed vertices for this time/frame
                    auto transformed_vertices = get_transformed_vertices(mesh, scene, time, animation);

                    // Compute the bounding box for this frame
                    auto frame_bounding_box = calculate_bounding_box(transformed_vertices);

                    // Inflate the box by some margin to account for skipped frames
                    frame_bounding_box.inflate(frame_bounding_box.get_extents() * 0.05f);

                    // Store the bounding box (for later use)
                    boxes.push_back(frame_bounding_box);
                }

                // APPLOG_TRACE("Mesh Importer : Animation precompute bounding box progress {:.2f}%", percent);
                current_steps++;
            }
        });

    return animation_bounding_boxes;
}

// Helper function to get the file extension from the compressed texture format

auto get_texture_extension_from_texture(const aiTexture* texture) -> std::string
{
    if(texture->achFormatHint[0] != '\0')
    {
        return std::string(".") + texture->achFormatHint;
    }
    return ".tga"; // Fallback extension raw
}

auto get_texture_extension(const aiTexture* texture) -> std::string
{
    auto extension = get_texture_extension_from_texture(texture);

    if(extension == ".jpg" || extension == ".jpeg")
    {
        extension = ".dds";
    }

    return extension;
}

auto get_embedded_texture_name(const aiTexture* texture,
                               size_t index,
                               const fs::path& filename,
                               const std::string& semantic) -> std::string
{
    return fmt::format("[{}] {} {}{}", index, semantic, filename.string(), get_texture_extension(texture));
}

auto process_matrix(const aiMatrix4x4& assimp_matrix) -> math::mat4
{
    math::mat4 matrix;

    matrix[0][0] = assimp_matrix.a1;
    matrix[1][0] = assimp_matrix.a2;
    matrix[2][0] = assimp_matrix.a3;
    matrix[3][0] = assimp_matrix.a4;

    matrix[0][1] = assimp_matrix.b1;
    matrix[1][1] = assimp_matrix.b2;
    matrix[2][1] = assimp_matrix.b3;
    matrix[3][1] = assimp_matrix.b4;

    matrix[0][2] = assimp_matrix.c1;
    matrix[1][2] = assimp_matrix.c2;
    matrix[2][2] = assimp_matrix.c3;
    matrix[3][2] = assimp_matrix.c4;

    matrix[0][3] = assimp_matrix.d1;
    matrix[1][3] = assimp_matrix.d2;
    matrix[2][3] = assimp_matrix.d3;
    matrix[3][3] = assimp_matrix.d4;

    return matrix;
}

void process_vertices(aiMesh* mesh, mesh::load_data& load_data)
{
    auto& submesh = load_data.submeshes.back();

    // Determine the correct offset to any relevant elements in the vertex
    bool has_position = load_data.vertex_format.has(gfx::attribute::Position);
    bool has_normal = load_data.vertex_format.has(gfx::attribute::Normal);
    bool has_bitangent = load_data.vertex_format.has(gfx::attribute::Bitangent);
    bool has_tangent = load_data.vertex_format.has(gfx::attribute::Tangent);
    bool has_texcoord0 = load_data.vertex_format.has(gfx::attribute::TexCoord0);
    auto vertex_stride = load_data.vertex_format.getStride();

    std::uint32_t current_vertex = load_data.vertex_count;
    load_data.vertex_count += mesh->mNumVertices;
    load_data.vertex_data.resize(load_data.vertex_count * vertex_stride);

    std::uint8_t* current_vertex_ptr = load_data.vertex_data.data() + current_vertex * vertex_stride;

    for(size_t i = 0; i < mesh->mNumVertices; ++i, current_vertex_ptr += vertex_stride)
    {
        // position
        if(mesh->HasPositions() && has_position)
        {
            float position[4];
            std::memcpy(position, &mesh->mVertices[i], sizeof(aiVector3D));

            gfx::vertex_pack(position, false, gfx::attribute::Position, load_data.vertex_format, current_vertex_ptr);

            submesh.bbox.add_point(math::vec3(position[0], position[1], position[2]));
        }

        // tex coords
        if(mesh->HasTextureCoords(0) && has_texcoord0)
        {
            float textureCoords[4];
            std::memcpy(textureCoords, &mesh->mTextureCoords[0][i], sizeof(aiVector2D));

            gfx::vertex_pack(textureCoords,
                             true,
                             gfx::attribute::TexCoord0,
                             load_data.vertex_format,
                             current_vertex_ptr);
        }

        ////normals
        math::vec4 normal;
        if(mesh->HasNormals() && has_normal)
        {
            std::memcpy(math::value_ptr(normal), &mesh->mNormals[i], sizeof(aiVector3D));

            gfx::vertex_pack(math::value_ptr(normal),
                             true,
                             gfx::attribute::Normal,
                             load_data.vertex_format,
                             current_vertex_ptr);
        }

        math::vec4 tangent;
        // tangents
        if(mesh->HasTangentsAndBitangents() && has_tangent)
        {
            std::memcpy(math::value_ptr(tangent), &mesh->mTangents[i], sizeof(aiVector3D));
            tangent.w = 1.0f;

            gfx::vertex_pack(math::value_ptr(tangent),
                             true,
                             gfx::attribute::Tangent,
                             load_data.vertex_format,
                             current_vertex_ptr);
        }

        // binormals
        math::vec4 bitangent;
        if(mesh->HasTangentsAndBitangents() && has_bitangent)
        {
            std::memcpy(math::value_ptr(bitangent), &mesh->mBitangents[i], sizeof(aiVector3D));
            float handedness =
                math::dot(math::vec3(bitangent), math::normalize(math::cross(math::vec3(normal), math::vec3(tangent))));
            tangent.w = handedness;

            gfx::vertex_pack(math::value_ptr(bitangent),
                             true,
                             gfx::attribute::Bitangent,
                             load_data.vertex_format,
                             current_vertex_ptr);
        }
    }
}

void process_faces(aiMesh* mesh, std::uint32_t submesh_offset, mesh::load_data& load_data)
{
    load_data.triangle_count += mesh->mNumFaces;

    load_data.triangle_data.reserve(load_data.triangle_data.size() + mesh->mNumFaces);

    for(size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];

        auto& triangle = load_data.triangle_data.emplace_back();
        triangle.data_group_id = mesh->mMaterialIndex;

        auto num_indices = std::min<size_t>(face.mNumIndices, 3);
        for(size_t j = 0; j < num_indices; ++j)
        {
            triangle.indices[j] = face.mIndices[j] + submesh_offset;
        }
    }
}

void process_bones(aiMesh* mesh, std::uint32_t submesh_offset, mesh::load_data& load_data)
{
    if(mesh->HasBones())
    {
        auto& bone_influences = load_data.skin_data.get_bones();

        for(size_t i = 0; i < mesh->mNumBones; ++i)
        {
            aiBone* assimp_bone = mesh->mBones[i];
            const std::string bone_name = assimp_bone->mName.C_Str();

            auto it = std::find_if(std::begin(bone_influences),
                                   std::end(bone_influences),
                                   [&bone_name](const auto& bone)
                                   {
                                       return bone_name == bone.bone_id;
                                   });

            skin_bind_data::bone_influence* bone_ptr = nullptr;
            if(it != std::end(bone_influences))
            {
                bone_ptr = &(*it);
            }
            else
            {
                const auto& assimp_matrix = assimp_bone->mOffsetMatrix;
                skin_bind_data::bone_influence bone_influence;
                bone_influence.bone_id = bone_name;
                bone_influence.bind_pose_transform = process_matrix(assimp_matrix);
                bone_influences.emplace_back(std::move(bone_influence));
                bone_ptr = &bone_influences.back();
            }

            if(bone_ptr == nullptr)
            {
                continue;
            }

            for(size_t j = 0; j < assimp_bone->mNumWeights; ++j)
            {
                aiVertexWeight assimp_influence = assimp_bone->mWeights[j];

                skin_bind_data::vertex_influence influence;
                influence.vertex_index = assimp_influence.mVertexId + submesh_offset;
                influence.weight = assimp_influence.mWeight;

                bone_ptr->influences.emplace_back(influence);
            }
        }
    }
}

void process_mesh(aiMesh* mesh, mesh::load_data& load_data)
{
    load_data.submeshes.emplace_back();
    auto& submesh = load_data.submeshes.back();
    submesh.vertex_start = load_data.vertex_count;
    submesh.vertex_count = mesh->mNumVertices;
    submesh.face_start = load_data.triangle_count;
    submesh.face_count = mesh->mNumFaces;
    submesh.data_group_id = mesh->mMaterialIndex;
    submesh.skinned = mesh->HasBones();
    load_data.material_count = std::max(load_data.material_count, submesh.data_group_id + 1);

    process_faces(mesh, submesh.vertex_start, load_data);
    process_bones(mesh, submesh.vertex_start, load_data);
    process_vertices(mesh, load_data);
}

void process_meshes(const aiScene* scene, mesh::load_data& load_data)
{
    for(size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[i];
        process_mesh(mesh, load_data);
    }
}

void process_node(const aiScene* scene,
                  mesh::load_data& load_data,
                  const aiNode* node,
                  const std::unique_ptr<mesh::armature_node>& armature_node,
                  const math::transform& parent_transform)
{
    armature_node->name = node->mName.C_Str();
    armature_node->local_transform = process_matrix(node->mTransformation);
    armature_node->children.resize(node->mNumChildren);

    auto resolved_transform = parent_transform * armature_node->local_transform;
    for(uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        uint32_t submesh_index = node->mMeshes[i];
        armature_node->submeshes.emplace_back(submesh_index);

        auto& submesh = load_data.submeshes[submesh_index];
        submesh.node_id = node->mName.C_Str();

        auto transformed_bbox = math::bbox::mul(submesh.bbox, resolved_transform);
        load_data.bbox.add_point(transformed_bbox.min);
        load_data.bbox.add_point(transformed_bbox.max);
    }

    for(size_t i = 0; i < node->mNumChildren; ++i)
    {
        armature_node->children[i] = std::make_unique<mesh::armature_node>();
        process_node(scene, load_data, node->mChildren[i], armature_node->children[i], resolved_transform);
    }
}

void process_nodes(const aiScene* scene, mesh::load_data& load_data)
{
    size_t index = 0;
    if(scene->mRootNode != nullptr)
    {
        load_data.bbox = {};
        load_data.root_node = std::make_unique<mesh::armature_node>();
        process_node(scene, load_data, scene->mRootNode, load_data.root_node, math::transform::identity());

        auto get_axis = [&](const std::string& name, math::vec3 fallback)
        {
            if(!scene->mMetaData)
            {
                return fallback;
            }

            int axis = 0;
            if(!scene->mMetaData->Get<int>(name, axis))
            {
                return fallback;
            }
            int axis_sign = 1;
            if(!scene->mMetaData->Get<int>(name + "Sign", axis_sign))
            {
                return fallback;
            }
            math::vec3 result{0.0f, 0.0f, 0.0f};

            if(axis < 0 || axis >= 3)
            {
                return fallback;
            }

            result[axis] = float(axis_sign);

            return result;
        };
        auto x_axis = get_axis("CoordAxis", {1.0f, 0.0f, 0.0f});
        auto y_axis = get_axis("UpAxis", {0.0f, 1.0f, 0.0f});
        auto z_axis = get_axis("FrontAxis", {0.0f, 0.0f, 1.0f});
        // load_data.root_node->local_transform.set_rotation(x_axis, y_axis, z_axis);
    }
}

void dfs_assign_indices(const aiNode* node,
                        std::unordered_map<std::string, unsigned int>& node_indices,
                        unsigned int& current_index)
{
    // Assign the current index to this node
    node_indices[node->mName.C_Str()] = current_index;

    // Increment the index for the next node
    current_index++;

    // Recursively visit all children (DFS)
    for(unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        dfs_assign_indices(node->mChildren[i], node_indices, current_index);
    }
}

auto assign_node_indices(const aiScene* scene) -> std::unordered_map<std::string, unsigned int>
{
    std::unordered_map<std::string, unsigned int> node_indices;
    unsigned int current_index = 0;

    // Start DFS traversal from the root node
    if(scene->mRootNode)
    {
        dfs_assign_indices(scene->mRootNode, node_indices, current_index);
    }

    return node_indices;
}

auto is_node_a_bone(const std::string& node_name, const aiScene* scene) -> bool
{
    for(unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        for(unsigned int j = 0; j < mesh->mNumBones; ++j)
        {
            if(mesh->mBones[j]->mName.C_Str() == node_name)
            {
                return true;
            }
        }
    }
    return false;
}

auto is_node_a_parent_of_bone(const std::string& node_name, const aiScene* scene) -> bool
{
    for(unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        for(unsigned int j = 0; j < mesh->mNumBones; ++j)
        {
            const aiNode* bone_node = scene->mRootNode->FindNode(mesh->mBones[j]->mName);
            const aiNode* current_node = bone_node;

            while(current_node != nullptr)
            {
                if(current_node->mName.C_Str() == node_name)
                {
                    return true;
                }
                current_node = current_node->mParent;
            }
        }
    }
    return false;
}

auto is_node_a_submesh(const std::string& node_name, const aiScene* scene) -> bool
{
    const aiNode* node = scene->mRootNode->FindNode(node_name.c_str());
    return node != nullptr && node->mNumMeshes > 0;
}

auto is_node_a_parent_of_submesh(const std::string& node_name, const aiScene* scene) -> bool
{
    const aiNode* root = scene->mRootNode;

    for(unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        const aiNode* submesh_node = root->FindNode(mesh->mName);
        const aiNode* current_node = submesh_node;

        while(current_node != nullptr)
        {
            if(current_node->mName.C_Str() == node_name)
            {
                return true;
            }
            current_node = current_node->mParent;
        }
    }
    return false;
}

void process_animation(const aiScene* scene,
                       const aiAnimation* assimp_anim,
                       mesh::load_data& load_data,
                       std::unordered_map<std::string, unsigned int>& node_to_index_lut,
                       animation_clip& anim)
{
    anim.name = assimp_anim->mName.C_Str();
    auto ticks_per_second = assimp_anim->mTicksPerSecond;
    if(ticks_per_second < 0.001)
    {
        ticks_per_second = 25.0;
    }

    auto ticks = assimp_anim->mDuration;

    anim.duration = decltype(anim.duration)(ticks / ticks_per_second);

    if(assimp_anim->mNumChannels > 0)
    {
        anim.channels.reserve(assimp_anim->mNumChannels);
    }

    size_t skipped = 0;
    for(size_t i = 0; i < assimp_anim->mNumChannels; ++i)
    {
        const aiNodeAnim* assimp_node_anim = assimp_anim->mChannels[i];

        bool is_bone = is_node_a_bone(assimp_node_anim->mNodeName.C_Str(), scene);
        bool is_parent_of_bone = is_node_a_parent_of_bone(assimp_node_anim->mNodeName.C_Str(), scene);
        bool is_submesh = is_node_a_submesh(assimp_node_anim->mNodeName.C_Str(), scene);
        bool is_parent_of_submesh = is_node_a_parent_of_submesh(assimp_node_anim->mNodeName.C_Str(), scene);

        bool is_relevant = is_bone || is_parent_of_bone || is_submesh || is_parent_of_submesh;

        // skip frames for non relevant nodes
        if(!is_relevant)
        {
            skipped++;
            continue;
        }

        auto& node_anim = anim.channels.emplace_back();
        node_anim.node_name = assimp_node_anim->mNodeName.C_Str();
        node_anim.node_index = node_to_index_lut[node_anim.node_name];

        if(assimp_node_anim->mNumPositionKeys > 0)
        {
            node_anim.position_keys.resize(assimp_node_anim->mNumPositionKeys);
        }

        for(size_t idx = 0; idx < assimp_node_anim->mNumPositionKeys; ++idx)
        {
            const auto& anim_key = assimp_node_anim->mPositionKeys[idx];
            auto& key = node_anim.position_keys[idx];
            key.time = decltype(key.time)(anim_key.mTime / ticks_per_second);
            key.value.x = anim_key.mValue.x;
            key.value.y = anim_key.mValue.y;
            key.value.z = anim_key.mValue.z;
        }

        if(assimp_node_anim->mNumRotationKeys > 0)
        {
            node_anim.rotation_keys.resize(assimp_node_anim->mNumRotationKeys);
        }

        for(size_t idx = 0; idx < assimp_node_anim->mNumRotationKeys; ++idx)
        {
            const auto& anim_key = assimp_node_anim->mRotationKeys[idx];
            auto& key = node_anim.rotation_keys[idx];
            key.time = decltype(key.time)(anim_key.mTime / ticks_per_second);
            key.value.x = anim_key.mValue.x;
            key.value.y = anim_key.mValue.y;
            key.value.z = anim_key.mValue.z;
            key.value.w = anim_key.mValue.w;
        }

        if(assimp_node_anim->mNumScalingKeys > 0)
        {
            node_anim.scaling_keys.resize(assimp_node_anim->mNumScalingKeys);
        }

        for(size_t idx = 0; idx < assimp_node_anim->mNumScalingKeys; ++idx)
        {
            const auto& anim_key = assimp_node_anim->mScalingKeys[idx];
            auto& key = node_anim.scaling_keys[idx];
            key.time = decltype(key.time)(anim_key.mTime / ticks_per_second);
            key.value.x = anim_key.mValue.x;
            key.value.y = anim_key.mValue.y;
            key.value.z = anim_key.mValue.z;
        }
    }

    APPLOG_INFO("Mesh Importer : Animation {} discarded {} non relevat node keys", anim.name, skipped);
}
void process_animations(const aiScene* scene,
                        mesh::load_data& load_data,
                        std::unordered_map<std::string, unsigned int>& node_to_index_lut,
                        std::vector<animation_clip>& animations)
{
    if(scene->mNumAnimations > 0)
    {
        animations.resize(scene->mNumAnimations);
    }

    for(size_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* assimp_anim = scene->mAnimations[i];
        auto& anim = animations[i];
        process_animation(scene, assimp_anim, load_data, node_to_index_lut, anim);
    }
}

void process_embedded_texture(const aiTexture* assimp_tex,
                              size_t assimp_tex_idx,
                              const fs::path& filename,
                              const fs::path& output_dir,
                              std::vector<imported_texture>& textures)
{
    imported_texture texture{};
    auto it = std::find_if(std::begin(textures),
                           std::end(textures),
                           [&](const imported_texture& texture)
                           {
                               return texture.embedded_index == assimp_tex_idx;
                           });
    if(it != std::end(textures))
    {
        if(it->process_count > 0)
        {
            return;
        }

        it->process_count++;
        texture = *it;
    }
    else if(assimp_tex->mFilename.length > 0)
    {
        texture.name = fs::path(assimp_tex->mFilename.C_Str()).filename().string();
    }
    else
    {
        texture.name = get_embedded_texture_name(assimp_tex, assimp_tex_idx, filename, "Texture");
    }

    fs::path output_file = output_dir / texture.name;

    if(assimp_tex->pcData)
    {
        bool compressed = assimp_tex->mHeight == 0;
        bool raw = assimp_tex->mHeight > 0;

        if(compressed)
        {
            // Compressed texture (e.g., PNG, JPEG)
            size_t texture_size = assimp_tex->mWidth;

            // Parse the image using bimg
            bimg::ImageContainer* image = imageLoad(assimp_tex->pcData, static_cast<uint32_t>(texture_size));
            if(image)
            {
                if(texture.inverse)
                {
                    uint8_t* imageData = static_cast<uint8_t*>(image->m_data);
                    for(uint32_t i = 0; i < image->m_width * image->m_height * 4; ++i)
                    {
                        imageData[i] = 255 - imageData[i];
                    }
                }

                imageSave(output_file.string().c_str(), image);

                bimg::imageFree(image);
            }
        }
        else if(raw)
        {
            // Uncompressed texture (e.g., raw RGBA)
            bx::FileWriter writer;
            bx::Error err;

            if(bx::open(&writer, output_file.string().c_str(), false, &err))
            {
                bimg::imageWriteTga(&writer,
                                    assimp_tex->mWidth,
                                    assimp_tex->mHeight,
                                    assimp_tex->mWidth * 4,
                                    assimp_tex->pcData,
                                    false,
                                    false,
                                    &err);
                bx::close(&writer);
            }
        }
    }
}

template<typename T>
void log_prop_value(aiMaterialProperty* prop, const char* name1)
{
    auto data = (T*)prop->mData;

    auto count = prop->mDataLength / sizeof(T);

    if(count == 1)
    {
        APPLOG_INFO("  {} = {}", name1, data[0]);
    }
    else
    {
        std::vector<T> vals(count);
        std::memcpy(vals.data(), data, count * sizeof(T));
        APPLOG_INFO("  {}[{}] = {}", name1, count, vals);
    }
}

void log_materials(const aiMaterial* material)
{
    for(uint32_t i = 0; i < material->mNumProperties; i++)
    {
        auto prop = material->mProperties[i];

        APPLOG_INFO("Material Property:");
        APPLOG_INFO("  name = {0}", prop->mKey.C_Str());

        if(prop->mDataLength > 0 && prop->mData)
        {
            auto semantic = aiTextureType(prop->mSemantic);
            if(semantic != aiTextureType_NONE && semantic != aiTextureType_UNKNOWN)
            {
                APPLOG_INFO("  semantic = {0}", aiTextureTypeToString(semantic));
            }

            switch(prop->mType)
            {
                case aiPropertyTypeInfo::aiPTI_Float:
                {
                    log_prop_value<float>(prop, "float");
                    break;
                }

                case aiPropertyTypeInfo::aiPTI_Double:
                {
                    log_prop_value<double>(prop, "double");
                    break;
                }
                case aiPropertyTypeInfo::aiPTI_Integer:
                {
                    log_prop_value<int32_t>(prop, "int");
                    break;
                }

                case aiPropertyTypeInfo::aiPTI_Buffer:
                {
                    log_prop_value<uint8_t>(prop, "buffer");
                    break;
                }
                case aiPropertyTypeInfo::aiPTI_String:
                {
                    aiString str;
                    if(aiGetMaterialString(material, prop->mKey.C_Str(), prop->mSemantic, prop->mIndex, &str) ==
                       AI_SUCCESS)
                    {
                        APPLOG_INFO("  string = {0}", str.C_Str());
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

void process_material(asset_manager& am,
                      const fs::path& filename,
                      const fs::path& output_dir,
                      const aiScene* scene,
                      const aiMaterial* material,
                      pbr_material& mat,
                      std::vector<imported_texture>& textures)
{
    if(!material)
    {
        return;
    }
    // log_materials(material);

    auto get_imported_texture = [&](const aiMaterial* material,
                                    aiTextureType type,
                                    unsigned int index,
                                    const std::string& semantic,
                                    imported_texture& tex) -> bool
    {
        aiString path{};
        aiTextureMapping mapping{};
        unsigned int uvindex{};
        float blend{};
        aiTextureOp op{};
        aiTextureMapMode mapmode{};
        unsigned int flags{};

        // Call the function
        aiReturn result = aiGetMaterialTexture(material, // The material pointer
                                               type,     // The type of texture (e.g., diffuse)
                                               index,    // The texture index
                                               &path     // The path where the texture file path will be stored
                                               // &mapping, // The mapping method
                                               // &uvindex, // The UV index
                                               // &blend,   // The blend factor
                                               // &op,      // The texture operation
                                               // &mapmode, // The texture map mode
                                               // &flags    // Additional flags
        );

        if(path.length > 0)
        {
            auto tex_pair = scene->GetEmbeddedTextureAndIndex(path.C_Str());
            const auto embedded_texture = tex_pair.first;
            if(embedded_texture)
            {
                const auto index = tex_pair.second;

                // std::string s = aiTextureTypeToString(type);
                tex.name = get_embedded_texture_name(embedded_texture, index, filename, semantic);
                tex.embedded_index = index;
            }
            else
            {
                tex.name = path.C_Str();
                auto texture_filepath = fs::path(tex.name);

                auto extension = texture_filepath.extension().string();
                auto texture_dir = texture_filepath.parent_path();
                auto texture_filename = texture_filepath.filename().stem().string();
                auto fixed_name = string_utils::replace(texture_filename, ".", "_");
                if(fixed_name != texture_filename)
                {
                    auto old_filepath = output_dir / tex.name;
                    auto fixed_relative = texture_dir / (fixed_name + extension);
                    auto fixed_filepath = output_dir / fixed_relative;

                    fs::error_code ec;
                    if(fs::exists(old_filepath, ec))
                    {
                        fs::rename(old_filepath, fixed_filepath, ec);
                    }
                    else
                    {
                        // doesnt exist. so try to import it
                        fs::copy_file(old_filepath, fixed_filepath, ec);
                    }
                    tex.name = fixed_relative.generic_string();
                }
            }
            tex.semantic = semantic;
            bool use_alpha = flags & aiTextureFlags_UseAlpha;
            bool ignore_alpha = flags & aiTextureFlags_IgnoreAlpha;
            bool invert = flags & aiTextureFlags_Invert;
            tex.inverse = invert;

            switch(mapmode)
            {
                case aiTextureMapMode_Mirror:
                    tex.flags = BGFX_SAMPLER_UVW_MIRROR;
                case aiTextureMapMode_Clamp:
                    tex.flags = BGFX_SAMPLER_UVW_CLAMP;
                case aiTextureMapMode_Decal:
                    tex.flags = BGFX_SAMPLER_UVW_BORDER;
                default:
                    break;
            }

            return true;
        }

        return false;
    };

    auto process_texture = [&](imported_texture& texture, std::vector<imported_texture>& textures)
    {
        if(texture.embedded_index >= 0)
        {
            auto it = std::find_if(std::begin(textures),
                                   std::end(textures),
                                   [&](const imported_texture& rhs)
                                   {
                                       return rhs.embedded_index == texture.embedded_index;
                                   });
            if(it != std::end(textures))
            {
                texture.name = it->name;
                texture.flags = it->flags;
                texture.inverse = it->inverse;
                texture.process_count = it->process_count;
                return;
            }
        }

        textures.emplace_back(texture);

        if(texture.embedded_index >= 0)
        {
            const auto& embedded_texture = scene->mTextures[texture.embedded_index];
            process_embedded_texture(embedded_texture, texture.embedded_index, filename, output_dir, textures);
        }
    };

    // technically there is a difference between MASK and BLEND mode
    // but for our purposes it's enough if we sort properly
    // aiString alpha_mode;
    // material->Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode);
    // aiString alpha_mode_opaque;
    // alpha_mode_opaque.Set("OPAQUE");

    // out.blend = alphaMode != alphaModeOpaque;

    // bool double_sided{};
    // if(material->Get(AI_MATKEY_TWOSIDED, double_sided) == AI_SUCCESS)
    // {
    //     mat.set_cull_type(double_sided ? cull_type::none : cull_type::counter_clockwise);
    // }

    // BASE COLOR TEXTURE
    {
        static const std::string semantic = "BaseColor";
        imported_texture texture;
        bool has_texture = false;

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, AI_MATKEY_BASE_COLOR_TEXTURE, semantic, texture);
        }

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_DIFFUSE, 0, semantic, texture);
        }

        if(has_texture)
        {
            process_texture(texture, textures);

            auto key = fs::convert_to_protocol(output_dir / texture.name);
            mat.set_color_map(am.get_asset<gfx::texture>(key.generic_string()));
        }
    }
    // BASE COLOR PROPERTY
    {
        aiColor3D property{};
        bool has_property = false;

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_BASE_COLOR, property) == AI_SUCCESS;
        }

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_COLOR_DIFFUSE, property) == AI_SUCCESS;
        }

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_COLOR_SPECULAR, property) == AI_SUCCESS;
        }

        if(has_property)
        {
            math::color base_color{};
            base_color = {property.r, property.g, property.b};
            base_color = math::clamp(base_color.value, 0.0f, 1.0f);
            mat.set_base_color(base_color);
        }
    }

    // METALLIC TEXTURE
    {
        static const std::string semantic = "Metallic";

        imported_texture texture;
        bool has_texture = false;

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material,
                                                AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
                                                "MetallicRoughness",
                                                texture);
        }

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, AI_MATKEY_METALLIC_TEXTURE, semantic, texture);
        }

        if(has_texture)
        {
            process_texture(texture, textures);

            auto key = fs::convert_to_protocol(output_dir / texture.name);
            mat.set_metalness_map(am.get_asset<gfx::texture>(key.generic_string()));
        }
    }
    // METALLIC PROPERTY
    {
        ai_real property{};
        bool has_property = false;

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_METALLIC_FACTOR, property) == AI_SUCCESS;
        }

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_REFLECTIVITY, property) == AI_SUCCESS;
        }

        if(has_property)
        {
            // Physically realistic materials are either metal (1.0) or not (0.0)
            // Some models seem to come in with 0.5 which seems wrong - materials are either metal or they are not.
            // (maybe these are specular workflow, and what we're seeing is specular = 0.5 in AI_MATKEY_REFLECTIVITY
            // (?))
            if(property < 0.9f)
            {
                property = 0.0f;
            }

            mat.set_metalness(math::clamp(property, 0.0f, 1.0f));
        }
    }
    // ROUGHNESS TEXTURE
    {
        static const std::string semantic = "Roughness";

        imported_texture texture;
        bool has_texture = false;
        bool invert_property = false;

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material,
                                                AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
                                                "MetallicRoughness",
                                                texture);
        }

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, AI_MATKEY_ROUGHNESS_TEXTURE, semantic, texture);
        }

        if(!has_texture)
        {
            // no PBR roughness. Try old-school shininess.  (note: this also picks up the gloss texture from PBR
            // specular/gloss workflow). Either way, Roughness = (1 - shininess)
            has_texture |= get_imported_texture(material, aiTextureType_SHININESS, 0, semantic, texture);

            invert_property = has_texture;
        }

        if(!has_texture)
        {
            // no PBR roughness. Try old-school shininess.  (note: this also picks up the gloss texture from PBR
            // specular/gloss workflow). Either way, Roughness = (1 - shininess)
            has_texture |= get_imported_texture(material, aiTextureType_SPECULAR, 0, semantic, texture);

            invert_property = has_texture;
        }

        if(has_texture)
        {
            texture.inverse = invert_property;
            process_texture(texture, textures);

            auto key = fs::convert_to_protocol(output_dir / texture.name);
            mat.set_roughness_map(am.get_asset<gfx::texture>(key.generic_string()));
        }
    }

    // ROUGHNESS PROPERTY
    {
        ai_real property{};
        bool has_property = false;
        bool invert_property = false;

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_ROUGHNESS_FACTOR, property) == AI_SUCCESS;
        }

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_GLOSSINESS_FACTOR, property) == AI_SUCCESS;

            invert_property = has_property;
        }

        if(has_property)
        {
            property = math::clamp(property, 0.0f, 1.0f);

            if(invert_property)
            {
                property = 1.0f - property;
            }

            mat.set_roughness(property);
        }
    }

    // NORMAL TEXTURE
    aiTextureType normals_type = aiTextureType_NORMALS;
    {
        static const std::string semantic = "Normals";

        imported_texture texture;
        bool has_texture = false;

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_NORMALS, 0, semantic, texture);
        }

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_NORMAL_CAMERA, 0, semantic, texture);

            if(has_texture)
            {
                normals_type = aiTextureType_NORMAL_CAMERA;
            }
        }

        if(has_texture)
        {
            process_texture(texture, textures);

            auto key = fs::convert_to_protocol(output_dir / texture.name);
            mat.set_normal_map(am.get_asset<gfx::texture>(key.generic_string()));
        }
    }
    // NORMAL BUMP PROPERTY
    {
        ai_real property{};
        bool has_property = false;

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(normals_type, 0), property) == AI_SUCCESS;
        }

        if(has_property)
        {
            mat.set_bumpiness(property);
        }
    }

    // OCCLUSION TEXTURE
    aiTextureType occlusion_type = aiTextureType_AMBIENT_OCCLUSION;
    {
        static const std::string semantic = "Occlusion";

        imported_texture texture;
        bool has_texture = false;

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_AMBIENT_OCCLUSION, 0, semantic, texture);
        }

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_AMBIENT, 0, semantic, texture);

            if(has_texture)
            {
                occlusion_type = aiTextureType_AMBIENT;
            }
        }

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_LIGHTMAP, 0, semantic, texture);
            if(has_texture)
            {
                occlusion_type = aiTextureType_LIGHTMAP;
            }
        }

        if(has_texture)
        {
            process_texture(texture, textures);

            auto key = fs::convert_to_protocol(output_dir / texture.name);
            mat.set_ao_map(am.get_asset<gfx::texture>(key.generic_string()));
        }
    }

    // OCCLUSION STERNGTH PROPERTY
    {
        ai_real property{};
        bool has_property = false;

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(occlusion_type, 0), property) == AI_SUCCESS;
        }

        if(has_property)
        {
        }
    }

    // EMISSIVE TEXTURE
    {
        static const std::string semantic = "Emissive";

        imported_texture texture;
        bool has_texture = false;

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_EMISSION_COLOR, 0, semantic, texture);
        }

        if(!has_texture)
        {
            has_texture |= get_imported_texture(material, aiTextureType_EMISSIVE, 0, semantic, texture);
        }

        if(has_texture)
        {
            process_texture(texture, textures);

            auto key = fs::convert_to_protocol(output_dir / texture.name);
            mat.set_emissive_map(am.get_asset<gfx::texture>(key.generic_string()));
        }
    }
    // EMISSIVE COLOR PROPERTY
    {
        aiColor3D property{};
        bool has_property = false;

        if(!has_property)
        {
            has_property |= material->Get(AI_MATKEY_COLOR_EMISSIVE, property) == AI_SUCCESS;
        }

        if(has_property)
        {
            math::color emissive{};
            emissive = {property.r, property.g, property.b};
            emissive = math::clamp(emissive.value, 0.0f, 1.0f);
            mat.set_emissive_color(emissive);
        }
    }
}

void process_materials(asset_manager& am,
                       const fs::path& filename,
                       const fs::path& output_dir,
                       const aiScene* scene,
                       std::vector<imported_material>& materials,
                       std::vector<imported_texture>& textures)
{
    if(scene->mNumMaterials > 0)
    {
        materials.resize(scene->mNumMaterials);
    }

    for(size_t i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* assimp_mat = scene->mMaterials[i];

        auto mat = std::make_shared<pbr_material>();
        process_material(am, filename, output_dir, scene, assimp_mat, *mat, textures);
        std::string assimp_mat_name = assimp_mat->GetName().C_Str();
        if(assimp_mat_name.empty())
        {
            assimp_mat_name = fmt::format("Material {}", filename.string());
        }
        materials[i].mat = mat;
        materials[i].name = string_utils::replace(fmt::format("[{}] {}", i, assimp_mat_name), ".", "_");
    }
}

void process_embedded_textures(asset_manager& am,
                               const fs::path& filename,
                               const fs::path& output_dir,
                               const aiScene* scene,
                               std::vector<imported_texture>& textures)
{
    if(scene->mNumTextures > 0)
    {
        for(size_t i = 0; i < scene->mNumTextures; ++i)
        {
            const aiTexture* assimp_tex = scene->mTextures[i];

            process_embedded_texture(assimp_tex, i, filename, output_dir, textures);
        }
    }
}

void process_imported_scene(asset_manager& am,
                            const fs::path& filename,
                            const fs::path& output_dir,
                            const aiScene* scene,
                            mesh::load_data& load_data,
                            std::vector<animation_clip>& animations,
                            std::vector<imported_material>& materials,
                            std::vector<imported_texture>& textures)
{
    int meshes_with_bones = 0;
    int meshes_without_bones = 0;

    APPLOG_TRACE_PERF_NAMED(std::chrono::milliseconds, "Mesh Importer: Parse Imported Data");

    load_data.vertex_format = gfx::mesh_vertex::get_layout();

    auto name_to_index_lut = assign_node_indices(scene);

    APPLOG_TRACE("Mesh Importer: Processing materials ...");
    process_materials(am, filename, output_dir, scene, materials, textures);

    APPLOG_TRACE("Mesh Importer: Processing embedded textures ...");
    process_embedded_textures(am, filename, output_dir, scene, textures);

    APPLOG_TRACE("Mesh Importer: Processing meshes ...");
    process_meshes(scene, load_data);

    APPLOG_TRACE("Mesh Importer: Processing nodes ...");
    process_nodes(scene, load_data);

    APPLOG_TRACE("Mesh Importer: Processing animations ...");
    process_animations(scene, load_data, name_to_index_lut, animations);

    APPLOG_TRACE("Mesh Importer: Processing animations bounding boxes ...");
    auto boxes = compute_bounding_boxes_for_animations(scene);

    if(!boxes.empty())
    {
        load_data.bbox = {};
        for(const auto& kvp : boxes)
        {
            for(const auto& box : kvp.second)
            {
                load_data.bbox.add_point(box.min);
                load_data.bbox.add_point(box.max);
            }
        }
    }
    else if(!load_data.bbox.is_populated())
    {
        for(const auto& submesh : load_data.submeshes)
        {
            load_data.bbox.add_point(submesh.bbox.min);
            load_data.bbox.add_point(submesh.bbox.max);
        }
    }

    APPLOG_TRACE("Mesh Importer: bbox min {}, max {}", load_data.bbox.min, load_data.bbox.max);
}

auto read_file(Assimp::Importer& importer, const fs::path& file, uint32_t flags) -> const aiScene*
{
    APPLOG_INFO_PERF_NAMED(std::chrono::milliseconds, "Importer Read File");
    return importer.ReadFile(file.string(), flags);
}

} // namespace

void mesh_importer_init()
{
    struct log_stream : public Assimp::LogStream
    {
        log_stream(Assimp::Logger::ErrorSeverity s) : severity(s)
        {
        }

        void write(const char* message) override
        {
            switch(severity)
            {
                case Assimp::Logger::Info:
                    APPLOG_INFO("Mesh Importer: {0}", message);
                    break;
                case Assimp::Logger::Warn:
                    APPLOG_WARNING("Mesh Importer: {0}", message);
                    break;
                case Assimp::Logger::Err:
                    APPLOG_ERROR("Mesh Importer: {0}", message);
                    break;
                default:
                    APPLOG_TRACE("Mesh Importer: {0}", message);
                    break;
            }
        }

        Assimp::Logger::ErrorSeverity severity{};
    };

    if(Assimp::DefaultLogger::isNullLogger())
    {
        auto logger = Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);

        logger->attachStream(new log_stream(Assimp::Logger::Debugging), Assimp::Logger::Debugging);
        logger->attachStream(new log_stream(Assimp::Logger::Info), Assimp::Logger::Info);
        logger->attachStream(new log_stream(Assimp::Logger::Warn), Assimp::Logger::Warn);
        logger->attachStream(new log_stream(Assimp::Logger::Err), Assimp::Logger::Err);
    }
}

auto load_mesh_data_from_file(asset_manager& am,
                              const fs::path& path,
                              mesh::load_data& load_data,
                              std::vector<animation_clip>& animations,
                              std::vector<imported_material>& materials,
                              std::vector<imported_texture>& textures) -> bool
{
    Assimp::Importer importer;

    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    fs::path file = path.stem();
    fs::path output_dir = path.parent_path();
    fs::path extension = path.extension();

    if(extension == "fbx" || extension == "FBX")
    {
        importer.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, true);
        importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);
    }

    static const uint32_t flags = aiProcess_ConvertToLeftHanded |          //
                                  aiProcessPreset_TargetRealtime_Quality | // some optimizations and safety checks
                                  aiProcess_OptimizeMeshes |               // minimize number of meshes
                                  // aiProcess_OptimizeGraph |                //
                                  aiProcess_TransformUVCoords | //
                                  aiProcess_GlobalScale;

    APPLOG_TRACE("Mesh Importer: Loading {}", path.generic_string());

    const aiScene* scene = read_file(importer, path, flags);

    if(scene == nullptr)
    {
        APPLOG_ERROR(importer.GetErrorString());
        return false;
    }

    process_imported_scene(am, file, output_dir, scene, load_data, animations, materials, textures);

    APPLOG_TRACE("Mesh Importer: Done with {}", path.generic_string());

    return true;
}
} // namespace importer

} // namespace ace
