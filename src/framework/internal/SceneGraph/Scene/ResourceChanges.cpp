//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/ResourceChanges.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"

namespace ramses::internal
{
    void ResourceChanges::clear()
    {
        m_resourcesAdded.clear();
        m_resourcesRemoved.clear();
        m_sceneResourceActions.clear();
    }

    bool ResourceChanges::empty() const
    {
        return m_resourcesAdded.empty()
            && m_resourcesRemoved.empty()
            && m_sceneResourceActions.empty();
    }

    template <typename ELEMENTTYPE>
    size_t estimatePutDataArraySize(const std::vector<ELEMENTTYPE>& dataArray)
    {
        return sizeof(uint32_t) + sizeof(ELEMENTTYPE) * dataArray.size();
    }

    size_t ResourceChanges::getPutSizeEstimate() const
    {
        return estimatePutDataArraySize(m_resourcesAdded) +
            estimatePutDataArraySize(m_resourcesRemoved) +
            estimatePutDataArraySize(m_sceneResourceActions);
    }
}
