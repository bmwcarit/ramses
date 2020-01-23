//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text/GlyphTexturePage.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/TextureSampler.h"
#include "ramses-client-api/Texture2DBuffer.h"
#include <assert.h>


namespace
{
    bool CanFitQuadInFreeSpace(const ramses::QuadSize& freeSpace, const ramses::QuadSize& quadToFit, uint32_t& remainingSpaceOut)
    {
        const bool canFit = freeSpace.x >= quadToFit.x && freeSpace.y >= quadToFit.y;
        if (!canFit)
        {
            return false;
        }
        else
        {
            remainingSpaceOut = freeSpace.getArea() - quadToFit.getArea();
            return true;
        }
    }
}

namespace ramses
{
    GlyphTexturePage::GlyphTexturePage(Scene& scene, const QuadSize& size)
        : m_size(size)
        , m_ownerScene(scene)
        , m_textureBuffer(*scene.createTexture2DBuffer(
            1u,
            size.x,
            size.y,
            ETextureFormat_R8,
            ""))
        , m_textureSampler(*scene.createTextureSampler(
            ETextureAddressMode_Clamp,
            ETextureAddressMode_Clamp,
            ETextureSamplingMethod_Linear,
            ETextureSamplingMethod_Linear,
            m_textureBuffer))
    {
        m_freeQuads.push_back(Quad(QuadOffset(0, 0), size));
    }

    GlyphTexturePage::~GlyphTexturePage()
    {
        m_ownerScene.destroy(m_textureBuffer);
        m_ownerScene.destroy(m_textureSampler);
    }

    void GlyphTexturePage::updateDataWithPadding(const Quad& targetQuad, const uint8_t* sourceData, GlyphPageData& cacheForDataUpdate)
    {
        // Glyph size contains the padding, but the source pixel data does not...
        // TODO Violin correct glyph size to not contain the padding
        assert(targetQuad.getSize().x >= 2);
        assert(targetQuad.getSize().y >= 2);

        // Assert that the target quad does not cross the page boundaries
        assert(targetQuad.getOrigin().x + targetQuad.getSize().x <= m_size.x);
        assert(targetQuad.getOrigin().y + targetQuad.getSize().y <= m_size.y);

        const uint32_t targetQuadArea = targetQuad.getSize().getArea();
        if (cacheForDataUpdate.size() < targetQuadArea)
        {
            cacheForDataUpdate.resize(targetQuadArea);
        }

        copyPaddingToCache(targetQuad, cacheForDataUpdate);
        copyUpdateDataWithoutPaddingToCache(targetQuad, sourceData, cacheForDataUpdate);
        updateTextureResource(targetQuad, cacheForDataUpdate);
    }

    const TextureSampler& GlyphTexturePage::getSampler() const
    {
        return m_textureSampler;
    }

    const Texture2DBuffer& GlyphTexturePage::getTextureBuffer() const
    {
        return m_textureBuffer;
    }

    const Quads& GlyphTexturePage::getFreeSpace() const
    {
        return m_freeQuads;
    }

    QuadOffset GlyphTexturePage::claimSpace(QuadIndex freeQuadIndex, const QuadSize& subportionSize)
    {
        const Quad box = m_freeQuads[freeQuadIndex];
        assert(subportionSize.y <= box.getSize().y && subportionSize.x <= box.getSize().x);

        m_freeQuads.erase(m_freeQuads.begin() + freeQuadIndex);

        const uint32_t px = box.getOrigin().x;
        const uint32_t py = box.getOrigin().y;

        const uint32_t diffSizeX = box.getSize().x - subportionSize.x;
        const uint32_t diffSizeY = box.getSize().y - subportionSize.y;

        if (diffSizeY < diffSizeX)
        {
            // Cut vertically
            const Quad box1(QuadOffset(subportionSize.x + px, py), QuadSize(box.getSize().x - subportionSize.x, box.getSize().y));
            const Quad box2(QuadOffset(px, subportionSize.y + py), QuadSize(subportionSize.x, box.getSize().y - subportionSize.y));

            if(box1.getSize().getArea() != 0)
                releaseSpace(box1);

            if (box2.getSize().getArea() != 0)
                releaseSpace(box2);
        }
        else
        {
            // Cut horizontally
            const Quad box1(QuadOffset(subportionSize.x + px, py), QuadSize(box.getSize().x - subportionSize.x, subportionSize.y));
            const Quad box2(QuadOffset(px, subportionSize.y + py), QuadSize(box.getSize().x, box.getSize().y - subportionSize.y));

            if (box1.getSize().getArea() != 0)
                releaseSpace(box1);

            if (box2.getSize().getArea() != 0)
                releaseSpace(box2);
        }

        return box.getOrigin();
    }

