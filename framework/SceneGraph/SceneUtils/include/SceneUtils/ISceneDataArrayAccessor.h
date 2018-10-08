//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISCENEDATAARRAYACCESSOR_H
#define RAMSES_ISCENEDATAARRAYACCESSOR_H

#include "SceneAPI/IScene.h"

namespace ramses_internal
{
    class ISceneDataArrayAccessor
    {
    public:
        template <typename T>
        static const T* GetDataArray(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field);

        template <typename T>
        static void SetDataArray(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const T* data);
    };

    // array getter template specializations
    template <>
    inline const Float* ISceneDataArrayAccessor::GetDataArray<Float>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataFloatArray(containerHandle, field);
    }

    template <>
    inline const Vector2* ISceneDataArrayAccessor::GetDataArray<Vector2>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector2fArray(containerHandle, field);
    }

    template <>
    inline const Vector3* ISceneDataArrayAccessor::GetDataArray<Vector3>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector3fArray(containerHandle, field);
    }

    template <>
    inline const Vector4* ISceneDataArrayAccessor::GetDataArray<Vector4>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector4fArray(containerHandle, field);
    }

    template <>
    inline const Int32* ISceneDataArrayAccessor::GetDataArray<Int32>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataIntegerArray(containerHandle, field);
    }

    template <>
    inline const Vector2i* ISceneDataArrayAccessor::GetDataArray<Vector2i>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector2iArray(containerHandle, field);
    }

    template <>
    inline const Vector3i* ISceneDataArrayAccessor::GetDataArray<Vector3i>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector3iArray(containerHandle, field);
    }

    template <>
    inline const Vector4i* ISceneDataArrayAccessor::GetDataArray<Vector4i>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector4iArray(containerHandle, field);
    }

    template <>
    inline const Matrix22f* ISceneDataArrayAccessor::GetDataArray<Matrix22f>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataMatrix22fArray(containerHandle, field);
    }

    template <>
    inline const Matrix33f* ISceneDataArrayAccessor::GetDataArray<Matrix33f>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataMatrix33fArray(containerHandle, field);
    }

    template <>
    inline const Matrix44f* ISceneDataArrayAccessor::GetDataArray<Matrix44f>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataMatrix44fArray(containerHandle, field);
    }

    // array setter template specializations
    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Float>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data)
    {
        scene->setDataFloatArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Vector2>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data)
    {
        scene->setDataVector2fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Vector3>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data)
    {
        scene->setDataVector3fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Vector4>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data)
    {
        scene->setDataVector4fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Int32>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data)
    {
        scene->setDataIntegerArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Vector2i>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data)
    {
        scene->setDataVector2iArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Vector3i>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data)
    {
        scene->setDataVector3iArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Vector4i>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data)
    {
        scene->setDataVector4iArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Matrix22f>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data)
    {
        scene->setDataMatrix22fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Matrix33f>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data)
    {
        scene->setDataMatrix33fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Matrix44f>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data)
    {
        scene->setDataMatrix44fArray(containerHandle, field, elementCount, data);
    }
}

#endif
