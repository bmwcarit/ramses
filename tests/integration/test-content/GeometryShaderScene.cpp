//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/GeometryShaderScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/EffectDescription.h"
#include <cassert>

namespace ramses::internal
{
    GeometryShaderScene::GeometryShaderScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(createTestEffect(state))
    {
        ramses::Appearance* appearance = m_scene.createAppearance(m_effect);
        appearance->setCullingMode(ramses::ECullMode::Disabled);
        ramses::Geometry* geometry = m_scene.createGeometry(m_effect);
        appearance->setDrawMode(GetDrawMode(state));

        const std::array<ramses::vec2f, 4u> vertexPositionsArray{
            ramses::vec2f{.3f, -.3f},
            ramses::vec2f{.3f,  .3f},
            ramses::vec2f{-.3f, -.3f},
            ramses::vec2f{-.3f,  .3f} };

        const std::array<ramses::vec3f, 4u> vertexColorsArray{
            ramses::vec3f{1.f, 0.f, 0.f},
            ramses::vec3f{0.f, 1.f, 0.f},
            ramses::vec3f{0.f, 0.f, 1.f},
            ramses::vec3f{1.f, 1.f, 1.f} };

        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());
        geometry->setInputBuffer(*m_effect.findAttributeInput("a_pos"), *vertexPositions);

