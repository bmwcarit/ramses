//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text/TextCacheImpl.h"
#include "ramses-text-api/TextLine.h"
#include "ramses-text-api/IFontAccessor.h"
#include "ramses-text-api/IFontInstance.h"
#include "ramses-text/Logger.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"

#include <iostream>
#include <limits>

namespace ramses
{
    TextCacheImpl::TextCacheImpl(Scene& scene, IFontAccessor& fontAccessor, uint32_t atlasTextureWidth, uint32_t atlasTextureHeight)
        : m_scene(scene)
        , m_fontAccessor(fontAccessor)
        , m_textureAtlas(scene, { atlasTextureWidth, atlasTextureHeight })
    {
    }

    GlyphMetricsVector TextCacheImpl::getPositionedGlyphs(const std::u32string& str, const FontInstanceOffsets& fontOffsets)
    {
        GlyphMetricsVector positionedGlyphs;
        positionedGlyphs.reserve(str.size());

        for (auto fontIt = fontOffsets.cbegin(); fontIt != fontOffsets.cend(); ++fontIt)
        {
            const auto substrBeginIt = std::min(str.cbegin() + fontIt->beginOffset, str.cend());
            const auto nextFontIt = std::next(fontIt);
            auto substrEndIt = (nextFontIt != fontOffsets.cend() ? str.cbegin() + nextFontIt->beginOffset : str.cend());
            substrEndIt = std::min(substrEndIt, str.cend());

            IFontInstance* fontInstance = m_fontAccessor.getFontInstance(fontIt->fontInstance);
            if (fontInstance != nullptr)
                fontInstance->loadAndAppendGlyphMetrics(substrBeginIt, substrEndIt, positionedGlyphs);
            else
                LOG_TEXT_ERROR("TextCache::getPositionedGlyphs: Could not find font instance " << fontIt->fontInstance.getValue());
        }

        return positionedGlyphs;
    }

    GlyphMetricsVector TextCacheImpl::getPositionedGlyphs(const std::u32string& str, FontInstanceId font)
    {
        return getPositionedGlyphs(str, { { font, 0u } });
    }

