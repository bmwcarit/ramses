//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYFILEOUTPUTSTREAM_H
#define RAMSES_BINARYFILEOUTPUTSTREAM_H

#include "Collections/IOutputStream.h"
#include "Utils/File.h"

namespace ramses_internal
{
    class BinaryFileOutputStream: public IOutputStream
    {
    public:
        BinaryFileOutputStream(File& file, EFileMode mode = EFileMode_WriteNewBinary);
        ~BinaryFileOutputStream();

        BinaryFileOutputStream(const BinaryFileOutputStream&) = delete;
        BinaryFileOutputStream& operator=(const BinaryFileOutputStream&) = delete;

        IOutputStream& write(const void* data, const UInt32 size) override;

        EStatus getPos(UInt& position);
        EStatus getState() const;

    private:
        File& m_file;
        EStatus m_state;
    };

    inline
    BinaryFileOutputStream::BinaryFileOutputStream(File& file, EFileMode mode)
        : m_file(file)
        , m_state(m_file.open(mode))
    {
    }

    inline
    BinaryFileOutputStream::~BinaryFileOutputStream()
    {
        m_file.flush();
        m_file.close();
    }

    inline
    IOutputStream&
    BinaryFileOutputStream::write(const void* data, const UInt32 size)
    {
        if (EStatus_RAMSES_OK == m_state)
        {
            m_state = m_file.write(reinterpret_cast<const char*>(data), size);
        }
        return *this;
    }

    inline
    ramses_internal::EStatus BinaryFileOutputStream::getPos(UInt& position)
    {
        m_state = m_file.getPos(position);
        return m_state;
    }

    inline
    EStatus BinaryFileOutputStream::getState() const
    {
        return m_state;
    }
}

#endif
