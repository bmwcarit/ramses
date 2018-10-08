//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererPeriodicLogSupplier.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "Utils/PeriodicLogger.h"

namespace ramses_internal
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

    void RendererPeriodicLogSupplier::triggerLogMessageForPeriodicLog() const
    {
        m_commandBuffer.logRendererInfo(ERendererLogTopic_PeriodicLog, false, ramses_internal::NodeHandle::Invalid());
    }
}