    void GlyphTexturePage::releaseSpace(Quad box)
    {
        assert(box.getSize().getArea() != 0);
        assert(m_freeQuads.end() == std::find_if(m_freeQuads.begin(), m_freeQuads.end(), [&box](Quad const& quad)
        {
            return quad.intersects(box);
        }));

        while (mergeFreeQuad(box));
        m_freeQuads.push_back(box);
    }

    // TODO Violin fix this, make it not have an "in and out" parameter
    bool GlyphTexturePage::mergeFreeQuad(Quad& freeQuadInAndOut)
    {
        const uint32_t n = uint32_t(m_freeQuads.size());
        for (uint32_t i = 0; i < n; i++)
        {
            Quad& box2 = m_freeQuads[i];
            if (freeQuadInAndOut.merge(box2))
            {
                m_freeQuads.erase(m_freeQuads.begin() + i);
                return true;
            }
        }
        return false;
    }

    void GlyphTexturePage::copyPaddingToCache(const Quad& updateQuad, GlyphPageData& cacheForDataUpdate)
    {
        const uint32_t targetRowCount = updateQuad.getSize().y;
        const uint32_t targetColumnCount = updateQuad.getSize().x;

        for (uint32_t i = 0u; i < targetRowCount; ++i)
        {
            cacheForDataUpdate[i * targetColumnCount] = 0; //first column
            cacheForDataUpdate[i * targetColumnCount + targetColumnCount - 1] = 0; //last column
        }

        for (uint32_t i = 0u; i < targetColumnCount; ++i)
        {
            cacheForDataUpdate[i] = 0; //first row
            cacheForDataUpdate[i + (targetRowCount - 1) * targetColumnCount] = 0; // last row
        }
    }

    void GlyphTexturePage::copyUpdateDataWithoutPaddingToCache(const Quad& updateQuad, const uint8_t* data, GlyphPageData& cacheForDataUpdate)
    {
        const uint32_t targetRowCount = updateQuad.getSize().y;
        const uint32_t targetColumnCount = updateQuad.getSize().x;
        const uint32_t sourceColumnCount = targetColumnCount - 2;
        for (uint32_t targetRow = 1u; targetRow < targetRowCount - 1u; ++targetRow)
        {
            for (uint32_t targetCol = 1u; targetCol < targetColumnCount - 1u; ++targetCol)
            {
                const uint32_t targetOffset = targetRow*(targetColumnCount)+targetCol;

                // Exclude the padding
                const uint32_t sourceRow = targetRow - 1;
                const uint32_t sourceCol = targetCol - 1;

                cacheForDataUpdate[targetOffset] = data[sourceColumnCount * sourceRow + sourceCol];
            }
        }
    }

    void GlyphTexturePage::updateTextureResource(const Quad& updateQuade, const GlyphPageData& pageData)
    {
        // Cast is needed because texture buffer API is more generic and allows more data types -> hence char*, not uint8_t
        const char* castedData = reinterpret_cast<const char*>(pageData.data());
        m_textureBuffer.setData(castedData, 0, updateQuade.getOrigin().x, updateQuade.getOrigin().y, updateQuade.getSize().x, updateQuade.getSize().y);
    }

    GlyphTexturePage::QuadIndex GlyphTexturePage::findFreeSpace(QuadSize const& size) const
    {
        assert(size.getArea() > 0);
        assert(size.x <= m_size.x && size.y <= m_size.y);

        uint32_t scoreOfBestFit = std::numeric_limits<uint32_t>::max();
        GlyphTexturePage::QuadIndex bestQuadIndex = std::numeric_limits<GlyphTexturePage::QuadIndex>::max();

        for (GlyphTexturePage::QuadIndex i = 0; i < m_freeQuads.size(); i++)
        {
            uint32_t newScore;
            const bool canFit = CanFitQuadInFreeSpace(m_freeQuads[i].getSize(), size, newScore);
            if (canFit)
            {
                const bool fitsBetter = (newScore < scoreOfBestFit);
                if (fitsBetter)
                {
                    bestQuadIndex = i;
                    scoreOfBestFit = newScore;
                }

                if (scoreOfBestFit == 0)
                    break;
            }
        }

        return bestQuadIndex;
    }
}
