//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DYNAMICQUAD_OFFSCREENRENDERTARGET_H
#define RAMSES_DYNAMICQUAD_OFFSCREENRENDERTARGET_H

#include "DynamicQuad_Base.h"

namespace ramses
{
    class RenderBuffer;
    class RenderTarget;
}

namespace ramses_internal
{
    struct RenderTargetResources
    {
        ramses::RenderBuffer*  renderBuffer = nullptr;
        ramses::RenderTarget*  renderTarget = nullptr;
        ramses::TextureSampler* textureSampler = nullptr;
    };

    class DynamicQuad_OffscreenRenderTarget : public DynamicQuad_Base
    {
    public:
        DynamicQuad_OffscreenRenderTarget(ramses::Scene& scene, ramses::RenderPass& offscreenRenderPass, const ScreenspaceQuad& screenspaceQuad);
        ~DynamicQuad_OffscreenRenderTarget() override;

        virtual void recreate() override final;

        // Needed for proper clean-up upon destruction (destroy scene -> mark objects destroyed -> destroy client resources)
        void markSceneObjectsDestroyed();

    private:
        RenderTargetResources   createRenderTarget();
        void destroyRenderTarget(const RenderTargetResources& renderTargetResources);

        ramses::RenderPass& m_offscreenRenderPass;

        RenderTargetResources m_renderTargetSceneObjects;
    };
}

#endif
