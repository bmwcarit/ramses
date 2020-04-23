//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SetFrameTimeLimits.h"
#include "Ramsh/RamshInput.h"

namespace ramses_internal
{
    SetFrameTimeLimits::SetFrameTimeLimits(FrameTimer& frametimer)
        : m_frametimer(frametimer)
    {
        description = "change values (us): limitForClientResourcesUpload limitForSceneActionsApply limitForOffscreenBufferRender";
        registerKeyword("limits");
    }

    Bool SetFrameTimeLimits::execute(UInt32& limitForClientResourcesUploadMicrosec, UInt32& limitForOffscreenBufferRenderMicrosec) const
    {
        m_frametimer.setSectionTimeBudget(EFrameTimerSectionBudget::ClientResourcesUpload, limitForClientResourcesUploadMicrosec);
        m_frametimer.setSectionTimeBudget(EFrameTimerSectionBudget::OffscreenBufferRender, limitForOffscreenBufferRenderMicrosec);

        LOG_INFO(CONTEXT_RENDERER, "Changed limiting values:" << limitForClientResourcesUploadMicrosec <<  "/" << limitForOffscreenBufferRenderMicrosec);
        return true;
    }
}
