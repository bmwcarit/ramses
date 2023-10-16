//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/text/Quad.h"

namespace ramses
{
    class Scene;
    class Texture2DBuffer;
    class TextureSampler;
}

namespace ramses::internal
{
    class GlyphTexturePage
    {
    public:
        GlyphTexturePage(Scene& scene, const QuadSize& size);
        ~GlyphTexturePage();

        GlyphTexturePage(const GlyphTexturePage&) = delete;
        GlyphTexturePage(const GlyphTexturePage&&) = delete;
        GlyphTexturePage& operator=(const GlyphTexturePage&) = delete;
        GlyphTexturePage& operator=(const GlyphTexturePage&&) = delete;

        using QuadIndex = size_t;
        using GlyphPageData = std::vector<uint8_t>;

        // Free space management
        [[nodiscard]] const Quads& getFreeSpace() const;
        QuadOffset claimSpace(QuadIndex freeQuadIndex, const QuadSize& subportionSize);
        void releaseSpace(Quad box);
        [[nodiscard]] QuadIndex findFreeSpace(QuadSize const& size) const;

        // Texture data management
        void updateDataWithPadding(const Quad& targetQuad, const uint8_t* sourceData, GlyphPageData& cacheForDataUpdate);
        [[nodiscard]] const Texture2DBuffer& getTextureBuffer() const;
        [[nodiscard]] const TextureSampler& getSampler() const;

    private:
        bool mergeFreeQuad(Quad& freeQuadInAndOut);
        static void CopyPaddingToCache(const Quad& updateQuad, GlyphPageData& cacheForDataUpdate);
        static void CopyUpdateDataWithoutPaddingToCache(const Quad& updateQuad, const uint8_t* data, GlyphPageData& cacheForDataUpdate);
        void updateTextureResource(const Quad& updateQuade, const GlyphPageData& pageData);

        const QuadSize m_size;
        Quads m_freeQuads;
        Scene& m_ownerScene;
        Texture2DBuffer& m_textureBuffer;
        TextureSampler&  m_textureSampler;
    };
}
