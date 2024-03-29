﻿//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/text/FontRegistry.h"
#include "impl/text/FontRegistryImpl.h"

namespace ramses
{
    FontRegistry::FontRegistry()
        : impl(*new internal::FontRegistryImpl)
    {
    }

    FontRegistry::~FontRegistry()
    {
        delete &impl;
    }

    IFontInstance* FontRegistry::getFontInstance(FontInstanceId fontInstanceId) const
    {
        return impl.getFontInstance(fontInstanceId);
    }

    FontId FontRegistry::createFreetype2Font(std::string_view fontPath)
    {
        return impl.createFreetype2Font(fontPath);
    }

    FontId FontRegistry::createFreetype2FontFromFileDescriptor(int fd, size_t offset, size_t length)
    {
        return impl.createFreetype2FontFromFileDescriptor(fd, offset, length);
    }

    bool FontRegistry::deleteFont(FontId fontId)
    {
        return impl.deleteFont(fontId);
    }

    FontInstanceId FontRegistry::createFreetype2FontInstance(FontId fontId, uint32_t size, bool forceAutohinting)
    {
        return impl.createFreetype2FontInstance(fontId, size, forceAutohinting);
    }

    FontInstanceId FontRegistry::createFreetype2FontInstanceWithHarfBuzz(FontId fontId, uint32_t size, bool forceAutohinting)
    {
        return impl.createFreetype2FontInstanceWithHarfBuzz(fontId, size, forceAutohinting);
    }

    bool FontRegistry::deleteFontInstance(FontInstanceId fontInstance)
    {
        return impl.deleteFontInstance(fontInstance);
    }
}
