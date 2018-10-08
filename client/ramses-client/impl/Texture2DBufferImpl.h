//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE2DBUFFERIMPL_H
#define RAMSES_TEXTURE2DBUFFERIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/MipMapSize.h"
#include "ramses-client-api/EDataType.h"
#include "ramses-client-api/TextureEnums.h"

namespace ramses
{
    class Texture2DBufferImpl : public SceneObjectImpl
    {
    public:
        Texture2DBufferImpl(SceneImpl& scene, const char* textureBufferName);
        virtual ~Texture2DBufferImpl();

        void             initializeFrameworkData(const ramses_internal::MipMapDimensions& mipDimensions, ETextureFormat textureFormat);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        status_t setData(const ramses_internal::Byte* data, uint32_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height);
        uint32_t getMipLevelCount() const;
        ETextureFormat getTexelFormat() const;
        status_t getMipLevelData(uint32_t mipLevel, char* buffer, uint32_t bufferSize) const;
        status_t getMipLevelSize(uint32_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const;
        uint32_t getMipLevelDataSizeInBytes(uint32_t mipLevel) const;

        ramses_internal::TextureBufferHandle getTextureBufferHandle() const;

    private:
        ramses_internal::TextureBufferHandle m_textureBufferHandle;
    };
}

#endif
