//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/text/FontRegistryImpl.h"
#include "impl/text/Freetype2FontInstance.h"
#include "impl/text/HarfbuzzFontInstance.h"
#include "impl/text/TextTypesImpl.h"
#include "internal/Core/Utils/LogMacros.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "internal/Core/Utils/File.h"
#include <fstream>
#include <cassert>

namespace ramses::internal
{
    FT_Library SharedFTLibrary::Ft2Library = nullptr;
    size_t SharedFTLibrary::NumRefs = 0u;

    SharedFTLibrary::SharedFTLibrary()
    {
        if (Ft2Library == nullptr)
        {
            const int32_t error = FT_Init_FreeType(&Ft2Library);
            if (error != 0)
            {
                Ft2Library = nullptr;
                LOG_ERROR(CONTEXT_TEXT, "SharedFTLibrary: Failed to initialize FreeType with error {}", error);
            }
        }
        NumRefs++;
    }

    SharedFTLibrary::~SharedFTLibrary()
    {
        assert(NumRefs > 0u);
        NumRefs--;
        if (NumRefs == 0u && Ft2Library != nullptr)
        {
            FT_Done_FreeType(Ft2Library);
            Ft2Library = nullptr;
        }
    }

    FT_Library SharedFTLibrary::Get()
    {
        return Ft2Library;
    }

    IFontInstance* FontRegistryImpl::getFontInstance(FontInstanceId fontInstanceId) const
    {
        auto it = m_fontInstances.find(fontInstanceId);
        return it != m_fontInstances.cend() ? it->second.get() : nullptr;
    }

    FontId FontRegistryImpl::createFreetype2Font(std::string_view fontPath)
    {
        // basic checks
        if (fontPath.empty() || !ramses::internal::File(fontPath).exists())
        {
            LOG_ERROR(CONTEXT_TEXT, "FontRegistry::createFreetype2Font: Font file not found {}", fontPath);
            return {};
        }

        LOG_INFO(CONTEXT_CLIENT, "FontRegistry::createFreetype2Font: path {}", fontPath);
        return createFreetype2FontCommon(std::make_unique<ramses::internal::FreetypeFontFaceFilePath>(fontPath, SharedFTLibrary::Get()));
    }

    FontId FontRegistryImpl::createFreetype2FontFromFileDescriptor(int fd, size_t offset, size_t length)
    {
        // basic checks
        if (fd <= 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "FontRegistry::createFreetype2FontFromFileDescriptor: filedescriptor must be valid {}", fd);
            return {};
        }
        if (length == 0)
        {
            LOG_ERROR(CONTEXT_CLIENT, "FontRegistry::createFreetype2FontFromFileDescriptor: length may not be 0");
            return {};
        }

        LOG_INFO(CONTEXT_CLIENT, "FontRegistry::createFreetype2FontFromFileDescriptor: fd {}, offset {}, length {}", fd, offset, length);
        return createFreetype2FontCommon(std::make_unique<ramses::internal::FreetypeFontFaceFileDescriptor>(fd, offset, length, SharedFTLibrary::Get()));
    }

    FontId FontRegistryImpl::createFreetype2FontCommon(std::unique_ptr<ramses::internal::FreetypeFontFace> face)
    {
        if (!face->init())
            return {};

        const FontId fontId = m_lastFontId;
        m_lastFontId.getReference()++;

        assert(m_fonts.count(fontId) == 0u);
        m_fonts.insert(std::make_pair(fontId, std::move(face)));

        return fontId;
    }

    bool FontRegistryImpl::deleteFont(FontId fontId)
    {
        if (m_fonts.erase(fontId) == 0u)
        {
            LOG_ERROR(CONTEXT_TEXT, "FontRegistryImpl::deleteFont: Cannot delete font {}, no such entry", fontId);
            return false;
        }
        return true;
    }

    FontInstanceId FontRegistryImpl::createFreetype2FontInstance(FontId fontId, uint32_t size, bool forceAutohinting)
    {
        const auto fontIt = m_fonts.find(fontId);
        if (fontIt == m_fonts.cend())
        {
            LOG_ERROR(CONTEXT_TEXT, "FontRegistry: Failed to create font instance, fontId {} does not exist", fontId);
            return {};
        }

        const FontInstanceId fontInstanceId = reserveFontInstanceId();
        registerFontInstance(fontInstanceId, std::unique_ptr<IFontInstance>{ new Freetype2FontInstance(fontInstanceId, fontIt->second->getFace(), size, forceAutohinting) });

        return fontInstanceId;
    }

    FontInstanceId FontRegistryImpl::createFreetype2FontInstanceWithHarfBuzz(FontId fontId, uint32_t size, bool forceAutohinting)
    {
        const auto fontIt = m_fonts.find(fontId);
        if (fontIt == m_fonts.cend())
        {
            LOG_ERROR(CONTEXT_TEXT, "FontRegistry: Failed to create font instance, fontId {} does not exist", fontId);
            return {};
        }

        const FontInstanceId fontInstanceId = reserveFontInstanceId();
        registerFontInstance(fontInstanceId, std::unique_ptr<IFontInstance>{ new HarfbuzzFontInstance(fontInstanceId, fontIt->second->getFace(), size, forceAutohinting) });

        return fontInstanceId;
    }

    bool FontRegistryImpl::deleteFontInstance(FontInstanceId fontInstance)
    {
        if (m_fontInstances.erase(fontInstance) == 0u)
        {
            LOG_ERROR(CONTEXT_TEXT, "FontRegistryImpl::deleteFontInstance: Cannot delete font instance {}, no such entry", fontInstance);
            return false;
        }
        return true;
    }

    FontInstanceId FontRegistryImpl::reserveFontInstanceId()
    {
        const FontInstanceId fontInstanceId = m_lastFontInstanceId;
        m_lastFontInstanceId.getReference()++;

        assert(m_fontInstances.count(fontInstanceId) == 0u);
        return fontInstanceId;
    }

    void FontRegistryImpl::registerFontInstance(FontInstanceId fontInstanceId, std::unique_ptr<IFontInstance> fontInstance)
    {
        assert(m_fontInstances.count(fontInstanceId) == 0u);
        m_fontInstances.insert(std::make_pair(fontInstanceId, std::move(fontInstance)));
    }
}
