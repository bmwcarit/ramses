//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SETFRAMETIMELIMITS_H
#define RAMSES_SETFRAMETIMELIMITS_H

#include "Ramsh/RamshCommandArguments.h"
#include "RendererLib/FrameTimer.h"

namespace ramses_internal
{
    class SetFrameTimeLimits : public RamshCommandArgs<UInt32, UInt32>
    {
    public:
        SetFrameTimeLimits(FrameTimer& frametimer);
        virtual Bool execute(UInt32& limitForClientResourcesUploadMicrosec, UInt32& limitForOffscreenBufferRenderMicrosec) const override;

    private:
        FrameTimer& m_frametimer;
    };
}

#endif
