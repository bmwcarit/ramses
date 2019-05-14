//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectConfig.h"
#include "gtest/gtest.h"
#include "ramses-client-api/EffectDescription.h"
#include "EffectDescriptionImpl.h"
#include "EffectInputSemanticUtils.h"

TEST(AnEffectConfig, canLoadCorrectlyWhenAllLinesAreValid)
{
    EffectConfig config;
    EXPECT_TRUE(config.loadFromFile("res/ramses-utils-test.config"));

    ramses::EffectDescription description;
    config.fillEffectDescription(description);

    ramses::EffectDescriptionImpl descriptionImpl = description.impl;
    const uint32_t numCompilerDefines = descriptionImpl.getNumberOfCompilerDefines();
    EXPECT_EQ(2u, numCompilerDefines);

    typedef ramses_internal::StringVector StringVector;
    const StringVector& defines = descriptionImpl.getCompilerDefines();
    EXPECT_TRUE(contains_c(defines, "dummy"));
    EXPECT_TRUE(contains_c(defines, "dummy1"));

    typedef ramses::EffectDescriptionImpl::SemanticsMap SemanticsMap;
    const SemanticsMap& semantics = descriptionImpl.getSemanticsMap();

    SemanticsMap::Iterator semanticIter = semantics.find("matrix44fInput");
    ASSERT_TRUE(semanticIter != semantics.end());
    EXPECT_TRUE(ramses::EEffectUniformSemantic_ModelViewMatrix == ramses::EffectInputSemanticUtils::GetEffectUniformSemanticFromInternal(semanticIter->value));

    semanticIter = semantics.find("texture2dInput");
    ASSERT_TRUE(semanticIter != semantics.end());
    EXPECT_TRUE(ramses::EEffectUniformSemantic_TextTexture == ramses::EffectInputSemanticUtils::GetEffectUniformSemanticFromInternal(semanticIter->value));

    semanticIter = semantics.find("vec2fArrayInput");
    ASSERT_TRUE(semanticIter != semantics.end());
    EXPECT_TRUE(ramses::EEffectAttributeSemantic_TextPositions == ramses::EffectInputSemanticUtils::GetEffectAttributeSemanticFromInternal(semanticIter->value));
}

TEST(AnEffectConfig, isFineToLoadValidFiles)
{
    {
        EffectConfig config;
        EXPECT_TRUE(config.loadFromFile("res/ramses-utils-empty.config"));
    }
    {
        EffectConfig config;
        EXPECT_TRUE(config.loadFromFile("res/ramses-utils-valid-with-additional-spaces-and-empty-lines.config"));
    }
}

TEST(AnEffectConfig, reportsErrorWhenLoadANonExistFile)
{
    EffectConfig config;
    EXPECT_FALSE(config.loadFromFile("res/ramses-utils-nonexist.config"));
}

TEST(AnEffectConfig, reportsErrorWhenLoadAFileContainingInvalidUniformSemanticDefine)
{
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-uniform-semantic-invalid-keyword.config"));
    }
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-uniform-semantic-invalid-semantic.config"));
    }
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-uniform-semantic-invalid-num-tokens.config"));
    }
}

TEST(AnEffectConfig, reportsErrorWhenLoadAFileContainingInvalidAttributeSemanticDefine)
{
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-attribute-semantic-invalid-keyword.config"));
    }
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-attribute-semantic-invalid-semantic.config"));
    }
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-attribute-semantic-invalid-num-tokens.config"));
    }
}

TEST(AnEffectConfig, reportsErrorWhenLoadAFileContainingInvalidCompilerDefine)
{
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-comipler-define-invalid-keyword.config"));
    }
    {
        EffectConfig config;
        EXPECT_FALSE(config.loadFromFile("res/ramses-utils-invalid-comipler-define-invalid-num-tokens.config"));
    }
}
