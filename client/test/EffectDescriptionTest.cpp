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

#include <string_view>

using namespace testing;

namespace ramses
{
    class EffectDescriptionTest : public testing::Test
    {
    public:
    protected:
        EEffectUniformSemantic getSemanticForUniform(std::string_view inputName)
        {
            ramses_internal::EFixedSemantics* internalSemantic = effectDesc.m_impl.get().getSemanticsMap().get(std::string{inputName});
            if (internalSemantic == nullptr)
            {
                return EEffectUniformSemantic::Invalid;
            }

            return EffectInputSemanticUtils::GetEffectUniformSemanticFromInternal(*internalSemantic);
        }

        EEffectAttributeSemantic getSemanticForAttribute(std::string_view inputName)
        {
            ramses_internal::EFixedSemantics* internalSemantic = effectDesc.m_impl.get().getSemanticsMap().get(std::string{inputName});
            if (internalSemantic == nullptr)
            {
                return EEffectAttributeSemantic::Invalid;
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
        EXPECT_EQ(0u, effectDesc.m_impl.get().getSemanticsMap().size());
    }

    TEST_F(EffectDescriptionTest, setShaderWithNULLRetrievesEmptyString)
    {
        EXPECT_EQ(StatusOK, effectDesc.setVertexShader({}));
        EXPECT_STREQ("", effectDesc.getVertexShader());
        EXPECT_EQ(StatusOK, effectDesc.setFragmentShader({}));
        EXPECT_STREQ("", effectDesc.getFragmentShader());
        EXPECT_EQ(StatusOK, effectDesc.setGeometryShader({}));
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
        EXPECT_NE(StatusOK, effectDesc.addCompilerDefine({}));
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
        EXPECT_NE(StatusOK, effectDesc.setUniformSemantic({}, EEffectUniformSemantic::ViewMatrix));
        EXPECT_EQ(0u, effectDesc.m_impl.get().getSemanticsMap().size());
    }

    TEST_F(EffectDescriptionTest, canAddAndRetrieveSemanticInput)
    {
        const char* semantic1 = "my_semantic";
        const char* semantic2 = "my_semantic2";
        const char* semantic3 = "my_semantic3";
        const EEffectUniformSemantic semanticType1 = EEffectUniformSemantic::ViewMatrix;
        const EEffectUniformSemantic semanticType2 = EEffectUniformSemantic::ViewMatrix;
        const EEffectAttributeSemantic semanticType3 = EEffectAttributeSemantic::TextPositions;

        EXPECT_EQ(StatusOK, effectDesc.setUniformSemantic(semantic1, semanticType1));
        EXPECT_EQ(semanticType1, getSemanticForUniform(semantic1));
        EXPECT_EQ(1u, effectDesc.m_impl.get().getSemanticsMap().size());

        EXPECT_EQ(StatusOK, effectDesc.setUniformSemantic(semantic2, semanticType2));
        EXPECT_EQ(semanticType1, getSemanticForUniform(semantic1));
        EXPECT_EQ(semanticType2, getSemanticForUniform(semantic2));
        EXPECT_EQ(2u, effectDesc.m_impl.get().getSemanticsMap().size());

        EXPECT_EQ(StatusOK, effectDesc.setAttributeSemantic(semantic3, semanticType3));
        EXPECT_EQ(semanticType1, getSemanticForUniform(semantic1));
        EXPECT_EQ(semanticType2, getSemanticForUniform(semantic2));
        EXPECT_EQ(semanticType3, getSemanticForAttribute(semantic3));
        EXPECT_EQ(3u, effectDesc.m_impl.get().getSemanticsMap().size());
    }

    TEST_F(EffectDescriptionTest, retrievesUnknownTypeForSemanticNotAdded)
    {
        const char* semantic1 = "my_semantic";
        EXPECT_EQ(EEffectUniformSemantic::Invalid, getSemanticForUniform(semantic1));
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
        const auto fileName = "nofile%&";
        EXPECT_NE(StatusOK, effectDesc.setVertexShaderFromFile(fileName));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile(fileName));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile(fileName));
        EXPECT_NE(StatusOK, effectDesc.setVertexShaderFromFile({}));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile({}));
        EXPECT_NE(StatusOK, effectDesc.setFragmentShaderFromFile({}));
    }

    TEST_F(EffectDescriptionTest, CanBeCopyAndMoveConstructed)
    {
        ASSERT_EQ(StatusOK, effectDesc.setVertexShader("test"));

        EffectDescription effectDescCopy{ effectDesc };
        EXPECT_STREQ("test", effectDescCopy.getVertexShader());

        EffectDescription effectDescMove{ std::move(effectDesc) };
        EXPECT_STREQ("test", effectDescMove.getVertexShader());
    }

    TEST_F(EffectDescriptionTest, CanBeCopyAndMoveAssigned)
    {
        ASSERT_EQ(StatusOK, effectDesc.setVertexShader("test"));

        EffectDescription effectDescCopy;
        effectDescCopy = effectDesc;
        EXPECT_STREQ("test", effectDescCopy.getVertexShader());

        EffectDescription effectDescMove;
        effectDescMove = std::move(effectDesc);
        EXPECT_STREQ("test", effectDescMove.getVertexShader());
    }

    TEST_F(EffectDescriptionTest, CanBeSelfAssigned)
    {
        ASSERT_EQ(StatusOK, effectDesc.setVertexShader("test"));

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        effectDesc = effectDesc;
        EXPECT_STREQ("test", effectDesc.getVertexShader());
        effectDesc = std::move(effectDesc);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        EXPECT_STREQ("test", effectDesc.getVertexShader());
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
}
