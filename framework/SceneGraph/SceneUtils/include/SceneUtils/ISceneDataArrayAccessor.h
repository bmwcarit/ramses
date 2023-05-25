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
    inline const float* ISceneDataArrayAccessor::GetDataArray<float>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataFloatArray(containerHandle, field);
    }

    template <>
    inline const glm::vec2* ISceneDataArrayAccessor::GetDataArray<glm::vec2>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector2fArray(containerHandle, field);
    }

    template <>
    inline const glm::vec3* ISceneDataArrayAccessor::GetDataArray<glm::vec3>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector3fArray(containerHandle, field);
    }

    template <>
    inline const glm::vec4* ISceneDataArrayAccessor::GetDataArray<glm::vec4>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector4fArray(containerHandle, field);
    }

    template <>
    inline const Int32* ISceneDataArrayAccessor::GetDataArray<Int32>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataIntegerArray(containerHandle, field);
    }

    template <>
    inline const glm::ivec2* ISceneDataArrayAccessor::GetDataArray<glm::ivec2>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector2iArray(containerHandle, field);
    }

    template <>
    inline const glm::ivec3* ISceneDataArrayAccessor::GetDataArray<glm::ivec3>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector3iArray(containerHandle, field);
    }

    template <>
    inline const glm::ivec4* ISceneDataArrayAccessor::GetDataArray<glm::ivec4>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataVector4iArray(containerHandle, field);
    }

    template <>
    inline const glm::mat2* ISceneDataArrayAccessor::GetDataArray<glm::mat2>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataMatrix22fArray(containerHandle, field);
    }

    template <>
    inline const glm::mat3* ISceneDataArrayAccessor::GetDataArray<glm::mat3>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataMatrix33fArray(containerHandle, field);
    }

    template <>
    inline const glm::mat4* ISceneDataArrayAccessor::GetDataArray<glm::mat4>(const IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field)
    {
        return scene->getDataMatrix44fArray(containerHandle, field);
    }

    // array setter template specializations
    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<float>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const float* data)
    {
        scene->setDataFloatArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::vec2>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::vec2* data)
    {
        scene->setDataVector2fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::vec3>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::vec3* data)
    {
        scene->setDataVector3fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::vec4>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::vec4* data)
    {
        scene->setDataVector4fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<Int32>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data)
    {
        scene->setDataIntegerArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::ivec2>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::ivec2* data)
    {
        scene->setDataVector2iArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::ivec3>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::ivec3* data)
    {
        scene->setDataVector3iArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::ivec4>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::ivec4* data)
    {
        scene->setDataVector4iArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::mat2>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::mat2* data)
    {
        scene->setDataMatrix22fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::mat3>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::mat3* data)
    {
        scene->setDataMatrix33fArray(containerHandle, field, elementCount, data);
    }

    template <>
    inline void ISceneDataArrayAccessor::SetDataArray<glm::mat4>(IScene* scene, DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const glm::mat4* data)
    {
        scene->setDataMatrix44fArray(containerHandle, field, elementCount, data);
    }
}

#endif
