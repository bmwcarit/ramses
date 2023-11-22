//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererPeriodicLogSupplier.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/Core/Utils/PeriodicLogger.h"

namespace ramses::internal
{
    RendererPeriodicLogSupplier::RendererPeriodicLogSupplier(PeriodicLogger& periodicLogger, RendererCommandBuffer& commandBuffer)
        : m_periodicLogger(periodicLogger)
        , m_commandBuffer(commandBuffer)
    {
        m_periodicLogger.registerPeriodicLogSupplier(this);
    }

    RendererPeriodicLogSupplier::~RendererPeriodicLogSupplier()
    {
        m_periodicLogger.removePeriodicLogSupplier(this);
    }

    void RendererPeriodicLogSupplier::triggerLogMessageForPeriodicLog()
    {
        // periodic log renderer info stored in the command will be filled with actual values when dispatching the command
        m_commandBuffer.enqueueCommand(RendererCommand::LogInfo{ ERendererLogTopic::PeriodicLog, false, {}, false, false, ELoopMode::UpdateAndRender, {} });
    }
}
