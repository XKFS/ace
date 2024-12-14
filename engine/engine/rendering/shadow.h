#pragma once
#include <engine/engine_export.h>

#include <engine/ecs/ecs.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/light.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/ecs/ecs.h>
#include <engine/rendering/gpu_program.h>
#include <graphics/graphics.h>

namespace ace
{
namespace shadow
{
struct LightType
{
    enum Enum
    {
        SpotLight,
        PointLight,
        DirectionalLight,

        Count
    };
};

struct DepthImpl
{
    enum Enum
    {
        InvZ,
        Linear,

        Count
    };
};

struct PackDepth
{
    enum Enum
    {
        RGBA,
        VSM,

        Count
    };
};

struct SmImpl
{
    enum Enum
    {
        Hard,
        PCF,
        PCSS,
        VSM,
        ESM,

        Count
    };
};

struct SmType
{
    enum Enum
    {
        Single,
        Omni,
        Cascade,

        Count
    };
};

struct TetrahedronFaces
{
    enum Enum
    {
        Green,
        Yellow,
        Blue,
        Red,

        Count
    };
};

struct ProjType
{
    enum Enum
    {
        Horizontal,
        Vertical,

        Count
    };
};

struct ShadowMapRenderTargets
{
    enum Enum
    {
        First,
        Second,
        Third,
        Fourth,

        Count
    };
};

struct PosNormalTexcoordVertex
{
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_normal;
    float m_u;
    float m_v;
};

struct Light
{
    union Position
    {
        struct
        {
            float m_x;
            float m_y;
            float m_z;
            float m_w;
        };

        float m_v[4];
    };

    union SpotDirectionInner
    {
        struct
        {
            float m_x;
            float m_y;
            float m_z;
            float m_inner;
        };

        float m_v[4];
    };

    Position m_position;
    SpotDirectionInner m_spotDirectionInner;
};

struct Uniforms
{
    void init()
    {
        m_ambientPass = 1.0f;
        m_lightingPass = 1.0f;

        m_shadowMapBias = 0.003f;
        m_shadowMapOffset = 0.0f;
        m_shadowMapParam0 = 0.5;
        m_shadowMapParam1 = 1.0;
        m_depthValuePow = 1.0f;
        m_showSmCoverage = 1.0f;
        m_shadowMapTexelSize = 1.0f / 512.0f;

        m_csmFarDistances[0] = 30.0f;
        m_csmFarDistances[1] = 90.0f;
        m_csmFarDistances[2] = 180.0f;
        m_csmFarDistances[3] = 1000.0f;

        m_tetraNormalGreen[0] = 0.0f;
        m_tetraNormalGreen[1] = -0.57735026f;
        m_tetraNormalGreen[2] = 0.81649661f;

        m_tetraNormalYellow[0] = 0.0f;
        m_tetraNormalYellow[1] = -0.57735026f;
        m_tetraNormalYellow[2] = -0.81649661f;

        m_tetraNormalBlue[0] = -0.81649661f;
        m_tetraNormalBlue[1] = 0.57735026f;
        m_tetraNormalBlue[2] = 0.0f;

        m_tetraNormalRed[0] = 0.81649661f;
        m_tetraNormalRed[1] = 0.57735026f;
        m_tetraNormalRed[2] = 0.0f;

        m_XNum = 2.0f;
        m_YNum = 2.0f;
        m_XOffset = 10.0f / 512.0f;
        m_YOffset = 10.0f / 512.0f;

        u_params0 = bgfx::createUniform("u_params0", bgfx::UniformType::Vec4);
        u_params1 = bgfx::createUniform("u_params1", bgfx::UniformType::Vec4);
        u_params2 = bgfx::createUniform("u_params2", bgfx::UniformType::Vec4);
        u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
        u_smSamplingParams = bgfx::createUniform("u_smSamplingParams", bgfx::UniformType::Vec4);
        u_csmFarDistances = bgfx::createUniform("u_csmFarDistances", bgfx::UniformType::Vec4);
        u_lightMtx = bgfx::createUniform("u_lightMtx", bgfx::UniformType::Mat4);

        u_tetraNormalGreen = bgfx::createUniform("u_tetraNormalGreen", bgfx::UniformType::Vec4);
        u_tetraNormalYellow = bgfx::createUniform("u_tetraNormalYellow", bgfx::UniformType::Vec4);
        u_tetraNormalBlue = bgfx::createUniform("u_tetraNormalBlue", bgfx::UniformType::Vec4);
        u_tetraNormalRed = bgfx::createUniform("u_tetraNormalRed", bgfx::UniformType::Vec4);

        u_shadowMapMtx0 = bgfx::createUniform("u_shadowMapMtx0", bgfx::UniformType::Mat4);
        u_shadowMapMtx1 = bgfx::createUniform("u_shadowMapMtx1", bgfx::UniformType::Mat4);
        u_shadowMapMtx2 = bgfx::createUniform("u_shadowMapMtx2", bgfx::UniformType::Mat4);
        u_shadowMapMtx3 = bgfx::createUniform("u_shadowMapMtx3", bgfx::UniformType::Mat4);
    }

