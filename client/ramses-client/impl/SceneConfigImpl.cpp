//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneConfigImpl.h"

namespace ramses
{
    status_t SceneConfigImpl::setPublicationMode(EScenePublicationMode publicationMode)
    {
        m_publicationMode = publicationMode;
        return StatusOK;
    }

    EScenePublicationMode SceneConfigImpl::getPublicationMode() const
    {
        return m_publicationMode;
    }

    status_t SceneConfigImpl::setMaximumLatency(uint32_t maxLatencyInMilliseconds)
    {
        m_maximumLatency = std::chrono::milliseconds(maxLatencyInMilliseconds);
        return StatusOK;
    }

    std::chrono::milliseconds SceneConfigImpl::getMaximumLatency() const
    {
        return m_maximumLatency;
    }
}
