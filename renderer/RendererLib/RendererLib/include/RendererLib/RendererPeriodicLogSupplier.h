//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERPERIODICLOGSUPPLIER_H
#define RAMSES_RENDERERPERIODICLOGSUPPLIER_H

#include "Utils/IPeriodicLogSupplier.h"

namespace ramses_internal
{
    class PeriodicLogger;
    class RendererCommandBuffer;

    class RendererPeriodicLogSupplier : public IPeriodicLogSupplier
    {
    public:
        RendererPeriodicLogSupplier(PeriodicLogger& periodicLogger, RendererCommandBuffer& commandBuffer);
        virtual ~RendererPeriodicLogSupplier();

        virtual void triggerLogMessageForPeriodicLog() const;

    private:
        PeriodicLogger& m_periodicLogger;
        RendererCommandBuffer& m_commandBuffer;
    };
}

#endif
