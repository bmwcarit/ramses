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
        ~BinaryFileInputStream() override;

        BinaryFileInputStream(const BinaryFileInputStream&) = delete;
        BinaryFileInputStream& operator=(const BinaryFileInputStream&) = delete;

        IInputStream& read(void* buffer, size_t size) override;

        EStatus seek(Int numberOfBytesToSeek, Seek origin) override;
        EStatus getPos(size_t& position) const override;

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
    BinaryFileInputStream::read(void* data, size_t size)
    {
        if (EStatus::Ok == m_state)
        {
            size_t readBytes = 0;
            size_t numBytes = 0;
            while (readBytes < size)
            {
                EStatus retVal = m_file.read(static_cast<Byte*>(data) + readBytes, size, numBytes);
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
    BinaryFileInputStream::seek(Int numberOfBytesToSeek, Seek origin)
    {
        File::SeekOrigin fileOrigin = File::SeekOrigin::BeginningOfFile;
        if (origin == Seek::FromBeginning)
            fileOrigin = File::SeekOrigin::BeginningOfFile;
        else if (origin == Seek::Relative)
            fileOrigin = File::SeekOrigin::RelativeToCurrentPosition;
        return m_file.seek(numberOfBytesToSeek, fileOrigin) ? EStatus::Ok : EStatus::Error;
    }

    inline
    ramses_internal::EStatus
    BinaryFileInputStream::getPos(size_t& position) const
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
