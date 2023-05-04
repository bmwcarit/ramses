//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERINGPASSORDERCOMPARATOR_H
#define RAMSES_RENDERINGPASSORDERCOMPARATOR_H

#include "SceneAPI/IScene.h"
#include "RendererLib/RenderingPassInfo.h"

namespace ramses_internal
{
    class RenderingPassOrderComparator
    {
    public:
        explicit RenderingPassOrderComparator(const IScene& scene)
            : m_scene(scene)
        {
        }

        bool operator()(const RenderingPassInfo& pass1, const RenderingPassInfo& pass2) const
        {
            const Int32 pass1RenderOrder = getRenderingPassRenderOrder(pass1);
            const Int32 pass2RenderOrder = getRenderingPassRenderOrder(pass2);
            return pass1RenderOrder < pass2RenderOrder;
        }

    private:
        [[nodiscard]] Int32 getRenderingPassRenderOrder(const RenderingPassInfo& pass) const
        {
            assert((ERenderingPassType::RenderPass == pass.getType() && m_scene.isRenderPassAllocated(pass.getRenderPassHandle()))
                || (ERenderingPassType::BlitPass == pass.getType() && m_scene.isBlitPassAllocated(pass.getBlitPassHandle())));
            return (ERenderingPassType::RenderPass == pass.getType() ? m_scene.getRenderPass(pass.getRenderPassHandle()).renderOrder : m_scene.getBlitPass(pass.getBlitPassHandle()).renderOrder);
        }

        const IScene& m_scene;
    };
}

#endif
