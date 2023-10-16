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
    void putDataArray(SceneActionCollection& action, const std::vector<ELEMENTTYPE>& dataArray)
    {
        const auto numElements = static_cast<uint32_t>(dataArray.size());
        action.write(numElements);
        if (numElements > 0u)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) TODO(tobias) questionable if correct because ELEMENTTYPE not POD
            const auto* rawData = reinterpret_cast<const std::byte*>(dataArray.data());
            const uint32_t size = numElements * sizeof(ELEMENTTYPE);
            action.write(rawData, size);
        }
    }

    template <typename ELEMENTTYPE>
    void getDataArray(SceneActionCollection::SceneActionReader& action, std::vector<ELEMENTTYPE>& dataArray)
    {
        uint32_t numElements = 0u;
        action.read(numElements);

        if (numElements > 0u)
        {
            const std::byte* rawData = nullptr;
            uint32_t size = 0u;
            action.readWithoutCopy(rawData, size);

            dataArray.resize(numElements);
            PlatformMemory::Copy(&dataArray.front(), rawData, size);
        }
        assert(dataArray.size() == numElements);
    }

    template <typename ELEMENTTYPE>
    size_t estimatePutDataArraySize(const std::vector<ELEMENTTYPE>& dataArray)
    {
        return sizeof(uint32_t) + sizeof(ELEMENTTYPE) * dataArray.size();
    }

    void ResourceChanges::putToSceneAction(SceneActionCollection& action) const
    {
        putDataArray(action, m_resourcesAdded);
        putDataArray(action, m_resourcesRemoved);
        putDataArray(action, m_sceneResourceActions);
    }

    void ResourceChanges::getFromSceneAction(SceneActionCollection::SceneActionReader& action)
    {
        getDataArray(action, m_resourcesAdded);
        getDataArray(action, m_resourcesRemoved);
        getDataArray(action, m_sceneResourceActions);
    }

    size_t ResourceChanges::getPutSizeEstimate() const
    {
        return estimatePutDataArraySize(m_resourcesAdded) +
            estimatePutDataArraySize(m_resourcesRemoved) +
            estimatePutDataArraySize(m_sceneResourceActions);
    }
}
