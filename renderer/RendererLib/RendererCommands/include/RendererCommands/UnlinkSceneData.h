//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UNLINKSCENEDATA_H
#define RAMSES_UNLINKSCENEDATA_H

#include "Ramsh/RamshCommandArguments.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class UnlinkSceneData : public RamshCommandArgs<UInt32, UInt32>
    {
    public:
        UnlinkSceneData(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool execute(UInt32& consumerSceneId, UInt32& consumerDataSlotId) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
