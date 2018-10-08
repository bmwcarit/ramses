//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURECUBEIMPL_H
#define RAMSES_TEXTURECUBEIMPL_H

#include "ramses-client-api/TextureEnums.h"
#include "ResourceImpl.h"

namespace ramses_internal
{
    class TextureCubeResource;
}

namespace ramses
{
    class TextureCubeImpl final : public ResourceImpl
    {
    public:
        /**
         * @brief This creates a Cube Texture. All texels are pre-allocated and initialized to 0.
         */
        TextureCubeImpl(ramses_internal::ResourceHashUsage texture, RamsesClientImpl& client, const char* name);
        virtual ~TextureCubeImpl();

        void initializeFromFrameworkData(uint32_t size, ETextureFormat textureFormat);
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t       getSize() const;
        ETextureFormat getTextureFormat() const;

    private:
        uint32_t m_size;
        ETextureFormat m_textureFormat;

    };
}

#endif
