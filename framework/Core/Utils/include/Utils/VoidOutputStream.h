//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VOIDOUTPUTSTREAM_H
#define RAMSES_VOIDOUTPUTSTREAM_H

#include <PlatformAbstraction/PlatformTypes.h>
#include "Collections/IOutputStream.h"

namespace ramses_internal
{
    class VoidOutputStream: public IOutputStream
    {
    public:
        explicit VoidOutputStream();

        IOutputStream& write(const void* data, size_t size) override;
        virtual EStatus getPos(size_t& position) const override;

        size_t getSize() const;

    private:
        size_t m_size;
    };

    inline
    VoidOutputStream::VoidOutputStream()
        : m_size(0u)
    {
    }

    inline IOutputStream& VoidOutputStream::write(const void*, size_t size)
    {
        m_size += size;
        return *this;
    }

    inline EStatus VoidOutputStream::getPos(size_t& position) const
    {
        position = m_size;
        return EStatus::Ok;
    }

    inline size_t VoidOutputStream::getSize() const
    {
        return m_size;
    }
}

#endif
