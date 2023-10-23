//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/IScene.h"

namespace ramses::internal
{
    enum class ERenderingPassType : uint8_t
    {
        RenderPass = 0,
        BlitPass
    };

    struct RenderingPassInfo
    {
        explicit RenderingPassInfo(RenderPassHandle handle)
            : m_type(ERenderingPassType::RenderPass)
            , m_handle(handle.asMemoryHandle())
        {
        }

        explicit RenderingPassInfo(BlitPassHandle handle)
            : m_type(ERenderingPassType::BlitPass)
            , m_handle(handle.asMemoryHandle())
        {
        }

        [[nodiscard]] ERenderingPassType getType() const
        {
            return m_type;
        }

        [[nodiscard]] RenderPassHandle getRenderPassHandle() const
        {
            assert(m_type == ERenderingPassType::RenderPass);
            return RenderPassHandle(m_handle);
        }

        [[nodiscard]] BlitPassHandle getBlitPassHandle() const
        {
            assert(m_type == ERenderingPassType::BlitPass);
            return BlitPassHandle(m_handle);
        }

    private:
        ERenderingPassType      m_type;
        MemoryHandle            m_handle;
    };

    using RenderingPassInfoVector = std::vector<RenderingPassInfo>;
}
