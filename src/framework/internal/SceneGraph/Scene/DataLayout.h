//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/DataFieldInfo.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses::internal
{
    class DataLayout
    {
    public:
        void setDataFields(const DataFieldInfoVector& fields)
        {
            assert(m_fields.empty());
            assert(m_fieldOffsets.empty());

            m_fields = fields;
            m_fieldOffsets.reserve(fields.size());
            for (const auto& field : fields)
            {
                const auto alignmentRequirement = static_cast<uint32_t>(EnumToAlignment(field.dataType));
                if (m_totalSize % alignmentRequirement != 0)
                    m_totalSize += alignmentRequirement - (m_totalSize % alignmentRequirement);

                m_fieldOffsets.push_back(m_totalSize);
                m_totalSize += static_cast<uint32_t>(EnumToSize(field.dataType)) * field.elementCount;
            }
        }

        [[nodiscard]] const DataFieldInfoVector& getDataFields() const
        {
            return m_fields;
        }

        [[nodiscard]] uint32_t getFieldCount() const
        {
            return static_cast<uint32_t>(m_fields.size());
        }

        [[nodiscard]] const DataFieldInfo& getField(DataFieldHandle id) const
        {
            assert(id < m_fields.size());
            return m_fields[id.asMemoryHandle()];
        }

        [[nodiscard]] uint32_t getFieldOffset(DataFieldHandle id) const
        {
            assert(id < m_fieldOffsets.size());
            return m_fieldOffsets[id.asMemoryHandle()];
        }

        [[nodiscard]] uint32_t getTotalSize() const
        {
            return m_totalSize;
        }

        void setEffectHash(const ResourceContentHash& newHash)
        {
            effectHash = newHash;
        }

        [[nodiscard]] const ResourceContentHash& getEffectHash() const
        {
            return effectHash;
        }

    private:
        DataFieldInfoVector m_fields;
        std::vector<uint32_t> m_fieldOffsets;
        uint32_t m_totalSize = 0u;
        ResourceContentHash effectHash;
    };
}
