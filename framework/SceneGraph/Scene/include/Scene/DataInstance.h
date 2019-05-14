//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAINSTANCE_H
#define RAMSES_DATAINSTANCE_H

#include "Scene/DataLayout.h"

namespace ramses_internal
{
    class DataInstance
    {
    public:
        DataInstance()
        {
        }

        DataInstance(DataLayoutHandle dataLayoutHandle, UInt32 size)
            : m_dataLayoutHandle(dataLayoutHandle)
            , m_data(size)
        {
        }

        DataInstance(const DataInstance&) = default;
        DataInstance& operator=(const  DataInstance&) = default;
        DataInstance(DataInstance&&) = default;
        DataInstance& operator=(DataInstance&&) = default;

        template <typename DATATYPE>
        const DATATYPE* getTypedDataPointer(UInt32 fieldOffset) const
        {
            return reinterpret_cast<const DATATYPE*>(&m_data[fieldOffset]);
        }

        template <typename DATATYPE>
        void setTypedData(UInt32 fieldOffset, UInt32 elementCount, const DATATYPE* value)
        {
            const UInt32 fieldSizeInByte = sizeof(DATATYPE) * elementCount;
            assert(fieldOffset + fieldSizeInByte <= m_data.size());
            void* dest = &m_data[fieldOffset];
            if (dest != value)
            {
                PlatformMemory::Copy(dest, value, fieldSizeInByte);
            }
        }

        DataLayoutHandle getLayoutHandle() const
        {
            return m_dataLayoutHandle;
        }

    private:
        DataLayoutHandle m_dataLayoutHandle;
        std::vector<Byte> m_data;
    };
}

#endif
