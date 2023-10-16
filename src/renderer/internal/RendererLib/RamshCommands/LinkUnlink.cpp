//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/LinkUnlink.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    LinkBuffer::LinkBuffer(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Links an offscreen buffer to a scene";
        registerKeyword("link");
        getArgument<0>().setDescription("scene id");
        getArgument<1>().setDescription("data slot");
        getArgument<2>().setDescription("buffer id");
    }

    bool LinkBuffer::execute(uint64_t& sceneId, uint32_t& dataSlot, uint32_t& bufferId) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::LinkOffscreenBuffer{ OffscreenBufferHandle{bufferId}, SceneId{sceneId}, DataSlotId{dataSlot} });
        return true;
    }
}

namespace ramses::internal
{
    UnlinkBuffer::UnlinkBuffer(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Unlinks data from a scene";
        registerKeyword("unlink");
        getArgument<0>().setDescription("scene id");
        getArgument<1>().setDescription("data slot");
    }

    bool UnlinkBuffer::execute(uint64_t& sceneId, uint32_t& dataSlot) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::UnlinkData{ SceneId{sceneId}, DataSlotId{dataSlot} });
        return true;
    }
}
