//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/ResourceChanges.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
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
        const UInt32 numElements = static_cast<UInt32>(dataArray.size());
        action.write(numElements);
        if (numElements > 0u)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) TODO(tobias) questionable if correct because ELEMENTTYPE not POD
            const Byte* rawData = reinterpret_cast<const Byte*>(dataArray.data());
            const UInt32 size = numElements * sizeof(ELEMENTTYPE);
            action.write(rawData, size);
        }
    }

    template <typename ELEMENTTYPE>
    void getDataArray(SceneActionCollection::SceneActionReader& action, std::vector<ELEMENTTYPE>& dataArray)
    {
        UInt32 numElements = 0u;
        action.read(numElements);

        if (numElements > 0u)
        {
            const Byte* rawData = nullptr;
            UInt32 size = 0u;
            action.readWithoutCopy(rawData, size);

            dataArray.resize(numElements);
            PlatformMemory::Copy(&dataArray.front(), rawData, size);
        }
        assert(dataArray.size() == numElements);
    }

    template <typename ELEMENTTYPE>
    UInt estimatePutDataArraySize(const std::vector<ELEMENTTYPE>& dataArray)
    {
        return sizeof(UInt32) + sizeof(ELEMENTTYPE) * dataArray.size();
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

    UInt ResourceChanges::getPutSizeEstimate() const
    {
        return estimatePutDataArraySize(m_resourcesAdded) +
            estimatePutDataArraySize(m_resourcesRemoved) +
            estimatePutDataArraySize(m_sceneResourceActions);
    }
}