    void setPtrs(Light* _lightPtr,
                 float* _colorPtr,
                 float* _lightMtxPtr,
                 float* _shadowMapMtx0,
                 float* _shadowMapMtx1,
                 float* _shadowMapMtx2,
                 float* _shadowMapMtx3)
    {
        m_lightMtxPtr = _lightMtxPtr;
        m_colorPtr = _colorPtr;
        m_lightPtr = _lightPtr;

        m_shadowMapMtx0 = _shadowMapMtx0;
        m_shadowMapMtx1 = _shadowMapMtx1;
        m_shadowMapMtx2 = _shadowMapMtx2;
        m_shadowMapMtx3 = _shadowMapMtx3;
    }

    // Call this once at initialization.
    void submitConstUniforms() const
    {
        bgfx::setUniform(u_tetraNormalGreen, m_tetraNormalGreen);
        bgfx::setUniform(u_tetraNormalYellow, m_tetraNormalYellow);
        bgfx::setUniform(u_tetraNormalBlue, m_tetraNormalBlue);
        bgfx::setUniform(u_tetraNormalRed, m_tetraNormalRed);
    }

    // Call this once per frame.
    void submitPerFrameUniforms() const
    {
        bgfx::setUniform(u_params1, m_params1);
        bgfx::setUniform(u_params2, m_params2);
        bgfx::setUniform(u_smSamplingParams, m_paramsBlur);
        bgfx::setUniform(u_csmFarDistances, m_csmFarDistances);
    }

    // Call this before each draw call.
    void submitPerDrawUniforms() const
    {
        bgfx::setUniform(u_shadowMapMtx0, m_shadowMapMtx0);
        bgfx::setUniform(u_shadowMapMtx1, m_shadowMapMtx1);
        bgfx::setUniform(u_shadowMapMtx2, m_shadowMapMtx2);
        bgfx::setUniform(u_shadowMapMtx3, m_shadowMapMtx3);

        bgfx::setUniform(u_params0, m_params0);
        bgfx::setUniform(u_lightMtx, m_lightMtxPtr);
        bgfx::setUniform(u_color, m_colorPtr);
    }

    void destroy()
    {
        bgfx::destroy(u_params0);
        bgfx::destroy(u_params1);
        bgfx::destroy(u_params2);
        bgfx::destroy(u_color);
        bgfx::destroy(u_smSamplingParams);
        bgfx::destroy(u_csmFarDistances);

        bgfx::destroy(u_tetraNormalGreen);
        bgfx::destroy(u_tetraNormalYellow);
        bgfx::destroy(u_tetraNormalBlue);
        bgfx::destroy(u_tetraNormalRed);

        bgfx::destroy(u_shadowMapMtx0);
        bgfx::destroy(u_shadowMapMtx1);
        bgfx::destroy(u_shadowMapMtx2);
        bgfx::destroy(u_shadowMapMtx3);

        bgfx::destroy(u_lightMtx);
    }

    union
    {
        struct
        {
            float m_ambientPass;
            float m_lightingPass;
            float m_unused00;
            float m_unused01;
        };

        float m_params0[4];
    };

    union
    {
        struct
        {
            float m_shadowMapBias;
            float m_shadowMapOffset;
            float m_shadowMapParam0;
            float m_shadowMapParam1;
        };

