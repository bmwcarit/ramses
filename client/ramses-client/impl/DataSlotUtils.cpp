//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DataSlotUtils.h"
#include "Scene/ClientScene.h"

namespace ramses_internal
{
    namespace DataSlotUtils
    {
        bool HasDataSlotId(const ClientScene& scene, DataSlotId id)
        {
            const UInt32 slotHandleCount = scene.getDataSlotCount();
            for (DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
            {
                if (scene.isDataSlotAllocated(slotHandle) && scene.getDataSlot(slotHandle).id == id)
                {
                    return true;
                }
            }

            return false;
        }

        bool HasDataSlotIdForNode(const ClientScene& scene, NodeHandle nodeHandle)
        {
            const UInt32 slotHandleCount = scene.getDataSlotCount();
            for (DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
            {
                if (scene.isDataSlotAllocated(slotHandle) && scene.getDataSlot(slotHandle).attachedNode == nodeHandle)
                {
                    return true;
                }
            }

            return false;
        }

        bool HasDataSlotIdForDataObject(const ClientScene& scene, DataInstanceHandle dataRef)
        {
            const UInt32 slotHandleCount = scene.getDataSlotCount();
            for (DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
            {
                if (scene.isDataSlotAllocated(slotHandle) &&
                    scene.getDataSlot(slotHandle).attachedDataReference == dataRef)
                {
                    return true;
                }
            }

            return false;
        }

        bool HasDataSlotIdForTextureSampler(const ClientScene& scene, TextureSamplerHandle sampler)
        {
            const UInt32 slotHandleCount = scene.getDataSlotCount();
            for (DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
            {
                if (scene.isDataSlotAllocated(slotHandle) &&
                    scene.getDataSlot(slotHandle).attachedTextureSampler == sampler)
                {
                    return true;
                }
            }

            return false;
        }

        bool HasDataSlotIdForTexture(const ClientScene& scene, const ResourceContentHash& texture)
        {
            const UInt32 slotHandleCount = scene.getDataSlotCount();
            for (DataSlotHandle slotHandle(0u); slotHandle < slotHandleCount; slotHandle++)
            {
                if (scene.isDataSlotAllocated(slotHandle) &&
                    scene.getDataSlot(slotHandle).attachedTexture == texture)
                {
                    return true;
                }
            }

            return false;
        }
    }
}
