//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYFILEINPUTSTREAM_H
#define RAMSES_BINARYFILEINPUTSTREAM_H

#include "Collections/IInputStream.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformError.h"
#include "Utils/File.h"

namespace ramses_internal
{
    class BinaryFileInputStream: public IInputStream
    {
    public:
        explicit BinaryFileInputStream(File& file);
        ~BinaryFileInputStream();

        BinaryFileInputStream(const BinaryFileInputStream&) = delete;
        BinaryFileInputStream& operator=(const BinaryFileInputStream&) = delete;

        IInputStream& read(Char* buffer, UInt32 size) override;

        EStatus seek(Int numberOfBytesToSeek, File::SeekOrigin origin);
        EStatus getPos(UInt& position);

        virtual EStatus getState() const override;

    private:
        File& m_file;
        EStatus m_state;
    };

    inline
    BinaryFileInputStream::BinaryFileInputStream(File& file)
        : m_file(file)
        , m_state(m_file.open(File::Mode::ReadOnlyBinary) ? EStatus::Ok : EStatus::Error)
    {
    }

    inline
    BinaryFileInputStream::~BinaryFileInputStream()
    {
        m_file.close();
    }

    inline
    IInputStream&
    BinaryFileInputStream::read(Char* data, UInt32 size)
    {
        if (EStatus::Ok == m_state)
        {
            UInt readBytes = 0;
            UInt numBytes = 0;
            while (readBytes < size)
            {
                EStatus retVal = m_file.read(data + readBytes, size, numBytes);
                readBytes += numBytes;
                if (retVal != EStatus::Ok)
                {
                    // error reading file, abort read method
                    // EOF is no error, but a valid return value, so we need a special handling here
                    m_state = retVal;
                    break;
                }
            }
        }
        return *this;
    }

    inline
    ramses_internal::EStatus
    BinaryFileInputStream::seek(Int numberOfBytesToSeek, File::SeekOrigin origin)
    {
        return m_file.seek(numberOfBytesToSeek, origin) ? EStatus::Ok : EStatus::Error;
    }

    inline
    ramses_internal::EStatus
    BinaryFileInputStream::getPos(UInt& position)
    {
        return m_file.getPos(position) ? EStatus::Ok : EStatus::Error;
    }

    inline
    ramses_internal::EStatus
    BinaryFileInputStream::getState() const
    {
        return m_state;
    }
}

#endif
