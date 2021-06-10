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

namespace ramses_internal
{
    FreetypeFontFace::FreetypeFontFace(FT_Library freetypeLib)
        : m_freetypeLib(freetypeLib)
    {
        assert(freetypeLib);
    }

    bool FreetypeFontFace::initFromOpenArgs(const FT_Open_Args* fontDataArgs)
    {
        {
            FT_Face localFaceForQuery = nullptr;
            int32_t error = FT_Open_Face(m_freetypeLib, fontDataArgs, -1, &localFaceForQuery);
            if (error != 0)
            {
                LOG_ERROR(CONTEXT_TEXT, "FreetypeFontFace::initFromOpenArgs: Failed to open face, FT error " << error);
                return false;
            }
            if (localFaceForQuery->num_faces < 1)
            {
                LOG_ERROR(CONTEXT_TEXT, "FreetypeFontFace::initFromOpenArgs: no font faces found in font data");
                FT_Done_Face(localFaceForQuery);
                return false;
            }
            else if (localFaceForQuery->num_faces > 1)
                LOG_INFO(CONTEXT_TEXT, "FreetypeFontFace::initFromOpenArgs: current implementation does not support multiple faces, face with index 0 will be used (" << localFaceForQuery->num_faces << " faces found in file)");

            // close temporary face
            FT_Done_Face(localFaceForQuery);
        }

        {
            // open again with face idx 0
            FT_Face localFace = nullptr;
            int32_t error = FT_Open_Face(m_freetypeLib, fontDataArgs, 0, &localFace);
            if (error != 0)
            {
                LOG_ERROR(CONTEXT_TEXT, "FreetypeFontFace::initFromOpenArgs: Failed to open face, FT error " << error);
                return false;
            }

            // leave size creation to instance

            m_face = localFace;
        }
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

    // =============================================================

    FreetypeFontFaceFilePath::FreetypeFontFaceFilePath(const char* fontPath, FT_Library freetypeLib)
        : FreetypeFontFace(freetypeLib)
        , m_fontPath(fontPath)
    {
        assert(fontPath);
    }

    bool FreetypeFontFaceFilePath::init()
    {
        FT_Open_Args openArgs;
        std::memset(&openArgs, 0, sizeof(openArgs));
        openArgs.flags = FT_OPEN_PATHNAME;
        openArgs.pathname = const_cast<char*>(m_fontPath.c_str());

        return initFromOpenArgs(&openArgs);
    }

    // =============================================================

    namespace
    {
        unsigned long ReadFTStream(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
        {
            assert(stream);
            assert(stream->descriptor.pointer);

            BinaryOffsetFileInputStream* bofis = static_cast<BinaryOffsetFileInputStream*>(stream->descriptor.pointer);
            bofis->seek(offset, IInputStream::Seek::FromBeginning);
            if (count > 0)
                bofis->read(buffer, count);
            return count;
        }

        void CloseFTStream(FT_Stream /*stream*/)
        {
            // When this gets called freetype tells us it does not have a reference on the FT_Stream (aka FT_StreamRec*)
            // anymore. It is the result of closing the face opened with FT_OPEN_STREAM. We could not free all memory
            // associated with it and close the open filedescriptor.

            // We on purpose ignore this callback because our FT_StreamRec lifecycle and also the filedescriptor is
            // bound to FreetypeFontFaceFileDescriptor object and we free all memory and close the fd in its (implicit)
            // destructor.
        }
    }

    FreetypeFontFaceFileDescriptor::FreetypeFontFaceFileDescriptor(int fd, size_t offset, size_t length, FT_Library freetypeLib)
        : FreetypeFontFace(freetypeLib)
        , m_fileStream(fd, offset, length)
    {
        // see https://www.freetype.org/freetype2/docs/reference/ft2-system_interface.html#ft_streamrec
        std::memset(&m_fontStream, 0, sizeof(m_fontStream));
        m_fontStream.size = static_cast<unsigned long>(length);
        m_fontStream.read  = ReadFTStream;
        m_fontStream.close = CloseFTStream;
        m_fontStream.descriptor.pointer = &m_fileStream;
    }

    bool FreetypeFontFaceFileDescriptor::init()
    {
        FT_Open_Args openArgs;
        std::memset(&openArgs, 0, sizeof(openArgs));
        openArgs.flags = FT_OPEN_STREAM;
        openArgs.stream = &m_fontStream;

        return initFromOpenArgs(&openArgs);
    }
}
