//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATALINKUTILS_H
#define RAMSES_DATALINKUTILS_H

#include "RendererLib/RendererScenes.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/DataSlot.h"

namespace ramses_internal
{
    class DataLinkUtils
    {
    public:
        static DataSlotHandle       GetDataSlotHandle(SceneId sceneId, DataSlotId slotId, const RendererScenes& scenes);
        static const DataSlot&      GetDataSlot(SceneId sceneId, DataSlotHandle slotHandle, const RendererScenes& scenes);
        static EDataType            GetSlotDataReferenceType(SceneId sceneId, DataSlotHandle slotHandle, const RendererScenes& scenes);
    };

    inline DataSlotHandle DataLinkUtils::GetDataSlotHandle(SceneId sceneId, DataSlotId slotId, const RendererScenes& scenes)
    {
        const IScene& scene = scenes.getScene(sceneId);
        const UInt32 slotCount = scene.getDataSlotCount();
        for (DataSlotHandle slotHandle(0u); slotHandle < slotCount; ++slotHandle)
        {
            if (scene.isDataSlotAllocated(slotHandle) &&
                scene.getDataSlot(slotHandle).id == slotId)
            {
                return slotHandle;
            }
        }

        return DataSlotHandle::Invalid();
    }

    inline const DataSlot& DataLinkUtils::GetDataSlot(SceneId sceneId, DataSlotHandle slotHandle, const RendererScenes& scenes)
    {
        const IScene& scene = scenes.getScene(sceneId);
        assert(scene.isDataSlotAllocated(slotHandle));
        return scene.getDataSlot(slotHandle);
    }

    inline EDataType DataLinkUtils::GetSlotDataReferenceType(SceneId sceneId, DataSlotHandle slotHandle, const RendererScenes& scenes)
    {
        const IScene& scene = scenes.getScene(sceneId);
        assert(scene.isDataSlotAllocated(slotHandle));
        const DataInstanceHandle dataRef = scene.getDataSlot(slotHandle).attachedDataReference;
        assert(scene.isDataInstanceAllocated(dataRef));
        const DataLayoutHandle dataLayout = scene.getLayoutOfDataInstance(dataRef);
        assert(scene.isDataLayoutAllocated(dataLayout));
        return scene.getDataLayout(dataLayout).getField(DataFieldHandle(0u)).dataType;
    }
}

#endif
