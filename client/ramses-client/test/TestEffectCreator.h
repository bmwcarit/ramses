//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTEFFECTCREATOR_H
#define RAMSES_TESTEFFECTCREATOR_H

#include <gtest/gtest.h>

#include "ramses-client-api/EffectDescription.h"
#include "ClientTestUtils.h"

namespace ramses
{
    class TestEffectCreator : public LocalTestClientWithScene
    {
    public:
        TestEffectCreator(bool withSemantics = false)
            : LocalTestClientWithScene()
        {
            effect = createEffect(client, withSemantics);
            EXPECT_TRUE(effect != NULL);
            appearance = this->m_scene.createAppearance(*effect);
            EXPECT_TRUE(appearance != NULL);
        }

        virtual ~TestEffectCreator()
        {
            EXPECT_EQ(StatusOK, this->m_scene.destroy(*appearance));
        }

        static Effect* createEffect(RamsesClient& client, bool withSemantics)
        {
            ramses_internal::String VertexShader(
                "uniform lowp float floatInput;\n"
                "uniform lowp float floatInputArray[3];\n"

                "uniform vec2 vec2fInput;\n"
                "uniform vec2 vec2fInputArray[3];\n"
                "uniform vec3 vec3fInput;\n"
                "uniform vec3 vec3fInputArray[3];\n"
                "uniform vec4 vec4fInput;\n"
                "uniform vec4 vec4fInputArray[3];\n"

                "uniform mat2 matrix22fInput;\n"
                "uniform mat2 matrix22fInputArray[3];\n"
                "uniform mat3 matrix33fInput;\n"
                "uniform mat3 matrix33fInputArray[3];\n"
                "uniform mat4 matrix44fInput;\n"
                "uniform mat4 matrix44fInputArray[3];\n"

                "uniform lowp int integerInput;\n"
                "uniform lowp int integerInputArray[3];\n"

                "uniform ivec2 vec2iInput;\n"
                "uniform ivec2 vec2iInputArray[3];\n"
                "uniform ivec3 vec3iInput;\n"
                "uniform ivec3 vec3iInputArray[3];\n"
                "uniform ivec4 vec4iInput;\n"
                "uniform ivec4 vec4iInputArray[3];\n"

                "attribute float floatArrayInput;\n"
                "attribute vec2 vec2fArrayInput;\n"
                "attribute vec3 vec3fArrayInput;\n"
                "attribute vec4 vec4fArrayInput;\n"

                "void main(void)\n"
                "{\n"
                "   lowp vec4 values = vec4(1) * floatInput* vec2fInput.x * vec3fInput.y* vec4fInput.z;\n"
                "   values[0] = float(integerInput) + floatInputArray[0] + floatInputArray[1];\n"
                "   values[1] = float(vec2iInput.x)* float(vec3iInput.y)* float(vec4iInput.z);\n"
                "   values[2] = vec2fArrayInput.x*vec3fArrayInput.y*vec4fArrayInput.z;\n"
                "   values[3] = floatArrayInput*vec2fArrayInput.x*vec4fArrayInput.z;\n"
                "   values[0] += floatInputArray[0] + vec2fInputArray[0].x + vec3fInputArray[0].x + vec4fInputArray[0].x;\n"
                "   values[1] += matrix44fInputArray[0][0].x + float(integerInputArray[0]);\n"
                "   values[2] += float(vec2iInputArray[0].x + vec3iInputArray[0].x + vec4iInputArray[0].x);\n"
                "   values[3] += matrix22fInput[0][0] + matrix33fInput[0][0] + matrix22fInputArray[0][0].x + matrix33fInputArray[0][0].x;\n"
                "   values = matrix44fInput * values;\n"
                "   gl_Position = values;\n"
                "}\n");

            ramses_internal::String FragmentShader(
                "uniform sampler2D texture2dInput;\n"
                "uniform samplerCube textureCubeInput;\n"
                "void main(void)\n"
                "{\n"
                "    gl_FragColor = vec4(1.0) + texture2D(texture2dInput, vec2(0,0)) + textureCube(textureCubeInput, vec3(0,0,0));\n"
                "}\n");

            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(VertexShader.c_str());
            effectDesc.setFragmentShader(FragmentShader.c_str());

            if (withSemantics)
            {
                effectDesc.setAttributeSemantic("vec2fArrayInput", ramses::EEffectAttributeSemantic_TextPositions);
                effectDesc.setUniformSemantic("matrix44fInput", ramses::EEffectUniformSemantic_ModelViewMatrix);
                effectDesc.setUniformSemantic("texture2dInput", ramses::EEffectUniformSemantic_TextTexture);
            }

            return client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "input test effect");
        }

        Effect*     effect;
        Appearance* appearance;
    };
}

#endif
