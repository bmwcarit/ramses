//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneObjectImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/MipMapSize.h"
#include "ramses/framework/EDataType.h"
#include "ramses/framework/TextureEnums.h"

#include <string_view>

namespace ramses::internal
{
    class Texture2DBufferImpl : public SceneObjectImpl
    {
    public:
        Texture2DBufferImpl(SceneImpl& scene, std::string_view textureBufferName);
        ~Texture2DBufferImpl() override;

        void initializeFrameworkData(const ramses::internal::MipMapDimensions& mipDimensions, ETextureFormat textureFormat);
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        bool setData(const std::byte* data, size_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);
        [[nodiscard]] size_t getMipLevelCount() const;
        [[nodiscard]] ETextureFormat getTexelFormat() const;
        bool getMipLevelData(size_t mipLevel, char* buffer, size_t bufferSize) const;
        bool getMipLevelSize(size_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const;
        [[nodiscard]] size_t getMipLevelDataSizeInBytes(size_t mipLevel) const;

        [[nodiscard]] ramses::internal::TextureBufferHandle getTextureBufferHandle() const;

    private:
        ramses::internal::TextureBufferHandle m_textureBufferHandle;
    };
}
