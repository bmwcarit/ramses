//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE3DIMPL_H
#define RAMSES_TEXTURE3DIMPL_H

#include "ramses-client-api/TextureEnums.h"
#include "ResourceImpl.h"

namespace ramses_internal
{
    class Texture3DResource;
}

namespace ramses
{
    class Texture3DImpl final : public ResourceImpl
    {
    public:
        Texture3DImpl(ramses_internal::ResourceHashUsage resource,
            SceneImpl& scene,
            const char* name);

        ~Texture3DImpl() override;

        void initializeFromFrameworkData(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat textureFormat);
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t       getWidth() const;
        uint32_t       getHeight() const;
        uint32_t       getDepth() const;
        ETextureFormat getTextureFormat() const;
    private:
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_depth;
        ETextureFormat m_textureFormat;
    };
}

#endif

