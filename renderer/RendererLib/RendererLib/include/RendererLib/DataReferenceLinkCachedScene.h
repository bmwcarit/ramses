//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAREFERENCELINKCACHEDSCENE_H
#define RAMSES_DATAREFERENCELINKCACHEDSCENE_H

#include "RendererLib/TransformationLinkCachedScene.h"
#include "Utils/Variant.h"

namespace ramses_internal
{
    class DataReferenceLinkCachedScene : public TransformationLinkCachedScene
    {
    public:
        DataReferenceLinkCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        // From IScene
        virtual DataSlotHandle          allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                    releaseDataSlot(DataSlotHandle handle) override;

        // Listen on data changes in order to handle fallback values
        virtual void                    setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data) override;
        virtual void                    setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data) override;
        virtual void                    setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data) override;
        virtual void                    setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data) override;
        virtual void                    setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data) override;
        virtual void                    setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data) override;
        virtual void                    setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data) override;
        virtual void                    setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data) override;
        virtual void                    setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data) override;
        virtual void                    setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data) override;
        virtual void                    setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data) override;

        void restoreFallbackValue(DataInstanceHandle containerHandle, DataFieldHandle field);
        void setValueWithoutUpdatingFallbackValue(DataInstanceHandle containerHandle, DataFieldHandle field, const Variant& value);

    private:
        template <typename T>
        void updateFallbackValue(DataInstanceHandle containerHandle, const T* data);

        typedef MemoryPool<Variant, DataInstanceHandle> FallbackValuePool;
        FallbackValuePool m_fallbackValues;
    };
}

#endif
