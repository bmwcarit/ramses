//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include <gtest/gtest.h>
#include "TestEffectCreator.h"
#include "ramses/client/EffectDescription.h"

namespace ramses::internal
{
    Effect* TestEffectCreator::CreateEffect(ramses::Scene& scene, bool withGeometryShader, EFeatureLevel featureLevel)
    {
        std::string VertexShader(R"SHADER(
                #version 320 es
                uniform lowp float floatInput;
                uniform lowp float floatInputArray[3];

                uniform vec2 vec2fInput;
                uniform vec2 vec2fInputArray[3];
                uniform vec3 vec3fInput;
                uniform vec3 vec3fInputArray[3];
                uniform vec4 vec4fInput;
                uniform vec4 vec4fInputArray[3];

                uniform mat2 matrix22fInput;
                uniform mat2 matrix22fInputArray[3];
                uniform mat3 matrix33fInput;
                uniform mat3 matrix33fInputArray[3];
                uniform mat4 matrix44fInput;
                uniform mat4 matrix44fInputArray[3];

                uniform bool boolInput;
                uniform bool boolInputArray[3];

                uniform lowp int integerInput;
                uniform lowp int integerInputArray[3];

                uniform ivec2 vec2iInput;
                uniform ivec2 vec2iInputArray[3];
                uniform ivec3 vec3iInput;
                uniform ivec3 vec3iInputArray[3];
                uniform ivec4 vec4iInput;
                uniform ivec4 vec4iInputArray[3];

                //__UBO__DECLARATION

                uniform mat4 mvMatrix;

                in float floatArrayInput;
                in vec2 vec2fArrayInput;
                in vec3 vec3fArrayInput;
                in vec4 vec4fArrayInput;

                void main(void)
                {
                    lowp vec4 values = vec4(1) * floatInput* vec2fInput.x * vec3fInput.y* vec4fInput.z;
                    values[0] = float(integerInput) + floatInputArray[0] + floatInputArray[1];
                    values[1] = float(vec2iInput.x)* float(vec3iInput.y)* float(vec4iInput.z);
                    values[2] = vec2fArrayInput.x*vec3fArrayInput.y*vec4fArrayInput.z;
                    values[3] = floatArrayInput*vec2fArrayInput.x*vec4fArrayInput.z;
                    values[0] += floatInputArray[0] + vec2fInputArray[0].x + vec3fInputArray[0].x + vec4fInputArray[0].x;
                    values[1] += matrix44fInputArray[0][0].x + float(integerInputArray[0]);
                    values[2] += float(vec2iInputArray[0].x + vec3iInputArray[0].x + vec4iInputArray[0].x);
                    values[3] += matrix22fInput[0][0] + matrix33fInput[0][0] + matrix22fInputArray[0][0].x + matrix33fInputArray[0][0].x;
                    //__UBO__USAGE
                    values = matrix44fInput * mvMatrix * values;
                    if (boolInput || boolInputArray[0] || boolInputArray[1] || boolInputArray[2])
                        gl_Position = values;
                }
                )SHADER");

        static_assert(EFeatureLevel_Latest != EFeatureLevel_01, "Remove all this and make UBO declaration/usage always part of test shader");
        if (featureLevel >= EFeatureLevel_02)
        {
            const std::string_view UBODecl(R"UBODecl(
                layout(std140,binding=1) uniform uniformBlock_t
                {
                    mat4 ubMat44;
                    float ubFloat[3];
                    mat3 ubMat33;
                } uniformBlock;

                //anonymous uniform block
                layout(std140,binding=2) uniform uniformBlock_t2
                {
                    bool ubBool;
                    mat3 ubMat33;
                    mat2 ubMat22;
                    ivec4 ubIVec4;
                    vec2 ubVec2;
                    int ubInt;
                };

                // uniform block with semantics
                layout(std140,binding=3) uniform uniformBlock_t3
                {
                    mat4 ubModelMat;
                } ubWithSemantics;
                )UBODecl");
            const std::string_view UBOUsage("values[0] += uniformBlock.ubFloat[0] + ubVec2.x + ubMat22[0][0] + ubMat33[0][0] + ubWithSemantics.ubModelMat[0][0];");

            auto replaceStr = [](std::string& str, std::string_view from, std::string_view to) { str.replace(str.find(from), from.length(), to); };
            replaceStr(VertexShader, "//__UBO__DECLARATION", UBODecl);
            replaceStr(VertexShader, "//__UBO__USAGE", UBOUsage);
        }

        const std::string_view FragmentShader(R"SHADER(
                #version 320 es
                precision mediump float;
                uniform sampler2D texture2dInput;
                uniform lowp sampler2DMS texture2dMSInput;
                uniform lowp sampler3D texture3dInput;
                uniform samplerCube textureCubeInput;
                #extension GL_OES_EGL_image_external_essl3 : require
                uniform samplerExternalOES textureExternalInput;
                out vec4 FragColor;
                void main(void)
                {
                    FragColor = vec4(1.0) + texture(texture2dInput, vec2(0,0)) + texelFetch(texture2dMSInput, ivec2(0,0), 0) + texture(texture3dInput, vec3(0, 0, 0)) + texture(textureCubeInput, vec3(0,0,0)) + texture(textureExternalInput, vec2(0,0));
                }
                )SHADER");

        EffectDescription effectDesc;
        effectDesc.setVertexShader(VertexShader);
        effectDesc.setFragmentShader(FragmentShader);

        if (withGeometryShader)
        {
            effectDesc.setGeometryShader(R"SHADER(
                #version 320 es
                layout(lines) in;
                layout(points, max_vertices = 1) out;
                void main() {
                    gl_Position = vec4(0.0);
                    EmitVertex();
                }
                )SHADER");
        }

        effectDesc.setAttributeSemantic("vec2fArrayInput", EEffectAttributeSemantic::TextPositions);
        effectDesc.setUniformSemantic("mvMatrix", EEffectUniformSemantic::ModelViewMatrix);
        effectDesc.setUniformSemantic("texture2dInput", EEffectUniformSemantic::TextTexture);
        effectDesc.setUniformSemantic(3u, EEffectUniformSemantic::ModelBlock);

        auto effect = scene.createEffect(effectDesc, "input test effect");
        EXPECT_TRUE(effect) << scene.getLastEffectErrorMessages();
        return effect;
    }

    std::array<std::unique_ptr<TestEffectCreator>, ramses::EFeatureLevel_Latest> TestWithSharedEffectPerFeatureLevel::effectCreators;
}
