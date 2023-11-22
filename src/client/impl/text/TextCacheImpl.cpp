//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/text/TextCacheImpl.h"
#include "ramses/client/text/TextLine.h"
#include "ramses/client/text/TextCache.h"
#include "ramses/client/text/IFontAccessor.h"
#include "ramses/client/text/IFontInstance.h"

#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"

#include "internal/Core/Utils/LogMacros.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/text/TextTypesImpl.h"
#include <limits>

namespace ramses::internal
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
            const auto substrBeginIt = std::min(str.cbegin() + static_cast<ptrdiff_t>(fontIt->beginOffset), str.cend());
            const auto nextFontIt = std::next(fontIt);
            auto substrEndIt = (nextFontIt != fontOffsets.cend() ? str.cbegin() + static_cast<ptrdiff_t>(nextFontIt->beginOffset) : str.cend());
            substrEndIt = std::min(substrEndIt, str.cend());

            IFontInstance* fontInstance = m_fontAccessor.getFontInstance(fontIt->fontInstance);
            if (fontInstance != nullptr)
            {
                fontInstance->loadAndAppendGlyphMetrics(substrBeginIt, substrEndIt, positionedGlyphs);
            }
            else
            {
                LOG_ERROR(CONTEXT_TEXT, "TextCache::getPositionedGlyphs: Could not find font instance {}", fontIt->fontInstance);
            }
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
            LOG_ERROR(CONTEXT_TEXT, "TextCache::createTextLine failed - cannot create text geometry for empty string");
            return {};
        }

        const auto texInput      = effect.findUniformInput(EEffectUniformSemantic::TextTexture);
        const auto posInput      = effect.findAttributeInput(EEffectAttributeSemantic::TextPositions);
        const auto texCoordInput = effect.findAttributeInput(EEffectAttributeSemantic::TextTextureCoordinates);

        if (!texInput.has_value() || !posInput.has_value() || !texCoordInput.has_value())
        {
            LOG_ERROR(CONTEXT_TEXT, "TextCache::createTextLine failed - text appearance effect must provide inputs for positions and coordinates attributes and a texture uniform");
            return {};
        }

        for (const auto& glyph : glyphs)
        {
            if (!m_textureAtlas.isGlyphRegistered(glyph.key))
            {
                IFontInstance* fontInstance = m_fontAccessor.getFontInstance(glyph.key.fontInstanceId);
                if (fontInstance == nullptr)
                {
                    LOG_ERROR(CONTEXT_TEXT, "TextCache::createTextLine: Could not find font instance {}", glyph.key.fontInstanceId);
                    return {};
                }
                QuadSize glyphSize;
                GlyphData data = fontInstance->loadGlyphBitmapData(glyph.key.identifier, glyphSize.x, glyphSize.y);
                m_textureAtlas.registerGlyph(glyph.key, glyphSize, std::move(data));
            }
        }

        const bool allGlyphsEmpty = !TextCache::ContainsRenderableGlyphs(glyphs);

        if (allGlyphsEmpty)
        {
            LOG_ERROR(CONTEXT_TEXT, "TextCache::createTextLine failed - string has only empty glyphs (whitespace or control signs). Can't create a mesh for them!");
            return {};
        }

        const GlyphGeometry geometry = m_textureAtlas.mapGlyphsAndCreateGeometry(glyphs);
        if (geometry.atlasPage == std::numeric_limits<decltype(geometry.atlasPage)>::max())
        {
            LOG_ERROR(CONTEXT_TEXT, "TextCache::createTextLine failed - glyphs could not be mapped in atlas");
            return {};
        }

        Geometry* geometryBinding = m_scene.createGeometry(effect);
        Appearance* appearance = m_scene.createAppearance(effect);
        if (geometryBinding == nullptr || appearance == nullptr)
        {
            LOG_ERROR(CONTEXT_TEXT, "TextCache::createTextLine failed - failed to create geometry binding and/or appearance, check Ramses logs for more details");
            m_textureAtlas.unmapGlyphsFromPage(glyphs, geometry.atlasPage);
            return {};
        }

        auto textLineId = m_textIdCounter;
        m_textIdCounter.getReference()++;

        TextLine& textLine = m_textLines[textLineId];
        textLine.atlasPage = geometry.atlasPage;
        textLine.glyphs = glyphs;
        textLine.meshNode = m_scene.createMeshNode();

        const auto numIndices = static_cast<uint32_t>(geometry.indices.size());
        textLine.indices = m_scene.createArrayBuffer(ramses::EDataType::UInt16, numIndices, "");
        textLine.indices->updateData(0u, numIndices, geometry.indices.data());

        const auto numVertexElements = static_cast<uint32_t>(geometry.positions.size());
        textLine.positions = m_scene.createArrayBuffer(ramses::EDataType::Vector2F, numVertexElements, "");
        textLine.positions->updateData(0u, numVertexElements, geometry.positions.data());

        textLine.textureCoordinates = m_scene.createArrayBuffer(ramses::EDataType::Vector2F, numVertexElements, "");
        textLine.textureCoordinates->updateData(0u, numVertexElements, geometry.texcoords.data());

        textLine.meshNode->setStartIndex(0);
        textLine.meshNode->setIndexCount(numIndices);

        geometryBinding->setIndices(*textLine.indices);
        geometryBinding->setInputBuffer(*posInput, *textLine.positions);
        geometryBinding->setInputBuffer(*texCoordInput, *textLine.textureCoordinates);

        appearance->setInputTexture(*texInput, m_textureAtlas.getTextureSampler(geometry.atlasPage));

        textLine.meshNode->setAppearance(*appearance);
        textLine.meshNode->setGeometry(*geometryBinding);

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
            LOG_ERROR(CONTEXT_TEXT, "TextCache::deleteTextLine: Cannot delete text line {}, no such entry", textId);
            return false;
        }

        TextLine& textLine = m_textLines[textId];
        auto geometry = textLine.meshNode->getGeometry();
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
