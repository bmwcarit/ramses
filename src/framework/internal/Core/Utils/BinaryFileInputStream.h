//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/PlatformAbstraction/PlatformError.h"
#include "internal/Core/Utils/File.h"

#include <cstdint>

namespace ramses::internal
{
    class BinaryFileInputStream: public IInputStream
    {
    public:
        explicit BinaryFileInputStream(File& file);
        ~BinaryFileInputStream() override;

        BinaryFileInputStream(const BinaryFileInputStream&) = delete;
        BinaryFileInputStream& operator=(const BinaryFileInputStream&) = delete;

        IInputStream& read(void* buffer, size_t size) override;

        EStatus seek(int64_t numberOfBytesToSeek, Seek origin) override;
        EStatus getPos(size_t& position) const override;

        [[nodiscard]] EStatus getState() const override;

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
    BinaryFileInputStream::read(void* buffer, size_t size)
    {
        if (m_state != EStatus::Ok)
            return *this;
        size_t numBytesRead = 0;
        m_state = m_file.read(buffer, size, numBytesRead);
        return *this;
    }

    inline
    ramses::internal::EStatus
    BinaryFileInputStream::seek(int64_t numberOfBytesToSeek, Seek origin)
    {
        File::SeekOrigin fileOrigin = File::SeekOrigin::BeginningOfFile;
        if (origin == Seek::FromBeginning)
        {
            fileOrigin = File::SeekOrigin::BeginningOfFile;
        }
        else if (origin == Seek::Relative)
        {
            fileOrigin = File::SeekOrigin::RelativeToCurrentPosition;
        }
        return m_file.seek(numberOfBytesToSeek, fileOrigin) ? EStatus::Ok : EStatus::Error;
    }

    inline
    ramses::internal::EStatus
    BinaryFileInputStream::getPos(size_t& position) const
    {
        return m_file.getPos(position) ? EStatus::Ok : EStatus::Error;
    }

    inline
    ramses::internal::EStatus
    BinaryFileInputStream::getState() const
    {
        return m_state;
    }
}
