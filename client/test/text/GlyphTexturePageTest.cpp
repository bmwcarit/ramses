//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text/GlyphTexturePage.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "gtest/gtest.h"

namespace ramses
{
    class AGlyphTexturePage : public testing::Test
    {
    public:
        static constexpr uint32_t PageWidth = 12;
        static constexpr uint32_t PageHeight = 16;

        AGlyphTexturePage();
        ~AGlyphTexturePage() override
        {
            delete m_glyphPage;
        }

    protected:
        RamsesFramework m_framework;
        RamsesClient& m_client;
        Scene& m_scene;
        GlyphTexturePage* m_glyphPage;

        void expectFreeArea(uint32_t expected)
        {
            uint32_t area = 0;
            for (auto const& quad : m_glyphPage->getFreeSpace())
            {
                area += quad.getSize().getArea();
            }
            EXPECT_EQ(area, expected);
        }

        enum class EClaimedQuadPosition
        {
            TopLeft = 0,
            TopRight,
            BottomLeft,
            BottomRight
        };

        enum class EClaimedQuadSize
        {
            FullPage = 0,
            HalfPage,
            QuarterPage,
            SinglePixel
        };

        [[nodiscard]] GlyphTexturePage::QuadIndex getFittingFreeQuadIndex(EClaimedQuadSize size) const
        {
            QuadSize quadSize = getQuadSize(size);
            GlyphTexturePage::QuadIndex i(0);
            for (auto const& quad : m_glyphPage->getFreeSpace())
            {
                if (quad.getSize().y >= quadSize.y && quad.getSize().x >= quadSize.x)
                {
                    return i;
                }
                i++;
            }
            return std::numeric_limits<GlyphTexturePage::QuadIndex>::max();
        }

        [[nodiscard]] bool canClaimQuad(EClaimedQuadSize size) const
        {
            return getFittingFreeQuadIndex(size) != std::numeric_limits<GlyphTexturePage::QuadIndex>::max();
        }

        [[nodiscard]] QuadSize getQuadSize(EClaimedQuadSize size) const
        {
            switch (size)
            {
            case EClaimedQuadSize::FullPage:
                return QuadSize(PageWidth, PageHeight);
            case EClaimedQuadSize::HalfPage:
                return QuadSize(PageWidth / 2, PageHeight);
            case EClaimedQuadSize::QuarterPage:
                return QuadSize(PageWidth / 2, PageHeight / 2);
            case EClaimedQuadSize::SinglePixel:
                return QuadSize(1, 1);
            default:
                assert(false);
                return QuadSize(0, 0);
            }
        }

        Quad claimTestQuad(EClaimedQuadSize size)
        {
            QuadSize quadSize = getQuadSize(size);
            auto freePlace = getFittingFreeQuadIndex(size);
            EXPECT_NE(freePlace, std::numeric_limits<GlyphTexturePage::QuadIndex>::max());
            return Quad(m_glyphPage->claimSpace(freePlace, quadSize), quadSize);
        }

        void expectQuadPlacing(Quad quad, EClaimedQuadPosition expectedPosition, EClaimedQuadSize expectedSize) const
        {
            switch (expectedPosition)
            {
            case EClaimedQuadPosition::TopLeft:
                EXPECT_EQ(quad.getOrigin().x, 0u);
                EXPECT_EQ(quad.getOrigin().y, 0u);
                break;
            case EClaimedQuadPosition::TopRight:
                EXPECT_EQ(quad.getOrigin().x, PageWidth / 2);
                EXPECT_EQ(quad.getOrigin().y, 0u);
                break;
            case EClaimedQuadPosition::BottomLeft:
                EXPECT_EQ(quad.getOrigin().x, 0u);
                EXPECT_EQ(quad.getOrigin().y, PageHeight / 2);
                break;
            case EClaimedQuadPosition::BottomRight:
                EXPECT_EQ(quad.getOrigin().x, PageWidth / 2);
                EXPECT_EQ(quad.getOrigin().y, PageHeight / 2);
                break;
            default:
                assert(false);
            }

            switch (expectedSize)
            {
            case EClaimedQuadSize::FullPage:
                EXPECT_EQ(quad.getSize().x, PageWidth);
                EXPECT_EQ(quad.getSize().y, PageHeight);
                break;
            case EClaimedQuadSize::HalfPage:
                EXPECT_EQ(quad.getSize().x, PageWidth / 2);
                EXPECT_EQ(quad.getSize().y, PageHeight);
                break;
            case EClaimedQuadSize::QuarterPage:
                EXPECT_EQ(quad.getSize().x, PageWidth / 2);
                EXPECT_EQ(quad.getSize().y, PageHeight / 2);
                break;
            case EClaimedQuadSize::SinglePixel:
                EXPECT_EQ(quad.getSize().x, 1u);
                EXPECT_EQ(quad.getSize().y, 1u);
                break;
            default:
                assert(false);
            }
        }

