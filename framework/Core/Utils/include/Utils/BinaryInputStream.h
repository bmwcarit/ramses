//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYINPUTSTREAM_H
#define RAMSES_BINARYINPUTSTREAM_H

#include "Collections/IInputStream.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformError.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    class BinaryInputStream: public IInputStream
    {
    public:
        explicit BinaryInputStream(const Char* input);
        explicit BinaryInputStream(const UChar* input);

        IInputStream& read(Char* buffer, UInt32 size) override;
        IInputStream& read(UChar* buffer, UInt32 size);

        virtual EStatus getState() const  override;

        const char* readPosition() const;

    private:
        const char* m_current;
    };

    inline BinaryInputStream::BinaryInputStream(const Char* input)
        : m_current(input)
    {
    }

    inline BinaryInputStream::BinaryInputStream(const UChar* input)
        : BinaryInputStream(reinterpret_cast<const Char*>(input))
    {
    }

    inline IInputStream& BinaryInputStream::read(Char* buffer, UInt32 size)
    {
        PlatformMemory::Copy(buffer, m_current, size);
        m_current += size;
        return *this;
    }

    inline IInputStream& BinaryInputStream::read(UChar* data, UInt32 size)
    {
        return read(reinterpret_cast<Char*>(data), size);
    }

    inline EStatus BinaryInputStream::getState() const
    {
        return EStatus_RAMSES_OK;
    }

    inline const char* BinaryInputStream::readPosition() const
    {
        return m_current;
    }
}

#endif
