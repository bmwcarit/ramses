//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "BufferResource.h"

#include <cstdint>
#include <string_view>

namespace ramses::internal
{
    class ArrayResource : public BufferResource
    {
    public:
        ArrayResource(EResourceType arrayType, uint32_t elementCount, EDataType elementType, const void* arrayData, std::string_view name)
            : BufferResource(arrayType, elementCount * EnumToSize(elementType), arrayData, name)
            , m_elementCount(elementCount)
            , m_elementType(elementType)
        {
        }

        uint32_t getElementCount() const
        {
            return m_elementCount;
        }

        EDataType getElementType() const
        {
            return m_elementType;
        }

        void serializeResourceMetadataToStream(IOutputStream& output) const override
        {
            output << static_cast<uint32_t>(getElementCount());
            output << static_cast<uint32_t>(getElementType());
        }

        static std::unique_ptr<IResource> CreateResourceFromMetadataStream(IInputStream& input, EResourceType arrayType, std::string_view name)
        {
            uint32_t elementCount = 0;
            uint32_t elementTypeAsUInt = 0;

            input >> elementCount;
            input >> elementTypeAsUInt;

            const auto indexElementType = static_cast<EDataType>(elementTypeAsUInt);

            // Data for resource will be filled later
            return std::make_unique<ArrayResource>(arrayType, elementCount, indexElementType, nullptr, name);
        }

    private:
        uint32_t  m_elementCount;
        EDataType m_elementType;
    };
}
