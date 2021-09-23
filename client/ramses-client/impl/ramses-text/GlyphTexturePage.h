//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLYPHTEXTUREPAGE_H
#define RAMSES_GLYPHTEXTUREPAGE_H

#include "ramses-text/Quad.h"

namespace ramses
{
    class Scene;
    class Texture2DBuffer;
    class TextureSampler;

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
        const Quads& getFreeSpace() const;
        QuadOffset claimSpace(QuadIndex freeQuadIndex, const QuadSize& subportionSize);
        void releaseSpace(Quad box);
        QuadIndex findFreeSpace(QuadSize const& size) const;

        // Texture data management
        void updateDataWithPadding(const Quad& targetQuad, const uint8_t* sourceData, GlyphPageData& cacheForDataUpdate);
        const Texture2DBuffer& getTextureBuffer() const;
        const TextureSampler& getSampler() const;

    private:
        bool mergeFreeQuad(Quad& freeQuadInAndOut);
        void copyPaddingToCache(const Quad& updateQuad, GlyphPageData& cacheForDataUpdate);
        void copyUpdateDataWithoutPaddingToCache(const Quad& updateQuad, const uint8_t* data, GlyphPageData& cacheForDataUpdate);
        void updateTextureResource(const Quad& updateQuade, const GlyphPageData& pageData);

        const QuadSize m_size;
        Quads m_freeQuads;
        Scene& m_ownerScene;
        Texture2DBuffer& m_textureBuffer;
        TextureSampler&  m_textureSampler;
    };
}

#endif
