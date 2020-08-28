//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-client-api/EffectDescription.h"
#include "EffectDescriptionImpl.h"
#include "EffectInputSemanticUtils.h"

using namespace testing;

namespace ramses
{
    class EffectDescriptionTest : public testing::Test
    {
    public:
    protected:
        EEffectUniformSemantic getSemanticForUniform(const char* inputName)
        {
            ramses_internal::EFixedSemantics* internalSemantic = effectDesc.impl.getSemanticsMap().get(inputName);
            if (internalSemantic == nullptr)
            {
                return EEffectUniformSemantic_Invalid;
            }

            return EffectInputSemanticUtils::GetEffectUniformSemanticFromInternal(*internalSemantic);
        }

        EEffectAttributeSemantic getSemanticForAttribute(const char* inputName)
        {
            ramses_internal::EFixedSemantics* internalSemantic = effectDesc.impl.getSemanticsMap().get(inputName);
            if (internalSemantic == nullptr)
            {
                return EEffectAttributeSemantic_Invalid;
            }

            return EffectInputSemanticUtils::GetEffectAttributeSemanticFromInternal(*internalSemantic);
        }

        EffectDescription effectDesc;
    };

    TEST_F(EffectDescriptionTest, initialState)
    {
        EXPECT_STREQ("", effectDesc.getVertexShader());
        EXPECT_STREQ("", effectDesc.getFragmentShader());
        EXPECT_STREQ("", effectDesc.getGeometryShader());
        EXPECT_EQ(0u, effectDesc.getNumberOfCompilerDefines());
        EXPECT_EQ(0u, effectDesc.impl.getSemanticsMap().size());
    }

