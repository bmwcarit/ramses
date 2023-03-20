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
#include "ramses-framework-api/EDataType.h"

namespace ramses
{
    class ArrayResourceImpl final : public ResourceImpl
    {
    public:
        ArrayResourceImpl(ramses_internal::ResourceHashUsage arrayHash, SceneImpl& scene, const char* name);
        virtual ~ArrayResourceImpl() override;

        void initializeFromFrameworkData(uint32_t elementCount, EDataType elementType);
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t  getElementCount() const;
        EDataType getElementType() const;

    private:
        uint32_t  m_elementCount;
        EDataType m_elementType;
    };
}

#endif
