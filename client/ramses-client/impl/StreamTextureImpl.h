//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STREAMTEXTUREIMPL_H
#define RAMSES_STREAMTEXTUREIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/ResourceContentHash.h"

namespace ramses
{
    class Texture2DImpl;

    class StreamTextureImpl final : public SceneObjectImpl
    {
    public:
        StreamTextureImpl(SceneImpl& client, const char* name);
        virtual ~StreamTextureImpl() override;

        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        void initializeFrameworkData(waylandIviSurfaceId_t source, const Texture2DImpl& fallbackTexture);
        virtual void deinitializeFrameworkData() override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        status_t forceFallbackImage(bool forceFallbackImage);
        bool getForceFallbackImage() const;

        waylandIviSurfaceId_t getStreamSource() const;
        ramses_internal::StreamTextureHandle getHandle() const;
        ramses_internal::ResourceContentHash getFallbackTextureHash() const;

    private:
        ramses_internal::StreamTextureHandle m_streamTextureHandle;
    };
}

#endif

