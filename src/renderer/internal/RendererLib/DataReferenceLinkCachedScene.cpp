//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DataReferenceLinkCachedScene.h"
#include "internal/RendererLib/SceneLinksManager.h"

namespace ramses::internal
{
    DataReferenceLinkCachedScene::DataReferenceLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : BaseT(sceneLinksManager, sceneInfo)
    {
    }

    DataSlotHandle DataReferenceLinkCachedScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        const DataSlotHandle actualHandle = BaseT::allocateDataSlot(dataSlot, handle);

        if (dataSlot.type == EDataSlotType::DataConsumer)
        {
            assert(dataSlot.attachedDataReference.isValid());
            m_fallbackValues.allocate(dataSlot.attachedDataReference);
            DataInstanceHelper::GetInstanceFieldData(*this, dataSlot.attachedDataReference, DataFieldHandle(0u), *m_fallbackValues.getMemory(dataSlot.attachedDataReference));
        }

        return actualHandle;
    }

    void DataReferenceLinkCachedScene::releaseDataSlot(DataSlotHandle handle)
    {
        const DataInstanceHandle dataRef = getDataSlot(handle).attachedDataReference;
        BaseT::releaseDataSlot(handle);
        if (m_fallbackValues.isAllocated(dataRef))
        {
            m_fallbackValues.release(dataRef);
        }
    }

    void DataReferenceLinkCachedScene::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data)
    {
        BaseT::setDataFloatArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data)
    {
        BaseT::setDataVector2fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data)
    {
        BaseT::setDataVector3fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data)
    {
        BaseT::setDataVector4fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data)
    {
        BaseT::setDataBooleanArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data)
    {
        BaseT::setDataIntegerArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data)
    {
        BaseT::setDataVector2iArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data)
    {
        BaseT::setDataVector3iArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data)
    {
        BaseT::setDataVector4iArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data)
    {
        BaseT::setDataMatrix22fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data)
    {
        BaseT::setDataMatrix33fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data)
    {
        BaseT::setDataMatrix44fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::restoreFallbackValue(DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        assert(m_fallbackValues.isAllocated(containerHandle));
        DataInstanceHelper::SetInstanceFieldData(*this, containerHandle, field, *m_fallbackValues.getMemory(containerHandle));
    }

    void DataReferenceLinkCachedScene::setValueWithoutUpdatingFallbackValue(DataInstanceHandle containerHandle, DataFieldHandle field, const DataInstanceValueVariant& value)
    {
        // store current fallback value
        // setting new value will update fallback value to the value we set
        const DataInstanceValueVariant fallbackValue = *m_fallbackValues.getMemory(containerHandle);

        DataInstanceHelper::SetInstanceFieldData(*this, containerHandle, field, value);

        // put fallback value back
        *m_fallbackValues.getMemory(containerHandle) = fallbackValue;
    }

    template <typename T>
    void DataReferenceLinkCachedScene::updateFallbackValue(DataInstanceHandle containerHandle, const T* data)
    {
        if (m_fallbackValues.isAllocated(containerHandle))
        {
            *m_fallbackValues.getMemory(containerHandle) = data[0];
        }
    }
}