    TextLineId TextCacheImpl::createTextLine(const GlyphMetricsVector& glyphs, const Effect& effect)
    {
        if (glyphs.empty())
        {
            LOG_TEXT_ERROR("TextCache::createTextLine failed - cannot create text geometry for empty string");
            return InvalidTextLineId;
        }

        UniformInput texInput;
        AttributeInput posInput;
        AttributeInput texCoordInput;
        effect.findUniformInput(EEffectUniformSemantic_TextTexture, texInput);
        effect.findAttributeInput(EEffectAttributeSemantic_TextPositions, posInput);
        effect.findAttributeInput(EEffectAttributeSemantic_TextTextureCoordinates, texCoordInput);
        if (!texInput.isValid() || !posInput.isValid() || !texCoordInput.isValid())
        {
            LOG_TEXT_ERROR("TextCache::createTextLine failed - text appearance effect must provide inputs for positions and coordinates attributes and a texture uniform");
            return InvalidTextLineId;
        }

        for (const auto& glyph : glyphs)
        {
            if (!m_textureAtlas.isGlyphRegistered(glyph.key))
            {
                IFontInstance* fontInstance = m_fontAccessor.getFontInstance(glyph.key.fontInstanceId);
                if (fontInstance == nullptr)
                {
                    LOG_TEXT_ERROR("TextCache::createTextLine: Could not find font instance " << glyph.key.fontInstanceId.getValue());
                    return InvalidTextLineId;
                }
                QuadSize glyphSize;
                GlyphData data = fontInstance->loadGlyphBitmapData(glyph.key.identifier, glyphSize.x, glyphSize.y);
                m_textureAtlas.registerGlyph(glyph.key, glyphSize, std::move(data));
            }
        }

        const GlyphGeometry geometry = m_textureAtlas.mapGlyphsAndCreateGeometry(glyphs);
        if (geometry.atlasPage == std::numeric_limits<decltype(geometry.atlasPage)>::max() || geometry.indices.empty())
        {
            LOG_TEXT_ERROR("TextCache::createTextLine failed - glyphs could not be mapped in atlas");
            return InvalidTextLineId;
        }

        GeometryBinding* geometryBinding = m_scene.createGeometryBinding(effect);
        Appearance* appearance = m_scene.createAppearance(effect);
        if (geometryBinding == nullptr || appearance == nullptr)
        {
            LOG_TEXT_ERROR("TextCache::createTextLine failed - failed to create geometry binding and/or appearance, check Ramses logs for more details");
            m_textureAtlas.unmapGlyphsFromPage(glyphs, geometry.atlasPage);
            return InvalidTextLineId;
        }

        auto textLineId = m_textIdCounter;
        m_textIdCounter.getReference()++;

        TextLine& textLine = m_textLines[textLineId];
        textLine.atlasPage = geometry.atlasPage;
        textLine.glyphs = glyphs;
        textLine.meshNode = m_scene.createMeshNode();

        const uint32_t numIndices = static_cast<uint32_t>(geometry.indices.size());
        const uint32_t indicesByteSize = numIndices * sizeof(geometry.indices[0]);
        textLine.indices = m_scene.createIndexDataBuffer(indicesByteSize, ramses::EDataType_UInt16, "");
        textLine.indices->setData(reinterpret_cast<const char*>(geometry.indices.data()), indicesByteSize);

        const uint32_t numVertexElements = static_cast<uint32_t>(geometry.positions.size());
        const uint32_t positionsDataSize = numVertexElements * sizeof(geometry.positions[0]);
        textLine.positions = m_scene.createVertexDataBuffer(positionsDataSize, ramses::EDataType_Vector2F, "");
        textLine.positions->setData(reinterpret_cast<const char*>(geometry.positions.data()), positionsDataSize);

        const uint32_t texCoordsDataSize = numVertexElements * sizeof(geometry.texcoords[0]);
        textLine.textureCoordinates = m_scene.createVertexDataBuffer(texCoordsDataSize, ramses::EDataType_Vector2F, "");
        textLine.textureCoordinates->setData(reinterpret_cast<const char*>(geometry.texcoords.data()), texCoordsDataSize);

        textLine.meshNode->setStartIndex(0);
        textLine.meshNode->setIndexCount(numIndices);

        geometryBinding->setIndices(*textLine.indices);
        geometryBinding->setInputBuffer(posInput, *textLine.positions);
        geometryBinding->setInputBuffer(texCoordInput, *textLine.textureCoordinates);

        appearance->setInputTexture(texInput, m_textureAtlas.getTextureSampler(geometry.atlasPage));

        textLine.meshNode->setAppearance(*appearance);
        textLine.meshNode->setGeometryBinding(*geometryBinding);

        return textLineId;
    }

    TextLine const* TextCacheImpl::getTextLine(TextLineId textId) const
    {
        const auto it = m_textLines.find(textId);
        return it != m_textLines.cend() ? &it->second : nullptr;
    }

    TextLine* TextCacheImpl::getTextLine(TextLineId textId)
    {
        const auto it = m_textLines.find(textId);
        return it != m_textLines.cend() ? &it->second : nullptr;
    }

    bool TextCacheImpl::deleteTextLine(TextLineId textId)
    {
        if (m_textLines.count(textId) != 1)
        {
            LOG_TEXT_ERROR("TextCache::deleteTextLine: Cannot delete text line " << textId.getValue() << ", no such entry");
            return false;
        }

        TextLine& textLine = m_textLines[textId];
        auto geometry = textLine.meshNode->getGeometryBinding();
        auto appearance = textLine.meshNode->getAppearance();
        m_scene.destroy(*textLine.meshNode);
        m_scene.destroy(*geometry);
        m_scene.destroy(*appearance);
        m_scene.destroy(*textLine.positions);
        m_scene.destroy(*textLine.textureCoordinates);
        m_scene.destroy(*textLine.indices);

        m_textureAtlas.unmapGlyphsFromPage(textLine.glyphs, textLine.atlasPage);

        m_textLines.erase(textId);
        return true;
    }
}