        void expectRamsesObjects(bool expect)
        {
            SceneObjectIterator bufferIter(m_scene, ERamsesObjectType::Texture2DBuffer);
            SceneObjectIterator samplerIter(m_scene, ERamsesObjectType::TextureSampler);
            EXPECT_EQ(!bufferIter.getNext(), !expect);
            EXPECT_EQ(!samplerIter.getNext(), !expect);
        }
    };

    AGlyphTexturePage::AGlyphTexturePage()
        : m_client(*m_framework.createClient("text test"))
        , m_scene(*m_client.createScene(sceneId_t(1u)))
        , m_glyphPage(nullptr)
    {
        expectRamsesObjects(false);
        m_glyphPage = new GlyphTexturePage(m_scene, QuadSize(PageWidth, PageHeight));
    }

    TEST_F(AGlyphTexturePage, CreatesTexture2DBufferAndSamplerWhenConstructed)
    {
        expectRamsesObjects(true);
    }

    TEST_F(AGlyphTexturePage, DestroysTexture2DBufferAndSamplerWhenDestructed)
    {
        delete m_glyphPage;
        m_glyphPage = nullptr;
        expectRamsesObjects(false);
    }

    TEST_F(AGlyphTexturePage, CreatesInternalDataCacheWithCorrectSizeAfterConstruction)
    {
        EXPECT_EQ(1u, m_glyphPage->getTextureBuffer().getMipLevelCount());
        uint32_t width;
        uint32_t height;
        m_glyphPage->getTextureBuffer().getMipLevelSize(0, width, height);
        EXPECT_EQ(PageWidth, width);
        EXPECT_EQ(PageHeight, height);
    }

    TEST_F(AGlyphTexturePage, CopiesSourceDataToTargetQuadAndAddsOnePixelPadding)
    {
        uint32_t targetX = 2;
        uint32_t targetY = 3;
        uint32_t targetW = 4;
        uint32_t targetH = 5;
        const Quad subPixelQuad(QuadOffset(targetX, targetY), QuadSize(targetW, targetH));
        GlyphTexturePage::GlyphPageData texelData((targetW - 2) * (targetH - 2));
        uint8_t fakeTexel = 1;
        for(auto& texel : texelData)
        {
            texel = fakeTexel++;
        }

        GlyphTexturePage::GlyphPageData tempCache;
        m_glyphPage->updateDataWithPadding(subPixelQuad, &texelData[0], tempCache);

        uint8_t databuffer[PageWidth * PageHeight * 4];
        m_glyphPage->getTextureBuffer().getMipLevelData(0, databuffer, PageWidth * PageHeight * 4);
        uint8_t currentlyExpectedFakeTexelColor = 1;
        for (uint32_t row = 0; row < PageHeight; row++)
        {
            for (uint32_t col = 0; col < PageWidth; col++)
            {
                if (row >= targetY &&
                    row < targetY + targetH &&
                    col >= targetX &&
                    col < targetX + targetW)
                {
                    const uint8_t cachedTexel = databuffer[row * PageWidth + col];

                    if (row == targetY ||
                        row == targetY + targetH - 1 ||
                        col == targetX ||
                        col == targetX + targetW - 1)
                    {
                        // Border of glyph => transparent
                        EXPECT_EQ(0x00, cachedTexel);
                    }
                    else
                    {
                        // Texel on the inside, should have the correct value
                        EXPECT_EQ(currentlyExpectedFakeTexelColor, cachedTexel);
                        ++currentlyExpectedFakeTexelColor;
                    }
                }
            }
        }
    }

