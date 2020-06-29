//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINKSCENEDATA_H
#define RAMSES_LINKSCENEDATA_H

#include "Ramsh/RamshCommandArguments.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class LinkSceneData : public RamshCommandArgs<UInt32, UInt32, UInt32, UInt32>
    {
    public:
        explicit LinkSceneData(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool execute(UInt32& providerSceneId, UInt32& providerDataSlotId, UInt32& consumerSceneId, UInt32& consumerDataSlotId) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
