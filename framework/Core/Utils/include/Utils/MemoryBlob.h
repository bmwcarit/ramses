//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_MEMORYBLOB_H
#define RAMSES_MEMORYBLOB_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/HeapArray.h"

namespace ramses_internal
{
    class CompressedMemoryBlob;

    /**
    * Container for raw byte data
    */
    class MemoryBlob
    {
    public:
        explicit MemoryBlob(UInt32 byteSize);
        explicit MemoryBlob(const void* data, UInt32 byteSize);
        explicit MemoryBlob(const CompressedMemoryBlob& compressedMemoryBlob);

        UInt32       size() const;
        const UInt8* getRawData() const;
        UInt8*       getRawData();

        UInt8 operator[](UInt32 index) const;
        UInt8& operator[](UInt32 index);

        void setDataToZero();

        MemoryBlob(const MemoryBlob&) = delete;
        MemoryBlob& operator=(const MemoryBlob&) = delete;
        MemoryBlob(MemoryBlob&&) = delete;
        MemoryBlob& operator=(MemoryBlob&&) = delete;

    private:
        HeapArray<UInt8> m_data;
    };

    inline
    UInt32 MemoryBlob::size() const
    {
        return static_cast<UInt32>(m_data.size());
    }

    inline
    const UInt8* MemoryBlob::getRawData() const
    {
        return m_data.data();
    }

    inline
    UInt8* MemoryBlob::getRawData()
    {
        return m_data.data();
    }

    inline
    UInt8 MemoryBlob::operator[](UInt32 index) const
    {
        return m_data.data()[index];
    }

    inline
    UInt8& MemoryBlob::operator[](UInt32 index)
    {
        return m_data.data()[index];
    }

    inline
    void MemoryBlob::setDataToZero()
    {
        m_data.setZero();
    }

}
#endif