    TEST_F(AGlyphTexturePage, NewGlyphPageHasOneFreeAreaWithWidthTimesHeightArea)
    {
        uint32_t fullArea = PageWidth * PageHeight;
        expectFreeArea(fullArea);
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 1u);
    }

    TEST_F(AGlyphTexturePage, claimingAZeroSpaceIsANoop)
    {
        m_glyphPage->claimSpace(0, QuadSize(0, 0));

        uint32_t fullArea = PageWidth * PageHeight;
        expectFreeArea(fullArea);
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 1u);
    }

    TEST_F(AGlyphTexturePage, claimingSpaceOnceIsCorrectAndRevertable)
    {
        uint32_t fullArea = PageWidth * PageHeight;
        auto claimedSpace = QuadSize(3, 3);
        auto offset = m_glyphPage->claimSpace(0, claimedSpace);

        expectFreeArea(fullArea - claimedSpace.getArea());
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);

        m_glyphPage->releaseSpace(Quad(offset, claimedSpace));
        expectFreeArea(fullArea);
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 1u);
    }

    TEST_F(AGlyphTexturePage, claimingSpaceTwiceWithSameWidthOrHeightLeadsToMergeOnRevert)
    {
        uint32_t fullArea = PageWidth * PageHeight;
        QuadSize claimedSpace(4, 3);
        QuadSize claimedSpaceW(4, 4);
        QuadSize claimedSpaceH(3, 3);

        auto offset = m_glyphPage->claimSpace(0, claimedSpace);
        expectFreeArea(fullArea - claimedSpace.getArea());
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);

        {
            auto offset2 = m_glyphPage->claimSpace(0, claimedSpaceH);
            expectFreeArea(fullArea - claimedSpace.getArea() - claimedSpaceH.getArea());
            EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);

            m_glyphPage->releaseSpace(Quad(offset2, claimedSpaceH));
            expectFreeArea(fullArea - claimedSpace.getArea());
            EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);
        }

        {
            auto offset2 = m_glyphPage->claimSpace(0, claimedSpaceW);
            expectFreeArea(fullArea - claimedSpace.getArea() - claimedSpaceW.getArea());
            EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);

            m_glyphPage->releaseSpace(Quad(offset2, claimedSpaceW));
            expectFreeArea(fullArea - claimedSpace.getArea());
            // no expect here since merging the new free quad is not possible with our algorithm
        }

        m_glyphPage->releaseSpace(Quad(offset, claimedSpace));
        expectFreeArea(fullArea);
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 1u);
    }

    TEST_F(AGlyphTexturePage, claimingSpaceAndOnlyPartiallyReleaseIt_ConnectedToFreeSpace)
    {
        uint32_t fullArea = PageWidth * PageHeight;
        auto claimedSpace = QuadSize(4, 4);
        auto offset = m_glyphPage->claimSpace(0, claimedSpace);

        expectFreeArea(fullArea - claimedSpace.getArea());
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);

        QuadOffset newOffset(offset.x + claimedSpace.x/2, offset.y);
        QuadSize newSize(claimedSpace.x / 2, claimedSpace.y);
        m_glyphPage->releaseSpace(Quad(newOffset, newSize));
        expectFreeArea(fullArea - claimedSpace.getArea() + newSize.getArea());
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);
    }

    TEST_F(AGlyphTexturePage, claimingSpaceAndOnlyPartiallyReleaseIt_DisconnectedToFreeSpace)
    {
        uint32_t fullArea = PageWidth * PageHeight;
        auto claimedSpace = QuadSize(4, 4);
        auto offset = m_glyphPage->claimSpace(0, claimedSpace);

        expectFreeArea(fullArea - claimedSpace.getArea());
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 2u);

        QuadSize newSize(claimedSpace.x / 2, claimedSpace.y / 2);
        m_glyphPage->releaseSpace(Quad(offset, newSize));
        expectFreeArea(fullArea - claimedSpace.getArea() + newSize.getArea());
        EXPECT_EQ(m_glyphPage->getFreeSpace().size(), 3u);
    }

    // old array tests moved here -------------------------------------------------------------------------
    TEST_F(AGlyphTexturePage, MapsSingleGlyphWhichExactlyFitsPageSize)
    {
        const Quad quad = claimTestQuad(EClaimedQuadSize::FullPage);
        expectQuadPlacing(quad, EClaimedQuadPosition::TopLeft, EClaimedQuadSize::FullPage);
    }

    TEST_F(AGlyphTexturePage, CanNotCreateAnotherGlyphWhenPageIsFull)
    {
        claimTestQuad(EClaimedQuadSize::FullPage);
        EXPECT_FALSE(canClaimQuad(EClaimedQuadSize::SinglePixel));
    }

    TEST_F(AGlyphTexturePage, MapsSecondGlyphWhenSpaceStillAvailableInPage)
    {
        const Quad leftHalf = claimTestQuad(EClaimedQuadSize::HalfPage);
        expectQuadPlacing(leftHalf, EClaimedQuadPosition::TopLeft, EClaimedQuadSize::HalfPage);

        const Quad rightHalf = claimTestQuad(EClaimedQuadSize::HalfPage);
        expectQuadPlacing(rightHalf, EClaimedQuadPosition::TopRight, EClaimedQuadSize::HalfPage);
    }

    TEST_F(AGlyphTexturePage, confidence_MapsFourGlyphs_WhichHaveQuarterPageSize_InOnePage)
    {
        const Quad glyphMapping1 = claimTestQuad(EClaimedQuadSize::QuarterPage);
        const Quad glyphMapping2 = claimTestQuad(EClaimedQuadSize::QuarterPage);
        const Quad glyphMapping3 = claimTestQuad(EClaimedQuadSize::QuarterPage);
        const Quad glyphMapping4 = claimTestQuad(EClaimedQuadSize::QuarterPage);

        expectQuadPlacing(glyphMapping1, EClaimedQuadPosition::TopLeft, EClaimedQuadSize::QuarterPage);
        expectQuadPlacing(glyphMapping2, EClaimedQuadPosition::TopRight, EClaimedQuadSize::QuarterPage);
        expectQuadPlacing(glyphMapping3, EClaimedQuadPosition::BottomLeft, EClaimedQuadSize::QuarterPage);
        expectQuadPlacing(glyphMapping4, EClaimedQuadPosition::BottomRight, EClaimedQuadSize::QuarterPage);
    }

    TEST_F(AGlyphTexturePage, confidence_MapsAMosaicOfGlyphsWithHeterogeneousSizes_SoThatNewGlyphDoesNotFitOnPage)
    {
        const Quad halfPage_Page1 = claimTestQuad(EClaimedQuadSize::HalfPage);
        const Quad quarterPage_Page1 = claimTestQuad(EClaimedQuadSize::QuarterPage);
        claimTestQuad(EClaimedQuadSize::SinglePixel);

        expectQuadPlacing(quarterPage_Page1, EClaimedQuadPosition::TopRight, EClaimedQuadSize::QuarterPage);
        expectQuadPlacing(halfPage_Page1, EClaimedQuadPosition::TopLeft, EClaimedQuadSize::HalfPage);

        EXPECT_FALSE(canClaimQuad(EClaimedQuadSize::QuarterPage));
    }

    TEST_F(AGlyphTexturePage, ReleasesSpaceInPageAndMapsANewGlyph)
    {
        const Quad glyph = claimTestQuad(EClaimedQuadSize::FullPage);
        m_glyphPage->releaseSpace(glyph);

        const Quad newGlyph = claimTestQuad(EClaimedQuadSize::FullPage);
        expectQuadPlacing(newGlyph, EClaimedQuadPosition::TopLeft, EClaimedQuadSize::FullPage);
    }

    TEST_F(AGlyphTexturePage, ReleaseFourGlyphsAndMapNewGlyphWhichIsFourTimesBigger)
    {
        const Quad glyphMapping1 = claimTestQuad(EClaimedQuadSize::QuarterPage);
        const Quad glyphMapping2 = claimTestQuad(EClaimedQuadSize::QuarterPage);
        const Quad glyphMapping3 = claimTestQuad(EClaimedQuadSize::QuarterPage);
        const Quad glyphMapping4 = claimTestQuad(EClaimedQuadSize::QuarterPage);

        m_glyphPage->releaseSpace(glyphMapping1);
        m_glyphPage->releaseSpace(glyphMapping2);
        m_glyphPage->releaseSpace(glyphMapping3);
        m_glyphPage->releaseSpace(glyphMapping4);

        const Quad glyphMapping = claimTestQuad(EClaimedQuadSize::FullPage);
        expectQuadPlacing(glyphMapping, EClaimedQuadPosition::TopLeft, EClaimedQuadSize::FullPage);
    }

    TEST_F(AGlyphTexturePage, FindsFreeSpaceForSizeEqualToPageSizeIfEmpty)
    {
        EXPECT_NE(m_glyphPage->findFreeSpace(QuadSize(PageWidth, PageHeight)), std::numeric_limits<GlyphTexturePage::QuadIndex>::max());
    }

    TEST_F(AGlyphTexturePage, FindsNoFreeSpaceForSizeTooBigForLeftoverSpace)
    {
        m_glyphPage->claimSpace(0, QuadSize(5, 5));
        EXPECT_EQ(m_glyphPage->findFreeSpace(QuadSize(PageWidth, PageHeight)), std::numeric_limits<GlyphTexturePage::QuadIndex>::max());
    }

    TEST_F(AGlyphTexturePage, FindsNoFreeSpaceForSizeTooBigForAllFreespaceQuads)
    {
        m_glyphPage->claimSpace(0, QuadSize(5, 5));
        // the next quad size would theoretically fit, but not with the segmentation after last claim
        EXPECT_EQ(m_glyphPage->findFreeSpace(QuadSize(PageWidth - 5, PageHeight)), std::numeric_limits<GlyphTexturePage::QuadIndex>::max());
    }

    TEST_F(AGlyphTexturePage, ChoosesExactFitFreespaceQuadWithFindFreespace)
    {
        m_glyphPage->claimSpace(0, QuadSize(5, 5));
        m_glyphPage->claimSpace(0, QuadSize(3, 3));
        std::vector<QuadSize> vec = { QuadSize(3, 2), QuadSize(PageWidth - 8, 5), QuadSize(PageWidth, PageHeight - 5) };

        for (auto const& entry : vec)
        {
            auto it = m_glyphPage->getFreeSpace().begin();
            GlyphTexturePage::QuadIndex index = 0;
            for (; it != m_glyphPage->getFreeSpace().end(); ++it)
            {
                if (it->getSize() == entry)
                    break;

                index++;
            }

            EXPECT_NE(it, m_glyphPage->getFreeSpace().end());
            EXPECT_EQ(m_glyphPage->findFreeSpace(entry), index);
        }
    }

    TEST_F(AGlyphTexturePage, ChoosesBestFitFreespaceQuadWithFindFreespace)
    {
        m_glyphPage->claimSpace(0, QuadSize(5, 5));
        m_glyphPage->claimSpace(0, QuadSize(3, 3));
        std::vector<QuadSize> vec = { QuadSize(3, 2), QuadSize(PageWidth - 8, 5), QuadSize(PageWidth, PageHeight - 5) }; //free space
        std::vector<QuadSize> vec2 = { QuadSize(1, 1), QuadSize(3, 3), QuadSize(PageWidth - 7, PageHeight - 5) }; // quads to find free space for

        assert(vec.size() == vec2.size());
        for (size_t i = 0; i < vec.size(); ++i)
        {
            auto it = m_glyphPage->getFreeSpace().begin();
            GlyphTexturePage::QuadIndex index = 0;
            for (; it != m_glyphPage->getFreeSpace().end(); ++it)
            {
                if (it->getSize() == vec[i])
                    break;

                index++;
            }

            EXPECT_NE(it, m_glyphPage->getFreeSpace().end());
            EXPECT_EQ(m_glyphPage->findFreeSpace(vec2[i]), index);
        }
    }
}
