//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/DataLayout.h"
#include "internal/Core/Utils/AssertMovable.h"

namespace ramses::internal
{
    class DataInstance
    {
    public:
        DataInstance() = default;

        DataInstance(DataLayoutHandle dataLayoutHandle, uint32_t size)
            : m_dataLayoutHandle(dataLayoutHandle)
            , m_data(size)
        {
        }

        DataInstance(const DataInstance&) = default;
        DataInstance& operator=(const  DataInstance&) = default;
        DataInstance(DataInstance&&) noexcept = default;
        DataInstance& operator=(DataInstance&&) noexcept = default;

        template <typename DATATYPE>
        [[nodiscard]] const DATATYPE* getTypedDataPointer(uint32_t fieldOffset) const
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) TODO(tobias) questionable if correct because because objects have no lifetime
            return reinterpret_cast<const DATATYPE*>(&m_data[fieldOffset]);
        }

        template <typename DATATYPE>
        void setTypedData(uint32_t fieldOffset, uint32_t elementCount, const DATATYPE* value)
        {
            const uint32_t fieldSizeInByte = sizeof(DATATYPE) * elementCount;
            assert(fieldOffset + fieldSizeInByte <= m_data.size());
            void* dest = &m_data[fieldOffset];
            if (dest != value)
            {
                PlatformMemory::Copy(dest, value, fieldSizeInByte);
            }
        }

        [[nodiscard]] DataLayoutHandle getLayoutHandle() const
        {
            return m_dataLayoutHandle;
        }

    private:
        DataLayoutHandle m_dataLayoutHandle;
        std::vector<std::byte> m_data;
    };

    ASSERT_MOVABLE(DataInstance)
}
