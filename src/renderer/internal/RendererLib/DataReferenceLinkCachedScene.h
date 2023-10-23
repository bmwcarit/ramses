//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/TransformationLinkCachedScene.h"
#include "internal/SceneGraph/SceneUtils/DataInstanceHelper.h"

namespace ramses::internal
{
    class DataReferenceLinkCachedScene : public TransformationLinkCachedScene
    {
    public:
        explicit DataReferenceLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        DataSlotHandle          allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle) override;
        void                    releaseDataSlot(DataSlotHandle handle) override;

        // Listen on data changes in order to handle fallback values
        void                    setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data) override;
        void                    setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data) override;
        void                    setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data) override;
        void                    setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data) override;
        void                    setDataBooleanArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data) override;
        void                    setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data) override;
        void                    setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data) override;
        void                    setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data) override;
        void                    setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data) override;
        void                    setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data) override;
        void                    setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data) override;
        void                    setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data) override;

        void restoreFallbackValue(DataInstanceHandle containerHandle, DataFieldHandle field);
        void setValueWithoutUpdatingFallbackValue(DataInstanceHandle containerHandle, DataFieldHandle field, const DataInstanceValueVariant& value);

    private:
        template <typename T>
        void updateFallbackValue(DataInstanceHandle containerHandle, const T* data);

        using FallbackValuePool = MemoryPool<DataInstanceValueVariant, DataInstanceHandle>;
        FallbackValuePool m_fallbackValues;
    };
}
