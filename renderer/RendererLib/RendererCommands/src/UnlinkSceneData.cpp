//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/UnlinkSceneData.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    UnlinkSceneData::UnlinkSceneData(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "unlink consumer";
        registerKeyword("unlinkData");
        getArgument<0>().setDescription("consumer scene id");
        getArgument<1>().setDescription("consumer id");
    }

    Bool UnlinkSceneData::execute(UInt32& consumerSceneId, UInt32& consumerDataSlotId) const
    {
        m_rendererCommandBuffer.unlinkSceneData(SceneId(consumerSceneId), DataSlotId(consumerDataSlotId));
        return true;
    }
}