        float m_params1[4];
    };

    union
    {
        struct
        {
            float m_depthValuePow;
            float m_showSmCoverage;
            float m_shadowMapTexelSize;
            float m_unused23;
        };

        float m_params2[4];
    };

    union
    {
        struct
        {
            float m_XNum;
            float m_YNum;
            float m_XOffset;
            float m_YOffset;
        };

        float m_paramsBlur[4];
    };

    float m_tetraNormalGreen[3];
    float m_tetraNormalYellow[3];
    float m_tetraNormalBlue[3];
    float m_tetraNormalRed[3];
    float m_csmFarDistances[4];

    float* m_lightMtxPtr;
    float* m_colorPtr;
    Light* m_lightPtr;
    float* m_shadowMapMtx0;
    float* m_shadowMapMtx1;
    float* m_shadowMapMtx2;
    float* m_shadowMapMtx3;

private:
    bgfx::UniformHandle u_params0;
    bgfx::UniformHandle u_params1;
    bgfx::UniformHandle u_params2;
    bgfx::UniformHandle u_color;
    bgfx::UniformHandle u_smSamplingParams;
    bgfx::UniformHandle u_csmFarDistances;

    bgfx::UniformHandle u_tetraNormalGreen;
    bgfx::UniformHandle u_tetraNormalYellow;
    bgfx::UniformHandle u_tetraNormalBlue;
    bgfx::UniformHandle u_tetraNormalRed;

    bgfx::UniformHandle u_shadowMapMtx0;
    bgfx::UniformHandle u_shadowMapMtx1;
    bgfx::UniformHandle u_shadowMapMtx2;
    bgfx::UniformHandle u_shadowMapMtx3;

    bgfx::UniformHandle u_lightMtx;
};

struct RenderState
{
    enum Enum
    {
        Default = 0,

        ShadowMap_PackDepth,
        ShadowMap_PackDepthHoriz,
        ShadowMap_PackDepthVert,

        Custom_BlendLightTexture,
        Custom_DrawPlaneBottom,

        Count
    };

    uint64_t m_state;
    uint32_t m_blendFactorRgba;
    uint32_t m_fstencil;
    uint32_t m_bstencil;
};

struct PosColorTexCoord0Vertex : gfx::vertex<PosColorTexCoord0Vertex>
{
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_rgba;
    float m_u;
    float m_v;

    static void init(gfx::vertex_layout& decl)
    {
        decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }
};

struct PosVertex : gfx::vertex<PosVertex>
{
    float m_x;
    float m_y;
    float m_z;
    static void init(gfx::vertex_layout& decl)
    {
        decl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
    }
};

struct Programs
{
    void init(rtti::context& ctx);

    void destroy()
    {
        // Pack depth.
        for(uint8_t ii = 0; ii < DepthImpl::Count; ++ii)
        {
            for(uint8_t jj = 0; jj < PackDepth::Count; ++jj)
            {
                m_packDepth[ii][jj].reset();
            }
        }

        // Draw depth.
        for(uint8_t ii = 0; ii < PackDepth::Count; ++ii)
        {
            m_drawDepth[ii].reset();
        }

        // Hblur.
        for(uint8_t ii = 0; ii < PackDepth::Count; ++ii)
        {
            m_hBlur[ii].reset();
        }

        // Vblur.
        for(uint8_t ii = 0; ii < PackDepth::Count; ++ii)
        {
            m_vBlur[ii].reset();
        }

        // Misc.
        m_black.reset();
    }

