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

        IOutputStream& write(const void* data, const UInt32 size) override;

        UInt32 getSize() const;

    private:
        UInt32 m_size;
    };

    inline
    VoidOutputStream::VoidOutputStream()
        : m_size(0u)
    {
    }

    inline IOutputStream& VoidOutputStream::write(const void* , const UInt32 size)
    {
        m_size += size;
        return *this;
    }

    inline UInt32 VoidOutputStream::getSize() const
    {
        return m_size;
    }
}

#endif
