//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERTARGETIMPL_H
#define RAMSES_RENDERTARGETIMPL_H

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "SceneAPI/Handles.h"

namespace ramses
{
    class RenderTargetDescriptionImpl;

    class RenderTargetImpl final : public SceneObjectImpl
    {
    public:
        RenderTargetImpl(SceneImpl& scene, const char* name);
        ~RenderTargetImpl() override;

        void             initializeFrameworkData(const RenderTargetDescriptionImpl& rtDesc);
        void     deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t getWidth() const;
        uint32_t getHeight() const;

        ramses_internal::RenderTargetHandle getRenderTargetHandle() const;

    private:
        ramses_internal::RenderTargetHandle m_renderTargetHandle;
    };
}

#endif
