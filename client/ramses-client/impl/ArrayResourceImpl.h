//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYRESOURCEIMPL_H
#define RAMSES_ARRAYRESOURCEIMPL_H

// internal
#include "ResourceImpl.h"
#include "SceneAPI/EDataType.h"

namespace ramses
{
    class ArrayResourceImpl final : public ResourceImpl
    {
    public:
        ArrayResourceImpl(ramses_internal::ResourceHashUsage arrayHash, ERamsesObjectType type, RamsesClientImpl& client, const char* name);
        virtual ~ArrayResourceImpl();

        void initializeFromFrameworkData(uint32_t elementCount, ramses_internal::EDataType elementType);
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t                   getElementCount() const;
        ramses_internal::EDataType getElementType() const;

    private:
        uint32_t                   m_elementCount;
        ramses_internal::EDataType m_elementType;
    };
}

#endif
