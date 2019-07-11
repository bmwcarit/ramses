//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATALAYOUT_H
#define RAMSES_DATALAYOUT_H

#include "SceneAPI/DataFieldInfo.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    class DataLayout
    {
    public:
        DataLayout() = default;
        DataLayout(const DataLayout&) = default;
        DataLayout& operator=(const  DataLayout&) = default;
        DataLayout(DataLayout&&) = default;
        DataLayout& operator=(DataLayout&&) = default;

        void setDataFields(const DataFieldInfoVector& fields)
        {
            assert(m_fields.empty());
            assert(m_fieldOffsets.empty());

            m_fields = fields;
            m_fieldOffsets.reserve(fields.size());
            for (const auto& field : fields)
            {
                const uint32_t alignmentRequirement = static_cast<uint32_t>(EnumToAlignment(field.dataType));
                if (m_totalSize % alignmentRequirement != 0)
                    m_totalSize += alignmentRequirement - (m_totalSize % alignmentRequirement);

                m_fieldOffsets.push_back(m_totalSize);
                m_totalSize += static_cast<UInt32>(EnumToSize(field.dataType)) * field.elementCount;
            }
        }

        const DataFieldInfoVector& getDataFields() const
        {
            return m_fields;
        }

        UInt32 getFieldCount() const
        {
            return static_cast<UInt32>(m_fields.size());
        }

        const DataFieldInfo& getField(DataFieldHandle id) const
        {
            assert(id < m_fields.size());
            return m_fields[id.asMemoryHandle()];
        }

        UInt32 getFieldOffset(DataFieldHandle id) const
        {
            assert(id < m_fieldOffsets.size());
            return m_fieldOffsets[id.asMemoryHandle()];
        }

        UInt32 getTotalSize() const
        {
            return m_totalSize;
        }

    private:
        DataFieldInfoVector m_fields;
        std::vector<UInt32> m_fieldOffsets;
        UInt32 m_totalSize = 0u;
    };
}

#endif
