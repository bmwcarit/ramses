//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DataReferenceLinkCachedScene.h"
#include "RendererLib/SceneLinksManager.h"

namespace ramses_internal
{
    DataReferenceLinkCachedScene::DataReferenceLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : TransformationLinkCachedScene(sceneLinksManager, sceneInfo)
    {
    }

    DataSlotHandle DataReferenceLinkCachedScene::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        const DataSlotHandle actualHandle = TransformationLinkCachedScene::allocateDataSlot(dataSlot, handle);

        if (dataSlot.type == EDataSlotType_DataConsumer)
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
        TransformationLinkCachedScene::releaseDataSlot(handle);
        if (m_fallbackValues.isAllocated(dataRef))
        {
            m_fallbackValues.release(dataRef);
        }
    }

    void DataReferenceLinkCachedScene::setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data)
    {
        TransformationLinkCachedScene::setDataFloatArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data)
    {
        TransformationLinkCachedScene::setDataVector2fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data)
    {
        TransformationLinkCachedScene::setDataVector3fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data)
    {
        TransformationLinkCachedScene::setDataVector4fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data)
    {
        TransformationLinkCachedScene::setDataIntegerArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data)
    {
        TransformationLinkCachedScene::setDataVector2iArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data)
    {
        TransformationLinkCachedScene::setDataVector3iArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data)
    {
        TransformationLinkCachedScene::setDataVector4iArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data)
    {
        TransformationLinkCachedScene::setDataMatrix22fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data)
    {
        TransformationLinkCachedScene::setDataMatrix33fArray(containerHandle, field, elementCount, data);
        updateFallbackValue(containerHandle, data);
    }

    void DataReferenceLinkCachedScene::setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data)
    {
        TransformationLinkCachedScene::setDataMatrix44fArray(containerHandle, field, elementCount, data);
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
