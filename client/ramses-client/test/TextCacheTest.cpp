//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text-api/TextCache.h"
#include "ramses-text-api/TextLine.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/IFontInstance.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/ArrayBuffer.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-utils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace ramses
{
    bool operator==(const GlyphMetrics& a, const GlyphMetrics& b)
    {
        return a.key == b.key
            && a.posX == b.posX
            && a.posY == b.posY
            && a.width == b.width
            && a.height == b.height
            && a.advance == b.advance;
    }

    class ATextCache :  public testing::Test
    {
    public:
        ATextCache()
            : m_client(*m_framework.createClient("test"))
            , m_scene(*m_client.createScene(sceneId_t(123)))
            , m_textCache(m_scene, *FRegistry, 64u, 64u)
        {
        }

        static void SetUpTestCase()
        {
            FRegistry = new FontRegistry;
            LatinFont = FRegistry->createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
            LatinFontInstance12 = FRegistry->createFreetype2FontInstance(LatinFont, 12);
            LatinFontInstance20 = FRegistry->createFreetype2FontInstance(LatinFont, 20);
        }

        static void TearDownTestCase()
        {
            delete FRegistry;
        }

    protected:

        Effect* createTestEffect(Scene& scene)
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "precision highp float;\n"
                "attribute vec2 a_position; \n"
                "attribute vec2 a_texcoord; \n"
                "\n"
                "varying vec2 v_texcoord; \n"
                "\n"
                "void main()\n"
                "{\n"
                "  v_texcoord = a_texcoord; \n"
                "  gl_Position = vec4(a_position, 0.0, 1.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "precision highp float;\n"
                "uniform sampler2D u_texture; \n"
                "varying vec2 v_texcoord; \n"
                "\n"
                "void main(void)\n"
                "{\n"
                "  float a = texture2D(u_texture, v_texcoord).r; \n"
                "  gl_FragColor = vec4(a, a, a, a); \n"
                "}\n");

            effectDesc.setAttributeSemantic("a_position", EEffectAttributeSemantic_TextPositions);
            effectDesc.setAttributeSemantic("a_texcoord", EEffectAttributeSemantic_TextTextureCoordinates);
            effectDesc.setUniformSemantic("u_texture", EEffectUniformSemantic_TextTexture);

            Effect* effect = scene.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, "");
            return effect;
        }

        RamsesFramework m_framework;
        RamsesClient& m_client;
        Scene& m_scene;

        TextCache m_textCache;

        static FontRegistry*    FRegistry;
        static FontId           LatinFont;
        static FontInstanceId   LatinFontInstance12;
        static FontInstanceId   LatinFontInstance20;
    };

    FontRegistry*    ATextCache::FRegistry(nullptr);
    FontId           ATextCache::LatinFont;
    FontInstanceId   ATextCache::LatinFontInstance12;
    FontInstanceId   ATextCache::LatinFontInstance20;

    TEST_F(ATextCache, getsPositionedGlyphs)
    {
        const std::u32string str = U" test ";
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, LatinFontInstance12);

        ASSERT_EQ(6u, positionedGlyphs.size());

        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[0].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[1].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[2].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[3].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[4].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[5].key.fontInstanceId);

        EXPECT_EQ(4u, positionedGlyphs[0].key.identifier.getValue());
        EXPECT_EQ(88u, positionedGlyphs[1].key.identifier.getValue());
        EXPECT_EQ(73u, positionedGlyphs[2].key.identifier.getValue());
        EXPECT_EQ(87u, positionedGlyphs[3].key.identifier.getValue());
        EXPECT_EQ(88u, positionedGlyphs[4].key.identifier.getValue());
        EXPECT_EQ(4u, positionedGlyphs[5].key.identifier.getValue());

        EXPECT_EQ(0u, positionedGlyphs[0].width);
        EXPECT_EQ(4u, positionedGlyphs[1].width);
        EXPECT_EQ(7u, positionedGlyphs[2].width);
        EXPECT_EQ(6u, positionedGlyphs[3].width);
        EXPECT_EQ(4u, positionedGlyphs[4].width);
        EXPECT_EQ(0u, positionedGlyphs[5].width);

        EXPECT_EQ(0u, positionedGlyphs[0].height);
        EXPECT_EQ(9u, positionedGlyphs[1].height);
        EXPECT_EQ(7u, positionedGlyphs[2].height);
        EXPECT_EQ(7u, positionedGlyphs[3].height);
        EXPECT_EQ(9u, positionedGlyphs[4].height);
        EXPECT_EQ(0u, positionedGlyphs[5].height);

        EXPECT_EQ(0, positionedGlyphs[0].posX);
        EXPECT_EQ(0, positionedGlyphs[1].posX);
        EXPECT_EQ(0, positionedGlyphs[2].posX);
        EXPECT_EQ(0, positionedGlyphs[3].posX);
        EXPECT_EQ(0, positionedGlyphs[4].posX);
        EXPECT_EQ(0, positionedGlyphs[5].posX);

        EXPECT_EQ(0, positionedGlyphs[0].posY);
        EXPECT_EQ(0, positionedGlyphs[1].posY);
        EXPECT_EQ(0, positionedGlyphs[2].posY);
        EXPECT_EQ(0, positionedGlyphs[3].posY);
        EXPECT_EQ(0, positionedGlyphs[4].posY);
        EXPECT_EQ(0, positionedGlyphs[5].posY);

        EXPECT_EQ(3, positionedGlyphs[0].advance);
        EXPECT_EQ(4, positionedGlyphs[1].advance);
        EXPECT_EQ(6, positionedGlyphs[2].advance);
        EXPECT_EQ(6, positionedGlyphs[3].advance);
        EXPECT_EQ(4, positionedGlyphs[4].advance);
        EXPECT_EQ(3, positionedGlyphs[5].advance);
    }

    TEST_F(ATextCache, getsPositionedGlyphsWithMultipleFontsUsingFontOffsets)
    {
        const std::u32string str = U"<ignore> test abcd ";
        const FontInstanceOffsets fontOffsets = { { LatinFontInstance12, 8u }, { LatinFontInstance20, 14u } };
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        ASSERT_EQ(11u, positionedGlyphs.size());

        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[0].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[1].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[2].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[3].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[4].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[5].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[6].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[7].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[8].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[9].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[10].key.fontInstanceId);

        EXPECT_EQ(4u, positionedGlyphs[0].key.identifier.getValue());
        EXPECT_EQ(88u, positionedGlyphs[1].key.identifier.getValue());
        EXPECT_EQ(73u, positionedGlyphs[2].key.identifier.getValue());
        EXPECT_EQ(87u, positionedGlyphs[3].key.identifier.getValue());
        EXPECT_EQ(88u, positionedGlyphs[4].key.identifier.getValue());
        EXPECT_EQ(4u, positionedGlyphs[5].key.identifier.getValue());
        EXPECT_EQ(69u, positionedGlyphs[6].key.identifier.getValue());
        EXPECT_EQ(70u, positionedGlyphs[7].key.identifier.getValue());
        EXPECT_EQ(71u, positionedGlyphs[8].key.identifier.getValue());
        EXPECT_EQ(72u, positionedGlyphs[9].key.identifier.getValue());
        EXPECT_EQ(4u, positionedGlyphs[10].key.identifier.getValue());

        EXPECT_EQ(0u, positionedGlyphs[0].width);
        EXPECT_EQ(4u, positionedGlyphs[1].width);
        EXPECT_EQ(7u, positionedGlyphs[2].width);
        EXPECT_EQ(6u, positionedGlyphs[3].width);
        EXPECT_EQ(4u, positionedGlyphs[4].width);
        EXPECT_EQ(0u, positionedGlyphs[5].width);
        EXPECT_EQ(11u, positionedGlyphs[6].width);
        EXPECT_EQ(10u, positionedGlyphs[7].width);
        EXPECT_EQ(10u, positionedGlyphs[8].width);
        EXPECT_EQ(11u, positionedGlyphs[9].width);
        EXPECT_EQ(0u, positionedGlyphs[10].width);

        EXPECT_EQ(0u, positionedGlyphs[0].height);
        EXPECT_EQ(9u, positionedGlyphs[1].height);
        EXPECT_EQ(7u, positionedGlyphs[2].height);
        EXPECT_EQ(7u, positionedGlyphs[3].height);
        EXPECT_EQ(9u, positionedGlyphs[4].height);
        EXPECT_EQ(0u, positionedGlyphs[5].height);
        EXPECT_EQ(11u, positionedGlyphs[6].height);
        EXPECT_EQ(15u, positionedGlyphs[7].height);
        EXPECT_EQ(11u, positionedGlyphs[8].height);
        EXPECT_EQ(15u, positionedGlyphs[9].height);
        EXPECT_EQ(0u, positionedGlyphs[10].height);

        EXPECT_EQ(0, positionedGlyphs[0].posX);
        EXPECT_EQ(0, positionedGlyphs[1].posX);
        EXPECT_EQ(0, positionedGlyphs[2].posX);
        EXPECT_EQ(0, positionedGlyphs[3].posX);
        EXPECT_EQ(0, positionedGlyphs[4].posX);
        EXPECT_EQ(0, positionedGlyphs[5].posX);
        EXPECT_EQ(0, positionedGlyphs[6].posX);
        EXPECT_EQ(1, positionedGlyphs[7].posX);
        EXPECT_EQ(0, positionedGlyphs[8].posX);
        EXPECT_EQ(0, positionedGlyphs[9].posX);
        EXPECT_EQ(0, positionedGlyphs[10].posX);

        EXPECT_EQ(0, positionedGlyphs[0].posY);
        EXPECT_EQ(0, positionedGlyphs[1].posY);
        EXPECT_EQ(0, positionedGlyphs[2].posY);
        EXPECT_EQ(0, positionedGlyphs[3].posY);
        EXPECT_EQ(0, positionedGlyphs[4].posY);
        EXPECT_EQ(0, positionedGlyphs[5].posY);
        EXPECT_EQ(0, positionedGlyphs[6].posY);
        EXPECT_EQ(0, positionedGlyphs[7].posY);
        EXPECT_EQ(0, positionedGlyphs[8].posY);
        EXPECT_EQ(0, positionedGlyphs[9].posY);
        EXPECT_EQ(0, positionedGlyphs[10].posY);

        EXPECT_EQ(3, positionedGlyphs[0].advance);
        EXPECT_EQ(4, positionedGlyphs[1].advance);
        EXPECT_EQ(6, positionedGlyphs[2].advance);
        EXPECT_EQ(6, positionedGlyphs[3].advance);
        EXPECT_EQ(4, positionedGlyphs[4].advance);
        EXPECT_EQ(3, positionedGlyphs[5].advance);
        EXPECT_EQ(11, positionedGlyphs[6].advance);
        EXPECT_EQ(11, positionedGlyphs[7].advance);
        EXPECT_EQ(10, positionedGlyphs[8].advance);
        EXPECT_EQ(11, positionedGlyphs[9].advance);
        EXPECT_EQ(5, positionedGlyphs[10].advance);
    }

    TEST_F(ATextCache, getsPositionedGlyphsWithMultipleFontsInlcudingInvalidFontUsingFontOffsets)
    {
        const std::u32string str = U"<ignore> test abcd ";
        const FontInstanceOffsets fontOffsets = { { FontInstanceId::Invalid(), 0u },{ LatinFontInstance12, 8u },{ LatinFontInstance20, 14u } };
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        ASSERT_EQ(11u, positionedGlyphs.size());

        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[0].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[1].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[2].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[3].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[4].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance12, positionedGlyphs[5].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[6].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[7].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[8].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[9].key.fontInstanceId);
        EXPECT_EQ(LatinFontInstance20, positionedGlyphs[10].key.fontInstanceId);

        EXPECT_EQ(4u, positionedGlyphs[0].key.identifier.getValue());
        EXPECT_EQ(88u, positionedGlyphs[1].key.identifier.getValue());
        EXPECT_EQ(73u, positionedGlyphs[2].key.identifier.getValue());
        EXPECT_EQ(87u, positionedGlyphs[3].key.identifier.getValue());
        EXPECT_EQ(88u, positionedGlyphs[4].key.identifier.getValue());
        EXPECT_EQ(4u, positionedGlyphs[5].key.identifier.getValue());
        EXPECT_EQ(69u, positionedGlyphs[6].key.identifier.getValue());
        EXPECT_EQ(70u, positionedGlyphs[7].key.identifier.getValue());
        EXPECT_EQ(71u, positionedGlyphs[8].key.identifier.getValue());
        EXPECT_EQ(72u, positionedGlyphs[9].key.identifier.getValue());
        EXPECT_EQ(4u, positionedGlyphs[10].key.identifier.getValue());

        EXPECT_EQ(0u, positionedGlyphs[0].width);
        EXPECT_EQ(4u, positionedGlyphs[1].width);
        EXPECT_EQ(7u, positionedGlyphs[2].width);
        EXPECT_EQ(6u, positionedGlyphs[3].width);
        EXPECT_EQ(4u, positionedGlyphs[4].width);
        EXPECT_EQ(0u, positionedGlyphs[5].width);
        EXPECT_EQ(11u, positionedGlyphs[6].width);
        EXPECT_EQ(10u, positionedGlyphs[7].width);
        EXPECT_EQ(10u, positionedGlyphs[8].width);
        EXPECT_EQ(11u, positionedGlyphs[9].width);
        EXPECT_EQ(0u, positionedGlyphs[10].width);

        EXPECT_EQ(0u, positionedGlyphs[0].height);
        EXPECT_EQ(9u, positionedGlyphs[1].height);
        EXPECT_EQ(7u, positionedGlyphs[2].height);
        EXPECT_EQ(7u, positionedGlyphs[3].height);
        EXPECT_EQ(9u, positionedGlyphs[4].height);
        EXPECT_EQ(0u, positionedGlyphs[5].height);
        EXPECT_EQ(11u, positionedGlyphs[6].height);
        EXPECT_EQ(15u, positionedGlyphs[7].height);
        EXPECT_EQ(11u, positionedGlyphs[8].height);
        EXPECT_EQ(15u, positionedGlyphs[9].height);
        EXPECT_EQ(0u, positionedGlyphs[10].height);

        EXPECT_EQ(0, positionedGlyphs[0].posX);
        EXPECT_EQ(0, positionedGlyphs[1].posX);
        EXPECT_EQ(0, positionedGlyphs[2].posX);
        EXPECT_EQ(0, positionedGlyphs[3].posX);
        EXPECT_EQ(0, positionedGlyphs[4].posX);
        EXPECT_EQ(0, positionedGlyphs[5].posX);
        EXPECT_EQ(0, positionedGlyphs[6].posX);
        EXPECT_EQ(1, positionedGlyphs[7].posX);
        EXPECT_EQ(0, positionedGlyphs[8].posX);
        EXPECT_EQ(0, positionedGlyphs[9].posX);
        EXPECT_EQ(0, positionedGlyphs[10].posX);

        EXPECT_EQ(0, positionedGlyphs[0].posY);
        EXPECT_EQ(0, positionedGlyphs[1].posY);
        EXPECT_EQ(0, positionedGlyphs[2].posY);
        EXPECT_EQ(0, positionedGlyphs[3].posY);
        EXPECT_EQ(0, positionedGlyphs[4].posY);
        EXPECT_EQ(0, positionedGlyphs[5].posY);
        EXPECT_EQ(0, positionedGlyphs[6].posY);
        EXPECT_EQ(0, positionedGlyphs[7].posY);
        EXPECT_EQ(0, positionedGlyphs[8].posY);
        EXPECT_EQ(0, positionedGlyphs[9].posY);
        EXPECT_EQ(0, positionedGlyphs[10].posY);

        EXPECT_EQ(3, positionedGlyphs[0].advance);
        EXPECT_EQ(4, positionedGlyphs[1].advance);
        EXPECT_EQ(6, positionedGlyphs[2].advance);
        EXPECT_EQ(6, positionedGlyphs[3].advance);
        EXPECT_EQ(4, positionedGlyphs[4].advance);
        EXPECT_EQ(3, positionedGlyphs[5].advance);
        EXPECT_EQ(11, positionedGlyphs[6].advance);
        EXPECT_EQ(11, positionedGlyphs[7].advance);
        EXPECT_EQ(10, positionedGlyphs[8].advance);
        EXPECT_EQ(11, positionedGlyphs[9].advance);
        EXPECT_EQ(5, positionedGlyphs[10].advance);
    }

    TEST_F(ATextCache, getsNullTextLineForNonExistingTextLineId)
    {
        EXPECT_EQ(nullptr, m_textCache.getTextLine(TextLineId::Invalid()));
        EXPECT_EQ(nullptr, m_textCache.getTextLine(TextLineId(3u)));
        const TextCache& constTextCache = m_textCache;
        EXPECT_EQ(nullptr, constTextCache.getTextLine(TextLineId::Invalid()));
        EXPECT_EQ(nullptr, constTextCache.getTextLine(TextLineId(3u)));
    }

    TEST_F(ATextCache, failsToDeleteNonExistingTextLineId)
    {
        EXPECT_FALSE(m_textCache.deleteTextLine(TextLineId::Invalid()));
        EXPECT_FALSE(m_textCache.deleteTextLine(TextLineId(3u)));
    }

    TEST_F(ATextCache, createsTextLine)
    {
        const std::u32string str = U" test ";
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, LatinFontInstance12);

        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        const TextLineId textLineId = m_textCache.createTextLine(positionedGlyphs, *textEffect);
        EXPECT_TRUE(textLineId.isValid());
        const TextLine* textLine = m_textCache.getTextLine(textLineId);
        ASSERT_TRUE(textLine != nullptr);

        EXPECT_EQ(positionedGlyphs, textLine->glyphs);
        const MeshNode* meshNode = textLine->meshNode;
        ASSERT_TRUE(meshNode != nullptr);
        EXPECT_NE(nullptr, meshNode->getAppearance());
        EXPECT_NE(nullptr, meshNode->getGeometryBinding());
        EXPECT_EQ(24u, meshNode->getIndexCount());
        EXPECT_NE(std::numeric_limits<decltype(textLine->atlasPage)>::max(), textLine->atlasPage);
        ASSERT_TRUE(textLine->indices != nullptr);
        EXPECT_EQ(24u, textLine->indices->getUsedNumberOfElements());
        ASSERT_TRUE(textLine->positions != nullptr);
        EXPECT_EQ(16u, textLine->positions->getUsedNumberOfElements());
        ASSERT_TRUE(textLine->textureCoordinates != nullptr);
        EXPECT_EQ(16u, textLine->textureCoordinates->getUsedNumberOfElements());
    }

    TEST_F(ATextCache, createsMultipleTextLines)
    {
        const auto positionedGlyphs1 = m_textCache.getPositionedGlyphs(U" test ", LatinFontInstance12);
        const auto positionedGlyphs2 = m_textCache.getPositionedGlyphs(U"123abc", LatinFontInstance20);

        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        const TextLineId textLineId1 = m_textCache.createTextLine(positionedGlyphs1, *textEffect);
        const TextLineId textLineId2 = m_textCache.createTextLine(positionedGlyphs2, *textEffect);
        EXPECT_TRUE(textLineId1.isValid());
        EXPECT_TRUE(textLineId2.isValid());
        const TextLine* textLine1 = m_textCache.getTextLine(textLineId1);
        const TextLine* textLine2 = m_textCache.getTextLine(textLineId2);
        ASSERT_TRUE(textLine1 != nullptr);
        ASSERT_TRUE(textLine2 != nullptr);

        EXPECT_EQ(positionedGlyphs1, textLine1->glyphs);
        EXPECT_EQ(positionedGlyphs2, textLine2->glyphs);
        const MeshNode* meshNode1 = textLine1->meshNode;
        const MeshNode* meshNode2 = textLine2->meshNode;
        ASSERT_TRUE(meshNode1 != nullptr);
        ASSERT_TRUE(meshNode2 != nullptr);
        EXPECT_NE(nullptr, meshNode1->getAppearance());
        EXPECT_NE(nullptr, meshNode2->getAppearance());
        EXPECT_NE(nullptr, meshNode1->getGeometryBinding());
        EXPECT_NE(nullptr, meshNode2->getGeometryBinding());
        EXPECT_EQ(24u, meshNode1->getIndexCount());
        EXPECT_EQ(36u, meshNode2->getIndexCount());
        EXPECT_NE(std::numeric_limits<decltype(textLine1->atlasPage)>::max(), textLine1->atlasPage);
        EXPECT_NE(std::numeric_limits<decltype(textLine2->atlasPage)>::max(), textLine2->atlasPage);
        ASSERT_TRUE(textLine1->indices != nullptr);
        ASSERT_TRUE(textLine2->indices != nullptr);
        EXPECT_EQ(24u, textLine1->indices->getUsedNumberOfElements());
        EXPECT_EQ(36u, textLine2->indices->getUsedNumberOfElements());
        ASSERT_TRUE(textLine1->positions != nullptr);
        ASSERT_TRUE(textLine2->positions != nullptr);
        EXPECT_EQ(16u, textLine1->positions->getUsedNumberOfElements());
        EXPECT_EQ(24u, textLine2->positions->getUsedNumberOfElements());
        ASSERT_TRUE(textLine1->textureCoordinates != nullptr);
        ASSERT_TRUE(textLine2->textureCoordinates != nullptr);
        EXPECT_EQ(16u, textLine1->textureCoordinates->getUsedNumberOfElements());
        EXPECT_EQ(24u, textLine2->textureCoordinates->getUsedNumberOfElements());
    }

    TEST_F(ATextCache, deletesTextLine)
    {
        const auto positionedGlyphs1 = m_textCache.getPositionedGlyphs(U" test ", LatinFontInstance12);

        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        const TextLineId textLineId1 = m_textCache.createTextLine(positionedGlyphs1, *textEffect);
        EXPECT_TRUE(m_textCache.deleteTextLine(textLineId1));
    }

    TEST_F(ATextCache, failsToDoubleDeleteTextLine)
    {
        const auto positionedGlyphs1 = m_textCache.getPositionedGlyphs(U" test ", LatinFontInstance12);

        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        const TextLineId textLineId1 = m_textCache.createTextLine(positionedGlyphs1, *textEffect);
        EXPECT_TRUE(m_textCache.deleteTextLine(textLineId1));
        EXPECT_FALSE(m_textCache.deleteTextLine(textLineId1));
    }

    TEST_F(ATextCache, deletesMultipleTextLines)
    {
        const auto positionedGlyphs1 = m_textCache.getPositionedGlyphs(U" test ", LatinFontInstance12);
        const auto positionedGlyphs2 = m_textCache.getPositionedGlyphs(U"123abc", LatinFontInstance20);

        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        const TextLineId textLineId1 = m_textCache.createTextLine(positionedGlyphs1, *textEffect);
        const TextLineId textLineId2 = m_textCache.createTextLine(positionedGlyphs2, *textEffect);
        const auto textLine2 = m_textCache.getTextLine(textLineId2);

        EXPECT_TRUE(m_textCache.deleteTextLine(textLineId1));
        EXPECT_EQ(nullptr, m_textCache.getTextLine(textLineId1));
        // can still get line2
        EXPECT_EQ(textLine2, m_textCache.getTextLine(textLineId2));
        EXPECT_TRUE(m_textCache.deleteTextLine(textLineId2));
        EXPECT_EQ(nullptr, m_textCache.getTextLine(textLineId2));
    }

    TEST_F(ATextCache, failsToCreateTextLineFromEmptyString)
    {
        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        EXPECT_FALSE(m_textCache.createTextLine({}, *textEffect).isValid());
    }

    TEST_F(ATextCache, failsToCreateTextLineUsingNonTextEffect)
    {
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(U"x", LatinFontInstance12);

        EffectDescription effectDesc;
        effectDesc.setVertexShader("void main() { gl_Position = vec4(1.0, 0.0, 0.0, 1.0); }\n");
        effectDesc.setFragmentShader("void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }\n");
        Effect* effect = m_scene.createEffect(effectDesc);
        ASSERT_TRUE(effect != nullptr);

        EXPECT_FALSE(m_textCache.createTextLine(positionedGlyphs, *effect).isValid());
    }

    TEST_F(ATextCache, failsToCreateTextLineIfFontInstanceNeededForAGlyphIsNotAvailable)
    {
        const std::u32string str = U" test ";
        auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, LatinFontInstance12);

        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        positionedGlyphs.back().key.fontInstanceId = FontInstanceId(999u);
        EXPECT_FALSE(m_textCache.createTextLine(positionedGlyphs, *textEffect).isValid());
    }

    TEST_F(ATextCache, failsToCreateTextLineIfStringDoesNotFitToAtlas)
    {
        const std::u32string str = U"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, LatinFontInstance20);

        Effect* textEffect = createTestEffect(m_scene);
        ASSERT_TRUE(textEffect != nullptr);

        EXPECT_FALSE(m_textCache.createTextLine(positionedGlyphs, *textEffect).isValid());
    }

    TEST_F(ATextCache, failsToCreateTextLineIfEffectComesFromAnotherScene)
    {
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(U"x", LatinFontInstance20);

        Scene& otherScene(*m_client.createScene(sceneId_t{ 111 }));

        Effect* textEffect = createTestEffect(otherScene);
        ASSERT_TRUE(textEffect != nullptr);

        EXPECT_FALSE(m_textCache.createTextLine(positionedGlyphs, *textEffect).isValid());
    }
}
