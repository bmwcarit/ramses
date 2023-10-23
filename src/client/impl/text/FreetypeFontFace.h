//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/text/Freetype2Wrapper.h"
#include "internal/Core/Utils/BinaryOffsetFileInputStream.h"

#include <string>
#include <string_view>

namespace ramses::internal
{
    class FreetypeFontFace
    {
    public:
        virtual ~FreetypeFontFace();

        virtual bool init() = 0;
        FT_Face getFace();

        FreetypeFontFace(const FreetypeFontFace&) = delete;
        FreetypeFontFace& operator=(const FreetypeFontFace&) = delete;
        FreetypeFontFace(FreetypeFontFace&&) = delete;
        FreetypeFontFace& operator=(FreetypeFontFace&&) = delete;

    protected:
        explicit FreetypeFontFace(FT_Library freetypeLib);
        bool initFromOpenArgs(const FT_Open_Args* args);

    private:
        FT_Library m_freetypeLib;
        FT_Face m_face = nullptr;
    };

    class FreetypeFontFaceFilePath : public FreetypeFontFace
    {
    public:
        FreetypeFontFaceFilePath(std::string_view fontPath, FT_Library freetypeLib);

        bool init() override;

    private:
        std::string m_fontPath;
    };

    class FreetypeFontFaceFileDescriptor : public FreetypeFontFace
    {
    public:
        FreetypeFontFaceFileDescriptor(int fd, size_t offset, size_t length, FT_Library freetypeLib);

        bool init() override;

    private:
        BinaryOffsetFileInputStream m_fileStream;
        FT_StreamRec m_fontStream{};
    };
}
