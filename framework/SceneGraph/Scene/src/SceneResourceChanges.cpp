//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneResourceChanges.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    void SceneResourceChanges::clear()
    {
        m_addedClientResourceRefs.clear();
        m_removedClientResourceRefs.clear();
        m_sceneResourceActions.clear();
    }

    Bool SceneResourceChanges::empty() const
    {
        return m_addedClientResourceRefs.empty()
            && m_removedClientResourceRefs.empty()
            && m_sceneResourceActions.empty();
    }

    template <typename ELEMENTTYPE>
    void putDataArray(SceneActionCollection& action, const std::vector<ELEMENTTYPE>& dataArray)
    {
        const UInt32 numElements = static_cast<UInt32>(dataArray.size());
        action.write(numElements);
        if (numElements > 0u)
        {
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
            const Byte* rawData = NULL;
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

    void SceneResourceChanges::putToSceneAction(SceneActionCollection& action) const
    {
        putDataArray(action, m_addedClientResourceRefs);
        putDataArray(action, m_removedClientResourceRefs);
        putDataArray(action, m_sceneResourceActions);
    }

    void SceneResourceChanges::getFromSceneAction(SceneActionCollection::SceneActionReader& action)
    {
        getDataArray(action, m_addedClientResourceRefs);
        getDataArray(action, m_removedClientResourceRefs);
        getDataArray(action, m_sceneResourceActions);
    }

    UInt SceneResourceChanges::getPutSizeEstimate() const
    {
        return estimatePutDataArraySize(m_addedClientResourceRefs) +
            estimatePutDataArraySize(m_removedClientResourceRefs) +
            estimatePutDataArraySize(m_sceneResourceActions);
    }

    const char* SceneResourceActionNames[ESceneResourceAction_NUMBER_OF_ELEMENTS] =
    {
        "Invalid",
        "CreateRenderBuffer",
        "DestroyRenderBuffer",
        "CreateRenderTarget",
        "DestroyRenderTarget",
        "CreateStreamTexture",
        "DestroyStreamTexture",
        "CreateBlitPass",
        "DestroyBlitPass",
        "CreateDataBuffer",
        "UpdateDataBuffer",
        "DestroyDataBuffer",
        "CreateTextureBuffer",
        "UpdateTextureBuffer",
        "DestroyTextureBuffer"
    };
    ENUM_TO_STRING(ESceneResourceAction, SceneResourceActionNames, ESceneResourceAction_NUMBER_OF_ELEMENTS);

    String SceneResourceChanges::asString() const
    {
        StringOutputStream str;
        str << "\n[ new client resources:";
        for (const auto& res : m_addedClientResourceRefs)
        {
            str << " " << StringUtils::HexFromResourceContentHash(res);
        }
        str << "]";

        str << "\n[ obsolete client resources:";
        for (const auto& res : m_removedClientResourceRefs)
        {
            str << " " << StringUtils::HexFromResourceContentHash(res);
        }
        str << "]";

        str << "\n[ scene resource actions:";
        for (const auto& res : m_sceneResourceActions)
        {
            str << " " << EnumToString(res.action) << ":" << res.handle;
        }
        str << "]";

        return str.release();
    }
}
