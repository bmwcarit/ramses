//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IINPUTSTREAM_H
#define RAMSES_IINPUTSTREAM_H

#include "PlatformAbstraction/PlatformError.h"
#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    class String;
    class Guid;
    class Matrix44f;
    struct ResourceContentHash;

    class IInputStream
    {
    public:
        virtual ~IInputStream();
        virtual IInputStream& operator>>(Int32& value) = 0;
        virtual IInputStream& operator>>(Int64& value) = 0;
        virtual IInputStream& operator>>(UInt32& value) = 0;
        virtual IInputStream& operator>>(UInt64& value) = 0;
        virtual IInputStream& operator>>(String&  value) = 0;
        virtual IInputStream& operator>>(Bool&  value) = 0;
        virtual IInputStream& operator>>(Float& value) = 0;
        virtual IInputStream& operator>>(UInt16& value) = 0;
        virtual IInputStream& operator>>(Guid& value) = 0;
        virtual IInputStream& operator>>(Matrix44f& value) = 0;
        virtual IInputStream& operator>>(ResourceContentHash& value) = 0;
        virtual IInputStream& read(Char* data, UInt32 size) = 0;
        virtual EStatus getState() const = 0;
    };

    inline
    IInputStream::~IInputStream()
    {
    }
}

#endif
