//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/LinkSceneData.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    LinkSceneData::LinkSceneData(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "link consumer to provider";
        registerKeyword("linkData");
        getArgument<0>().setDescription("provider scene id");
        getArgument<1>().setDescription("provider id");
        getArgument<2>().setDescription("consumer scene id");
        getArgument<3>().setDescription("consumer id");
    }

    Bool LinkSceneData::execute(UInt32& providerSceneId, UInt32& providerDataSlotId, UInt32& consumerSceneId, UInt32& consumerDataSlotId) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses_internal::RendererCommand::LinkData{
            SceneId{providerSceneId}, DataSlotId{providerDataSlotId}, SceneId{consumerSceneId}, DataSlotId{consumerDataSlotId} });
        return true;
    }
}
