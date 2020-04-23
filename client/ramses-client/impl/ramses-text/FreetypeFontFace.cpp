//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "FreetypeFontFace.h"
#include "Utils/LogMacros.h"
#include <cassert>

namespace ramses
{
    FreetypeFontFace::FreetypeFontFace(const char* fontPath, FT_Library freetypeLib)
        : m_fontPath(fontPath)
        , m_freetypeLib(freetypeLib)
    {
        assert(fontPath);
        assert(freetypeLib);
    }

    bool FreetypeFontFace::init()
    {
        // open and check for single face
        FT_Open_Args fontDataArgs;
        fontDataArgs.flags = FT_OPEN_PATHNAME;
        fontDataArgs.pathname = const_cast<char*>(m_fontPath.data());
        fontDataArgs.num_params = 0;

        FT_Face localFace = nullptr;
        int32_t error = FT_Open_Face(m_freetypeLib, &fontDataArgs, -1, &localFace);
        if (error != 0)
        {
            LOG_ERROR(CONTEXT_TEXT, "FreetypeFontFace::init: Failed to open face, FT error " << error);
            return false;
        }
        if (localFace->num_faces < 1)
        {
            LOG_ERROR(CONTEXT_TEXT, "FreetypeFontFace::init: no font faces found in font data");
            return false;
        }
        else if (localFace->num_faces > 1)
            LOG_INFO(CONTEXT_TEXT, "FreetypeFontFace::init: current implementation does not support multiple faces, face with index 0 will be used (" << localFace->num_faces << " faces found in file)");

        // open again with face idx 0
        error = FT_Open_Face(m_freetypeLib, &fontDataArgs, 0, &localFace);
        if (error != 0)
        {
            LOG_ERROR(CONTEXT_TEXT, "FreetypeFontFace::init: Failed to open face, FT error " << error);
            return false;
        }

        // leave size creation to instance

        m_face = localFace;
        return true;
    }

    FreetypeFontFace::~FreetypeFontFace()
    {
        if (m_face)
            FT_Done_Face(m_face);
    }

    FT_Face FreetypeFontFace::getFace()
    {
        return m_face;
    }
}
