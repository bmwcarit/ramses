//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// internal
#include "impl/ResourceImpl.h"
#include "ramses/framework/EDataType.h"

#include <string_view>

namespace ramses::internal
{
    class ArrayResourceImpl final : public ResourceImpl
    {
    public:
        ArrayResourceImpl(ResourceHashUsage arrayHash, SceneImpl& scene, std::string_view name);
        ~ArrayResourceImpl() override;

        void initializeFromFrameworkData(size_t elementCount, ramses::EDataType elementType);
        bool serialize(IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] size_t            getElementCount() const;
        [[nodiscard]] ramses::EDataType getElementType() const;

    private:
        uint32_t          m_elementCount;
        ramses::EDataType m_elementType;
    };
}