        const ramses::ArrayResource* vertexColors = m_scene.createArrayResource(4u, vertexColorsArray.data());
        geometry->setInputBuffer(*m_effect.findAttributeInput("a_color"), *vertexColors);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);
        meshNode->setIndexCount(4);

        addMeshNodeToDefaultRenderGroup(*meshNode);
    }

    const ramses::Effect& GeometryShaderScene::createTestEffect(uint32_t state)
    {
        const std::string shaderVersion = GetShaderVersion(state);

        const std::string vertexShaderV320es = shaderVersion + R"SHADER(
            in highp vec2 a_pos;
            in highp vec3 a_color;
            out highp vec3 v_color;
            void main()
            {
                gl_Position = vec4(a_pos, 0.0, 1.0);
                v_color = a_color;
            }
            )SHADER";

        const std::string fragmentSahderV320es = shaderVersion + R"SHADER(
            out highp vec4 color;
            in highp vec3 g_color;
            void main()
            {
                color = vec4(g_color, 1.0);
            }
            )SHADER";

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShader(vertexShaderV320es.c_str());
        effectDesc.setFragmentShader(fragmentSahderV320es.c_str());

        const std::string geometryShaderExtensions = GetGeometryShaderExtensions(state);
        const std::string geometryShaderLayour = GetGeometryShaderLayout(state);

        switch(state)
        {
        case GLSL320_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_POINTS_IN_LINE_STRIP_OUT:
        case GLSL320_POINTS_IN_POINTS_OUT:
        case GLSL310_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_POINTS_IN_LINE_STRIP_OUT:
        case GLSL310_POINTS_IN_POINTS_OUT:
            effectDesc.setGeometryShader(std::string(shaderVersion + geometryShaderExtensions + geometryShaderLayour + R"SHADER(
                in highp vec3 v_color[];
                out highp vec3 g_color;
                void main() {
                    vec4 positionIn = gl_in[0].gl_Position;

                    gl_Position = positionIn + vec4(0.3 , -0.3, 0.0, 0.0);
                    g_color = v_color[0];
                    EmitVertex();
                    gl_Position = positionIn + vec4(0.0, 0.3, 0.0, 0.0);
                    g_color = v_color[0];
                    EmitVertex();
                    gl_Position = positionIn + vec4(-0.0, -0.3, 0.0, 0.0);
                    g_color = v_color[0];
                    EmitVertex();

                    if(gl_PrimitiveIDIn > 1)
                    {
                        gl_Position = positionIn + vec4(-0.3, 0.3, 0.0, 0.0);
                        g_color = v_color[0];
                        EmitVertex();
                    }
                    EndPrimitive();
                }
                )SHADER").c_str());
            break;

        case GLSL320_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_TRIANGLES_IN_POINTS_OUT:
        case GLSL310_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_TRIANGLES_IN_POINTS_OUT:
            effectDesc.setGeometryShader(std::string(shaderVersion + geometryShaderExtensions + geometryShaderLayour + R"SHADER(
                in highp vec3 v_color[];
                out highp vec3 g_color;
                void main() {
                    vec4 positionIn_0 = gl_in[0].gl_Position;
                    vec4 positionIn_1 = gl_in[1].gl_Position;
                    vec4 positionIn_2 = gl_in[2].gl_Position;

                    gl_Position = positionIn_0 + vec4(0.1 , -0.1, 0.0, 0.0);
                    g_color = v_color[0];
                    EmitVertex();
                    gl_Position = positionIn_0 + vec4(0.0, 0.1, 0.0, 0.0);
                    g_color = v_color[0];
                    EmitVertex();
                    gl_Position = positionIn_0 + vec4(-0.0, -0.1, 0.0, 0.0);
                    g_color = v_color[0];
                    EmitVertex();

                    gl_Position = positionIn_1;
                    g_color = v_color[1];
                    EmitVertex();
                    gl_Position = positionIn_1 + vec4(0.0, 0.1, 0.0, 0.0);
                    g_color = v_color[1];
                    EmitVertex();

                    gl_Position = positionIn_2;
                    g_color = v_color[2];
                    EmitVertex();
                    gl_Position = positionIn_2 + vec4(0.0, 0.1, 0.0, 0.0);
                    g_color = v_color[2];
                    EmitVertex();
                    EndPrimitive();
                }
                )SHADER").c_str());
            break;
        default:
            assert(false);
        }

        return *m_scene.createEffect(effectDesc);
    }

    std::string GeometryShaderScene::GetGeometryShaderLayout(uint32_t state)
    {
        switch (state)
        {
        case GLSL320_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_POINTS_IN_TRIANGLE_STRIP_OUT:
            return R"SHADER(
                layout(points) in;
                layout(triangle_strip, max_vertices = 4) out;
                    )SHADER";

        case GLSL320_POINTS_IN_LINE_STRIP_OUT:
        case GLSL310_POINTS_IN_LINE_STRIP_OUT:
            return R"SHADER(
                layout(points) in;
                layout(line_strip, max_vertices = 4) out;
                    )SHADER";

        case GLSL320_POINTS_IN_POINTS_OUT:
        case GLSL310_POINTS_IN_POINTS_OUT:
            return R"SHADER(
                layout(points) in;
                layout(points, max_vertices = 4) out;
                    )SHADER";

        case GLSL320_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
            return R"SHADER(
                layout(triangles) in;
                layout(triangle_strip, max_vertices = 8) out;
                    )SHADER";

        case GLSL320_TRIANGLES_IN_POINTS_OUT:
        case GLSL310_TRIANGLES_IN_POINTS_OUT:
            return R"SHADER(
                layout(triangles) in;
                layout(points, max_vertices = 8) out;
                    )SHADER";

        default:
            assert(false);
        }

        return "";
    }

    std::string GeometryShaderScene::GetShaderVersion(uint32_t state)
    {
        switch (state)
        {
        case GLSL320_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_POINTS_IN_LINE_STRIP_OUT:
        case GLSL320_POINTS_IN_POINTS_OUT:
        case GLSL320_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_TRIANGLES_IN_POINTS_OUT:
            return R"SHADER(
                #version 320 es
                    )SHADER";
        case GLSL310_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_POINTS_IN_LINE_STRIP_OUT:
        case GLSL310_POINTS_IN_POINTS_OUT:
        case GLSL310_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_TRIANGLES_IN_POINTS_OUT:
            return R"SHADER(
                #version 310 es
                    )SHADER";
        default:
            assert(false);
        }

        return "";
    }

    std::string GeometryShaderScene::GetGeometryShaderExtensions(uint32_t state)
    {
        switch (state)
        {
        case GLSL320_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_POINTS_IN_LINE_STRIP_OUT:
        case GLSL320_POINTS_IN_POINTS_OUT:
        case GLSL320_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_TRIANGLES_IN_POINTS_OUT:
            return "";
        case GLSL310_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_POINTS_IN_LINE_STRIP_OUT:
        case GLSL310_POINTS_IN_POINTS_OUT:
        case GLSL310_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_TRIANGLES_IN_POINTS_OUT:
            return R"SHADER(
                #extension GL_EXT_geometry_shader: enable
                    )SHADER";
        default:
            assert(false);
        }

        return "";
    }

    ramses::EDrawMode GeometryShaderScene::GetDrawMode(uint32_t state)
    {
        switch (state)
        {
        case GLSL320_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_POINTS_IN_LINE_STRIP_OUT:
        case GLSL320_POINTS_IN_POINTS_OUT:
        case GLSL310_POINTS_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_POINTS_IN_LINE_STRIP_OUT:
        case GLSL310_POINTS_IN_POINTS_OUT:
            return ramses::EDrawMode::Points;

        case GLSL320_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL320_TRIANGLES_IN_POINTS_OUT:
        case GLSL310_TRIANGLES_IN_TRIANGLE_STRIP_OUT:
        case GLSL310_TRIANGLES_IN_POINTS_OUT:
            return ramses::EDrawMode::TriangleStrip;
        default:
            break;
        }

        assert(false);
        return ramses::EDrawMode::Points;
    }

}
