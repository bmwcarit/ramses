//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/BinaryOffsetFileInputStream.h"

namespace ramses_internal
{

    BinaryOffsetFileInputStream::BinaryOffsetFileInputStream(int fd, size_t offset, size_t length)
        : m_file(fdopen(fd, "rb"))
        , m_state(EStatus::Ok)
        , m_startPos(offset)
        , m_length(length)
        , m_pos(m_startPos)
    {
        if (!m_file)
        {
            m_state = EStatus::Error;
            return;
        }
        m_state = seek(0, Seek::FromBeginning);
    }

    BinaryOffsetFileInputStream::~BinaryOffsetFileInputStream()
    {
        if (m_file)
            fclose(m_file);
    }

    IInputStream& BinaryOffsetFileInputStream::read(void* buffer, size_t size)
    {
        if (EStatus::Ok != m_state)
            return *this;
        if (buffer == nullptr)
        {
            m_state = EStatus::Error;
            return *this;
        }
        if (m_pos + size > m_startPos + m_length)
        {
            m_state = EStatus::Eof;
            return *this;
        }

        const size_t bytesRead = fread(buffer, 1, size, m_file);
        m_pos += bytesRead;

        if (bytesRead != size)
        {
            if (feof(m_file))
                m_state = EStatus::Eof;
            else
                m_state = EStatus::Error;
        }

        return *this;
    }

    EStatus BinaryOffsetFileInputStream::seek(Int offset, Seek origin)
    {
        if (m_state != EStatus::Ok)
            return EStatus::Error;

        Int newPos = 0;
        switch (origin)
        {
        case Seek::FromBeginning:
            newPos = static_cast<Int>(m_startPos + offset);
            break;
        case Seek::Relative:
            newPos = static_cast<Int>(m_pos + offset);
            break;
        }
        if (newPos < static_cast<Int>(m_startPos) || newPos > static_cast<Int>(m_startPos + m_length))
            return EStatus::Error;

        if (fseek(m_file, static_cast<long>(newPos), SEEK_SET) != 0)
            return EStatus::Error;

        m_pos = newPos;

        return EStatus::Ok;
    }

    EStatus BinaryOffsetFileInputStream::getPos(size_t& position) const
    {
        if (m_state != EStatus::Ok)
            return EStatus::Error;
        position = m_pos - m_startPos;
        return EStatus::Ok;
    }

    EStatus BinaryOffsetFileInputStream::getState() const
    {
        return m_state;
    }
}
