//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERINGPASSINFO_H
#define RAMSES_RENDERINGPASSINFO_H

#include "SceneAPI/IScene.h"

namespace ramses_internal
{
    enum class ERenderingPassType : UInt8
    {
        RenderPass = 0,
        BlitPass
    };

    struct RenderingPassInfo
    {
        RenderingPassInfo(RenderPassHandle handle)
            : m_type(ERenderingPassType::RenderPass)
            , m_handle(handle.asMemoryHandle())
        {
        }

        RenderingPassInfo(BlitPassHandle handle)
            : m_type(ERenderingPassType::BlitPass)
            , m_handle(handle.asMemoryHandle())
        {
        }

        ERenderingPassType getType() const
        {
            return m_type;
        }

        RenderPassHandle getRenderPassHandle() const
        {
            assert(m_type == ERenderingPassType::RenderPass);
            return RenderPassHandle(m_handle);
        }

        BlitPassHandle getBlitPassHandle() const
        {
            assert(m_type == ERenderingPassType::BlitPass);
            return BlitPassHandle(m_handle);
        }

    private:
        ERenderingPassType      m_type;
        MemoryHandle            m_handle;
    };

    typedef std::vector<RenderingPassInfo> RenderingPassInfoVector;
}

#endif
