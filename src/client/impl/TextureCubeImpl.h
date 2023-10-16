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
    class TextureCubeResource;
}

namespace ramses::internal
{
    class TextureCubeImpl final : public ResourceImpl
    {
    public:
        /**
         * @brief This creates a Cube Texture. All texels are pre-allocated and initialized to 0.
         */
        TextureCubeImpl(ramses::internal::ResourceHashUsage texture, SceneImpl& scene, std::string_view name);
        ~TextureCubeImpl() override;

        void initializeFromFrameworkData(uint32_t size, ETextureFormat textureFormat, const TextureSwizzle& swizzle);
        [[nodiscard]] bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        [[nodiscard]] bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] uint32_t       getSize() const;
        [[nodiscard]] ETextureFormat getTextureFormat() const;
        [[nodiscard]] const TextureSwizzle& getTextureSwizzle() const;

    private:
        uint32_t m_size;
        ETextureFormat m_textureFormat;
        TextureSwizzle m_swizzle;
    };
}
