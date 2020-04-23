//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FREETYPEFONTFACE_H
#define RAMSES_FREETYPEFONTFACE_H

#include "ramses-text/Freetype2Wrapper.h"
#include <string>

namespace ramses
{
    class FreetypeFontFace
    {
    public:
        FreetypeFontFace(const char* fontPth, FT_Library freetypeLib);
        ~FreetypeFontFace();

        bool init();
        FT_Face getFace();

        FreetypeFontFace(const FreetypeFontFace&) = delete;
        FreetypeFontFace& operator=(const FreetypeFontFace&) = delete;
        FreetypeFontFace(FreetypeFontFace&&) = delete;
        FreetypeFontFace& operator=(FreetypeFontFace&&) = delete;

    private:
        std::string m_fontPath;
        FT_Library m_freetypeLib;
        FT_Face m_face = nullptr;
    };
}

#endif
