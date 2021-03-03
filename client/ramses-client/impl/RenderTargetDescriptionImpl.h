//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERTARGETDESCRIPTIONIMPL_H
#define RAMSES_RENDERTARGETDESCRIPTIONIMPL_H

#include "StatusObjectImpl.h"
#include "SceneAPI/SceneTypes.h"

namespace ramses
{
    class SceneImpl;
    class RenderBufferImpl;

    class RenderTargetDescriptionImpl : public StatusObjectImpl
    {
    public:
        RenderTargetDescriptionImpl();
        virtual ~RenderTargetDescriptionImpl() override;

        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        status_t addRenderBuffer(const RenderBufferImpl& renderBuffer);

        const ramses_internal::RenderBufferHandleVector& getRenderBuffers() const;

    private:
        const SceneImpl* m_scene;
        ramses_internal::RenderBufferHandleVector m_renderBuffers;
    };
}

#endif
