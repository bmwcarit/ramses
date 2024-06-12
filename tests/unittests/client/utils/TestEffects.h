//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/RamsesClient.h"
#include "ramses/client/EffectDescription.h"
#include "impl/RamsesClientImpl.h"

#include <string_view>

namespace ramses::internal
{
    class TestEffects
    {
    public:
        static Effect* CreateTestEffect(ramses::Scene& scene, std::string_view name = {})
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "void main()\n"
                "{\n"
                "  gl_Position = vec4(0.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "precision highp float;"
                "uniform float u_FragColorR;"
                "uniform float u_FragColorG;"
                "void main(void)\n"
                "{\n"
                "  gl_FragColor = vec4(u_FragColorR, u_FragColorG, 0.0, 0.0); \n"
                "}\n");
            Effect* effect = scene.createEffect(effectDesc, name);
            assert(effect != nullptr);
            return effect;
        }

        static Effect* CreateDifferentTestEffect(ramses::Scene& scene, std::string_view name = {})
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "void main()\n"
                "{\n"
                "  gl_Position = vec4(1.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "void main(void)\n"
                "{\n"
                "  gl_FragColor = vec4(1.0); \n"
                "}\n");
            Effect* effect = scene.createEffect(effectDesc, name);
            assert(effect != nullptr);
            return effect;
        }

        static Effect* CreateTestEffectWithAttribute(ramses::Scene& scene, std::string_view name = {})
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "attribute float a_position;\n"
                "attribute vec2  a_vec2;\n"
                "attribute vec3  a_vec3;\n"
                "attribute vec4  a_vec4;\n"
                "void main()\n"
                "{\n"
                "  gl_Position = vec4(a_position, 0.0, 0.0, 1.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "void main(void)\n"
                "{\n"
                "  gl_FragColor = vec4(0.0); \n"
                "}\n");
            Effect* effect = scene.createEffect(effectDesc, name);
            assert(effect != nullptr);
            return effect;
        }

        static Effect* CreateTestEffectWithAllStages(ramses::Scene& scene, std::string_view name = {})
        {
            const char* vs = R"SHADER(
                #version 320 es
                in vec3 a_position1;
                in float a_position2;
                in float a_position3;
                uniform highp float vs_uniform;
                void main(void)
                {
                    gl_Position = vec4(vs_uniform, a_position1.y, a_position2, a_position3);
                }
                )SHADER";

            const char* gs = R"SHADER(
                #version 320 es
                layout(lines) in;
                layout(points, max_vertices = 1) out;
                uniform highp float gs_uniform;
                void main() {
                    gl_Position = vec4(gs_uniform, 0.0, 0.0, 1.0);
                    EmitVertex();
                }
                )SHADER";

            const char* fs = R"SHADER(
                #version 320 es
                uniform highp vec2 colorRG;
                uniform highp float colorBA[2];
                out lowp vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(colorRG, colorBA[0], colorBA[1]);
                })SHADER";

            EffectDescription effectDesc;
            effectDesc.setVertexShader(vs);
            effectDesc.setFragmentShader(fs);
            effectDesc.setGeometryShader(gs);
            Effect* effect = scene.createEffect(effectDesc, name);
            assert(effect != nullptr);
            return effect;
        }

        static Effect* CreateTestEffectWithAllStagesWithWarnings(ramses::Scene& scene, std::string_view name = {})
        {
            const char* vs = R"SHADER(
                #version 320 es
                in vec3 a_position1;
                in float a_position2;
                in float a_position3;
                uniform highp float vs_uniform;

                out lowp vec3 v_texcoord;

                void main(void)
                {
                    gl_Position = vec4(vs_uniform, a_position1.y, a_position2, a_position3);
                    v_texcoord = a_position1;
                }
                )SHADER";

            const char* gs = R"SHADER(
                #version 320 es
                layout(lines) in;
                layout(points, max_vertices = 1) out;
                uniform highp float gs_uniform;
                void main() {
                    gl_Position = vec4(gs_uniform, 0.0, 0.0, 1.0);
                    EmitVertex();
                }
                )SHADER";

            const char* fs = R"SHADER(
                #version 320 es
                uniform highp vec2 colorRG;
                uniform highp float colorBA[2];
                out lowp vec4 colorOut;

                // unused variable with wrong type: vec2 != vec3
                in highp vec2 v_texcoord;

                void main(void)
                {
                    colorOut = vec4(colorRG, colorBA[0], colorBA[1]);
                })SHADER";

            EffectDescription effectDesc;
            effectDesc.setVertexShader(vs);
            effectDesc.setFragmentShader(fs);
            effectDesc.setGeometryShader(gs);
            Effect* effect = scene.createEffect(effectDesc, name);
            assert(effect != nullptr);
            return effect;
        }

        static Effect* CreateTestEffectWithUBOSemantics(ramses::Scene& scene, const std::unordered_set<EEffectUniformSemantic>& semantics, std::string_view name = {})
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(R"SHADER(
                #version 320 es
                layout(std140,binding=0) uniform uniformBlock_t3
                {
                    mat4 ubModelMat;
                } modelUBO;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 projMat;
                    mat4 viewMat;
                    vec3 cameraPos;
                } camUBO;
                layout(std140,binding=2) uniform uniformBuffer_t2
                {
                    mat4 mvpMat;
                    mat4 mvMat;
                    mat4 normalMat;
                } modelCamUBO;
                void main()
                {
                    gl_Position = modelUBO.ubModelMat * camUBO.projMat * camUBO.viewMat * modelCamUBO.mvpMat * vec4(0.0);
                })SHADER");
            effectDesc.setFragmentShader(R"SHADER(
                #version 320 es
                precision mediump float;
                out vec4 FragColor;
                void main(void)
                {
                    FragColor = vec4(0.0);
                })SHADER");

            if (semantics.count(EEffectUniformSemantic::ModelBlock) != 0u)
                effectDesc.setUniformSemantic(0, EEffectUniformSemantic::ModelBlock);
            if (semantics.count(EEffectUniformSemantic::CameraBlock) != 0u)
                effectDesc.setUniformSemantic(1, EEffectUniformSemantic::CameraBlock);
            if (semantics.count(EEffectUniformSemantic::ModelCameraBlock) != 0u)
                effectDesc.setUniformSemantic(2, EEffectUniformSemantic::ModelCameraBlock);
            Effect* effect = scene.createEffect(effectDesc, name);
            assert(effect != nullptr);
            return effect;
        }
    };
}
