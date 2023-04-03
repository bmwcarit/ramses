//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE2DIMPL_H
#define RAMSES_TEXTURE2DIMPL_H

#include "ramses-client-api/TextureEnums.h"
#include "ResourceImpl.h"
#include "ramses-client-api/TextureSwizzle.h"

namespace ramses_internal
{
    class Texture2DResource;
}

namespace ramses
{
    class Texture2DImpl final : public ResourceImpl
    {
    public:
        // overrideType is required if another texture type is reusing the impl, but needs a different type ID
        Texture2DImpl(ramses_internal::ResourceHashUsage resource,
            SceneImpl& scene,
            const char* name,
            ERamsesObjectType overrideType = ERamsesObjectType_Texture2D);

        ~Texture2DImpl() override;

        void initializeFromFrameworkData(uint32_t width, uint32_t height, ETextureFormat textureFormat, const TextureSwizzle& swizzle);
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t       getWidth() const;
        uint32_t       getHeight() const;
        ETextureFormat getTextureFormat() const;
        const TextureSwizzle& getTextureSwizzle() const;

    private:
        uint32_t m_width;
        uint32_t m_height;
        ETextureFormat m_textureFormat;
        TextureSwizzle m_swizzle;
    };
}

#endif

