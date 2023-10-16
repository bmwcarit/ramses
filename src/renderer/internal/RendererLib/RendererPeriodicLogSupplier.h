//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/IPeriodicLogSupplier.h"

namespace ramses::internal
{
    class PeriodicLogger;
    class RendererCommandBuffer;

    class RendererPeriodicLogSupplier : public IPeriodicLogSupplier
    {
    public:
        RendererPeriodicLogSupplier(PeriodicLogger& periodicLogger, RendererCommandBuffer& commandBuffer);
        ~RendererPeriodicLogSupplier() override;

        void triggerLogMessageForPeriodicLog() override;

    private:
        PeriodicLogger& m_periodicLogger;
        RendererCommandBuffer& m_commandBuffer;
    };
}