    TEST_F(EffectDescriptionTest, setShaderWithNULLRetrievesEmptyString)
    {
        EXPECT_EQ(StatusOK, effectDesc.setVertexShader(nullptr));
        EXPECT_STREQ("", effectDesc.getVertexShader());
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShader(nullptr));
        EXPECT_STREQ("", effectDesc.getFragmentShader());
        EXPECT_EQ(StatusOK, effectDesc.setGeometryShader(nullptr));
        EXPECT_STREQ("", effectDesc.getGeometryShader());
    }

    TEST_F(EffectDescriptionTest, retrievesSetVertexShader)
    {
        const char* src = "code";
        EXPECT_EQ(StatusOK, effectDesc.setVertexShader(src));
        EXPECT_STREQ(src, effectDesc.getVertexShader());

        const char* src2 = "bad code";
        EXPECT_EQ(StatusOK, effectDesc.setVertexShader(src2));
        EXPECT_STREQ(src2, effectDesc.getVertexShader());
    }

    TEST_F(EffectDescriptionTest, retrievesSetFragmentShader)
    {
        const char* src = "code";
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShader(src));
        EXPECT_STREQ(src, effectDesc.getFragmentShader());

        const char* src2 = "bad code";
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShader(src2));
        EXPECT_STREQ(src2, effectDesc.getFragmentShader());
    }

    TEST_F(EffectDescriptionTest, retrievesSetGeometryShader)
    {
        const char* src = "code";
        EXPECT_EQ(StatusOK, effectDesc.setGeometryShader(src));
        EXPECT_STREQ(src, effectDesc.getGeometryShader());

        const char* src2 = "bad code";
        EXPECT_EQ(StatusOK, effectDesc.setGeometryShader(src2));
        EXPECT_STREQ(src2, effectDesc.getGeometryShader());
    }

    TEST_F(EffectDescriptionTest, addDefineAsNULLReportsError)
    {
        EXPECT_NE(StatusOK, effectDesc.addCompilerDefine(nullptr));
        EXPECT_EQ(nullptr, effectDesc.getCompilerDefine(0u));
        EXPECT_EQ(0u, effectDesc.getNumberOfCompilerDefines());
    }

    TEST_F(EffectDescriptionTest, canAddAndRetrieveDefineInOrder)
    {
        const char* def1 = "my define";
        const char* def2 = "my useless define";

        EXPECT_EQ(StatusOK, effectDesc.addCompilerDefine(def1));
        EXPECT_STREQ(def1, effectDesc.getCompilerDefine(0u));
        EXPECT_EQ(1u, effectDesc.getNumberOfCompilerDefines());

        EXPECT_EQ(StatusOK, effectDesc.addCompilerDefine(def2));
        EXPECT_STREQ(def1, effectDesc.getCompilerDefine(0u));
        EXPECT_STREQ(def2, effectDesc.getCompilerDefine(1u));
        EXPECT_EQ(2u, effectDesc.getNumberOfCompilerDefines());
    }

    TEST_F(EffectDescriptionTest, addSemanticNameAsNULLReportsError)
    {
        EXPECT_NE(StatusOK, effectDesc.setUniformSemantic(nullptr, EEffectUniformSemantic_CameraViewMatrix));
        EXPECT_EQ(0u, effectDesc.impl.getSemanticsMap().size());
    }

    TEST_F(EffectDescriptionTest, canAddAndRetrieveSemanticInput)
    {
        const char* semantic1 = "my_semantic";
        const char* semantic2 = "my_semantic2";
        const char* semantic3 = "my_semantic3";
        const EEffectUniformSemantic semanticType1 = EEffectUniformSemantic_CameraViewMatrix;
        const EEffectUniformSemantic semanticType2 = EEffectUniformSemantic_CameraViewMatrix;
        const EEffectAttributeSemantic semanticType3 = EEffectAttributeSemantic_TextPositions;

        EXPECT_EQ(StatusOK, effectDesc.setUniformSemantic(semantic1, semanticType1));
        EXPECT_EQ(semanticType1, getSemanticForUniform(semantic1));
        EXPECT_EQ(1u, effectDesc.impl.getSemanticsMap().size());

        EXPECT_EQ(StatusOK, effectDesc.setUniformSemantic(semantic2, semanticType2));
        EXPECT_EQ(semanticType1, getSemanticForUniform(semantic1));
        EXPECT_EQ(semanticType2, getSemanticForUniform(semantic2));
        EXPECT_EQ(2u, effectDesc.impl.getSemanticsMap().size());

        EXPECT_EQ(StatusOK, effectDesc.setAttributeSemantic(semantic3, semanticType3));
        EXPECT_EQ(semanticType1, getSemanticForUniform(semantic1));
        EXPECT_EQ(semanticType2, getSemanticForUniform(semantic2));
        EXPECT_EQ(semanticType3, getSemanticForAttribute(semantic3));
        EXPECT_EQ(3u, effectDesc.impl.getSemanticsMap().size());
    }

    TEST_F(EffectDescriptionTest, retrievesUnknownTypeForSemanticNotAdded)
    {
        const char* semantic1 = "my_semantic";
        EXPECT_EQ(EEffectUniformSemantic_Invalid, getSemanticForUniform(semantic1));
    }

    TEST_F(EffectDescriptionTest, setVertexShaderFromFile)
    {
        const char* fileName = "res/ramses-client-test_minimalShader.vert";
        EXPECT_EQ(StatusOK, effectDesc.setVertexShaderFromFile(fileName));
    }

    TEST_F(EffectDescriptionTest, setFragmentShaderFromFile)
    {
        const char* fileName = "res/ramses-client-test_minimalShader.frag";
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShaderFromFile(fileName));
    }

    TEST_F(EffectDescriptionTest, setGeometryShaderFromFile)
    {
        const char* fileName = "res/ramses-client-test_minimalShader.geom";
        EXPECT_EQ(StatusOK, effectDesc.setGeometryShaderFromFile(fileName));
    }

    TEST_F(EffectDescriptionTest, reportsErrorWhenLoadingNonexistingFile)
    {
        const char* fileName = "nofile%&";
        EXPECT_NE(StatusOK, effectDesc.setVertexShaderFromFile(fileName));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile(fileName));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile(fileName));
        EXPECT_NE(StatusOK, effectDesc.setVertexShaderFromFile(nullptr));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile(nullptr));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile(nullptr));
    }
}
