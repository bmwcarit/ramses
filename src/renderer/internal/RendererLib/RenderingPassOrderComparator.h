//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/RendererLib/RenderingPassInfo.h"

namespace ramses::internal
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
            const int32_t pass1RenderOrder = getRenderingPassRenderOrder(pass1);
            const int32_t pass2RenderOrder = getRenderingPassRenderOrder(pass2);
            return pass1RenderOrder < pass2RenderOrder;
        }

    private:
        [[nodiscard]] int32_t getRenderingPassRenderOrder(const RenderingPassInfo& pass) const
        {
            assert((ERenderingPassType::RenderPass == pass.getType() && m_scene.isRenderPassAllocated(pass.getRenderPassHandle()))
                || (ERenderingPassType::BlitPass == pass.getType() && m_scene.isBlitPassAllocated(pass.getBlitPassHandle())));
            return (ERenderingPassType::RenderPass == pass.getType() ? m_scene.getRenderPass(pass.getRenderPassHandle()).renderOrder : m_scene.getBlitPass(pass.getBlitPassHandle()).renderOrder);
        }

        const IScene& m_scene;
    };
}