    gpu_program::ptr m_black;
    gpu_program::ptr m_vBlur[PackDepth::Count];
    gpu_program::ptr m_hBlur[PackDepth::Count];
    gpu_program::ptr m_drawDepth[PackDepth::Count];
    gpu_program::ptr m_packDepth[DepthImpl::Count][PackDepth::Count];
    gpu_program::ptr m_packDepthSkinned[DepthImpl::Count][PackDepth::Count];
};

struct ShadowMapSettings
{
#define SHADOW_FLOAT_PARAM(_name)                                                                                      \
    float _name{}, _name##Min{}, _name##Max{}, _name##Step                                                             \
    {                                                                                                                  \
    }
    SHADOW_FLOAT_PARAM(m_sizePwrTwo);
    SHADOW_FLOAT_PARAM(m_depthValuePow);
    SHADOW_FLOAT_PARAM(m_near);
    SHADOW_FLOAT_PARAM(m_far);
    SHADOW_FLOAT_PARAM(m_bias);
    SHADOW_FLOAT_PARAM(m_normalOffset);
    SHADOW_FLOAT_PARAM(m_customParam0);
    SHADOW_FLOAT_PARAM(m_customParam1);
    SHADOW_FLOAT_PARAM(m_xNum);
    SHADOW_FLOAT_PARAM(m_yNum);
    SHADOW_FLOAT_PARAM(m_xOffset);
    SHADOW_FLOAT_PARAM(m_yOffset);
    bool m_doBlur{};
    gpu_program* m_progPack{};
    gpu_program* m_progPackSkinned{};
#undef SHADOW_FLOAT_PARAM
};

struct SceneSettings
{
    LightType::Enum m_lightType;
    DepthImpl::Enum m_depthImpl;
    SmImpl::Enum m_smImpl;
    float m_spotOuterAngle;
    float m_spotInnerAngle;
    float m_fovXAdjust;
    float m_fovYAdjust;
    float m_coverageSpotL;
    float m_splitDistribution;
    int m_numSplits;
    bool m_updateLights;
    bool m_updateScene;
    bool m_drawDepthBuffer;
    bool m_showSmCoverage;
    bool m_stencilPack;
    bool m_stabilize;
};

struct ClearValues
{
    ClearValues(uint32_t _clearRgba = 0x30303000, float _clearDepth = 1.0f, uint8_t _clearStencil = 0)
        : clear_rgba(_clearRgba)
        , clear_depth(_clearDepth)
        , clear_stencil(_clearStencil)
    {
    }

    uint32_t clear_rgba;
    float clear_depth;
    uint8_t clear_stencil;
};

using shadow_map_models_t = std::vector<entt::handle>;

class shadowmap_generator
{
public:
    shadowmap_generator();
    ~shadowmap_generator();

    void init(rtti::context& ctx);
    void deinit();
    void deinit_textures();
    void deinit_uniforms();

    void update(const camera& cam, const light& l, const math::transform& ltrans);
    auto already_updated() const -> bool;

    void generate_shadowmaps(const shadow_map_models_t& model);

    auto get_depth_type() const -> PackDepth::Enum;
    auto get_rt_texture(uint8_t split) const -> bgfx::TextureHandle;
    auto get_depth_render_program(PackDepth::Enum depth) const -> gpu_program::ptr;
    void submit_uniforms(uint8_t stage) const;

private:
    auto render_scene_into_shadowmap(uint8_t shadowmap_1_id,
                                     const shadow_map_models_t& models,
                                     const math::frustum frustums[ShadowMapRenderTargets::Count],
                                     ShadowMapSettings* currentSmSettings) -> bool;

    ClearValues clear_values_;

    float color_[4];
    Light point_light_;
    Light directional_light_;

    float light_mtx_[16];
    float shadow_map_mtx_[ShadowMapRenderTargets::Count][16];

    ShadowMapSettings sm_settings_[LightType::Count][DepthImpl::Count][SmImpl::Count];
    SceneSettings settings_;

    uint16_t current_shadow_map_size_{};

    Uniforms uniforms_;
    Programs programs_;

    float light_view_[ShadowMapRenderTargets::Count][16];
    float light_proj_[ShadowMapRenderTargets::Count][16];

    math::frustum light_frustums_[ShadowMapRenderTargets::Count];

    bgfx::UniformHandle tex_color_{bgfx::kInvalidHandle};
    bgfx::UniformHandle shadow_map_[ShadowMapRenderTargets::Count];
    bgfx::FrameBufferHandle rt_shadow_map_[ShadowMapRenderTargets::Count];
    bgfx::FrameBufferHandle rt_blur_{bgfx::kInvalidHandle};

    bool valid_{};

    uint64_t last_update_ = -1;
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};

} // namespace shadow
} // namespace ace
