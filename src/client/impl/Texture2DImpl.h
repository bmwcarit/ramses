//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/TextureEnums.h"
#include "impl/ResourceImpl.h"
#include "ramses/client/TextureSwizzle.h"

#include <string_view>

namespace ramses::internal
{
    class Texture2DResource;
}

namespace ramses::internal
{
    class Texture2DImpl final : public ResourceImpl
    {
    public:
        // overrideType is required if another texture type is reusing the impl, but needs a different type ID
        Texture2DImpl(ramses::internal::ResourceHashUsage resource,
            SceneImpl& scene,
            std::string_view name,
            ERamsesObjectType overrideType = ERamsesObjectType::Texture2D);

        ~Texture2DImpl() override;

        void initializeFromFrameworkData(uint32_t width, uint32_t height, ETextureFormat textureFormat, const TextureSwizzle& swizzle);
        [[nodiscard]] bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        [[nodiscard]] bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] uint32_t       getWidth() const;
        [[nodiscard]] uint32_t       getHeight() const;
        [[nodiscard]] ETextureFormat getTextureFormat() const;
        [[nodiscard]] const TextureSwizzle& getTextureSwizzle() const;

    private:
        uint32_t m_width;
        uint32_t m_height;
        ETextureFormat m_textureFormat;
        TextureSwizzle m_swizzle;
    };
}

