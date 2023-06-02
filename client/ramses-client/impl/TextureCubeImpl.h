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
#include "ramses-client-api/TextureSwizzle.h"

#include <string_view>

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
        TextureCubeImpl(ramses_internal::ResourceHashUsage texture, SceneImpl& scene, std::string_view name);
        ~TextureCubeImpl() override;

        void initializeFromFrameworkData(uint32_t size, ETextureFormat textureFormat, const TextureSwizzle& swizzle);
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t       getSize() const;
        ETextureFormat getTextureFormat() const;
        const TextureSwizzle& getTextureSwizzle() const;

    private:
        uint32_t m_size;
        ETextureFormat m_textureFormat;
        TextureSwizzle m_swizzle;
    };
}

#endif
