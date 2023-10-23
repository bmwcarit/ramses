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

#include <string_view>

namespace ramses::internal
{
    class Texture3DResource;
}

namespace ramses::internal
{
    class Texture3DImpl final : public ResourceImpl
    {
    public:
        Texture3DImpl(ramses::internal::ResourceHashUsage resource,
            SceneImpl& scene,
            std::string_view name);

        ~Texture3DImpl() override;

        void initializeFromFrameworkData(uint32_t width, uint32_t height, uint32_t depth, ETextureFormat textureFormat);
        [[nodiscard]] bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        [[nodiscard]] bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] uint32_t       getWidth() const;
        [[nodiscard]] uint32_t       getHeight() const;
        [[nodiscard]] uint32_t       getDepth() const;
        [[nodiscard]] ETextureFormat getTextureFormat() const;
    private:
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_depth;
        ETextureFormat m_textureFormat;
    };
}

