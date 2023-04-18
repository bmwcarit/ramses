//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERBUFFERIMPL_H
#define RAMSES_RENDERBUFFERIMPL_H

#include "ramses-client-api/TextureEnums.h"

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "SceneAPI/Handles.h"

namespace ramses
{
    class RenderBufferImpl final : public SceneObjectImpl
    {
    public:
        RenderBufferImpl(SceneImpl& scene, const char* name);
        ~RenderBufferImpl() override;

        void             initializeFrameworkData(uint32_t width, uint32_t height, ERenderBufferType bufferType, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount);
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        status_t validate() const override;

        uint32_t            getWidth() const;
        uint32_t            getHeight() const;
        ERenderBufferType   getBufferType() const;
        ERenderBufferFormat getBufferFormat() const;
        ERenderBufferAccessMode getAccessMode() const;
        uint32_t            getSampleCount() const;

        ramses_internal::RenderBufferHandle getRenderBufferHandle() const;

    private:
        ramses_internal::RenderBufferHandle m_renderBufferHandle;
    };
}

#endif
