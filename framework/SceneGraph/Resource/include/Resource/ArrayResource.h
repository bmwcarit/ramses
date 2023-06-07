//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_ARRAYRESOURCE_H
#define RAMSES_INTERNAL_ARRAYRESOURCE_H

#include "BufferResource.h"
#include "PlatformAbstraction/PlatformTypes.h"

#include <string_view>

namespace ramses_internal
{
    class ArrayResource : public BufferResource
    {
    public:
        ArrayResource(EResourceType arrayType, uint32_t elementCount, EDataType elementType, const void* arrayData, ResourceCacheFlag cacheFlag, std::string_view name)
            : BufferResource(arrayType, elementCount * EnumToSize(elementType), arrayData, cacheFlag, name)
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

        static std::unique_ptr<IResource> CreateResourceFromMetadataStream(IInputStream& input, ResourceCacheFlag cacheFlag, EResourceType arrayType, std::string_view name)
        {
            uint32_t elementCount = 0;
            uint32_t elementTypeAsUInt = 0;

            input >> elementCount;
            input >> elementTypeAsUInt;

            const EDataType indexElementType = static_cast<EDataType>(elementTypeAsUInt);

            // Data for resource will be filled later
            return std::make_unique<ArrayResource>(arrayType, elementCount, indexElementType, nullptr, cacheFlag, name);
        }

    private:
        uint32_t    m_elementCount;
        EDataType m_elementType;
    };
}

#endif
