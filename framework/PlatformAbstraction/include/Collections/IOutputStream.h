//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_IOUTPUTSTREAM_H
#define RAMSES_UTILS_IOUTPUTSTREAM_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformError.h>

namespace ramses_internal
{
    class String;
    class Guid;
    class IOutputStreamMonitor;
    class Matrix44f;
    struct ResourceContentHash;

    class IOutputStream
    {
    public:
        virtual ~IOutputStream();

        virtual IOutputStream& operator<<(const Int32 value) = 0;
        virtual IOutputStream& operator<<(const Int64 value) = 0;
        virtual IOutputStream& operator<<(const UInt64 value) = 0;
        virtual IOutputStream& operator<<(const UInt32 value) = 0;
        virtual IOutputStream& operator<<(const String&  value) = 0;
        virtual IOutputStream& operator<<(const Char*  value) = 0;
        virtual IOutputStream& operator<<(const Bool  value) = 0;
        virtual IOutputStream& operator<<(const Float value) = 0;
        virtual IOutputStream& operator<<(const UInt16 value) = 0;
        virtual IOutputStream& operator<<(const Guid& value) = 0;
        virtual IOutputStream& operator<<(const Matrix44f& value) = 0;
        virtual IOutputStream& operator<<(const ResourceContentHash& value) = 0;
        virtual IOutputStream& write(const void* data, const UInt32 size) = 0;
        virtual EStatus flush() = 0;
    };

    inline
    IOutputStream::~IOutputStream()
    {
    }
}

#endif
