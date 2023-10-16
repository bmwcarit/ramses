//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/SceneUtils/DataInstanceHelper.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/SceneGraph/SceneUtils/ISceneDataArrayAccessor.h"
#include "internal/SceneGraph/Scene/DataLayout.h"

namespace ramses::internal
{
    template <typename T>
    void getAndStoreInstanceFieldData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, DataInstanceValueVariant& value)
    {
        const T* data = ISceneDataArrayAccessor::GetDataArray<T>(&scene, dataInstance, dataField);
        value = data[0];
    }

    void DataInstanceHelper::GetInstanceFieldData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, DataInstanceValueVariant& value)
    {
        const ramses::internal::DataLayoutHandle dataLayoutHandle = scene.getLayoutOfDataInstance(dataInstance);
        const ramses::internal::DataLayout& layout = scene.getDataLayout(dataLayoutHandle);
        const ramses::internal::EDataType dataType = layout.getField(dataField).dataType;
        assert(layout.getField(dataField).elementCount == 1u);

        switch (dataType)
        {
        case ramses::internal::EDataType::Float:
            getAndStoreInstanceFieldData<float>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector2F:
            getAndStoreInstanceFieldData<glm::vec2>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector3F:
            getAndStoreInstanceFieldData<glm::vec3>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector4F:
            getAndStoreInstanceFieldData<glm::vec4>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Bool:
            getAndStoreInstanceFieldData<bool>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Int32:
            getAndStoreInstanceFieldData<int32_t>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector2I:
            getAndStoreInstanceFieldData<glm::ivec2>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector3I:
            getAndStoreInstanceFieldData<glm::ivec3>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector4I:
            getAndStoreInstanceFieldData<glm::ivec4>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Matrix22F:
            getAndStoreInstanceFieldData<glm::mat2>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Matrix33F:
            getAndStoreInstanceFieldData<glm::mat3>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Matrix44F:
            getAndStoreInstanceFieldData<glm::mat4>(scene, dataInstance, dataField, value);
            break;
        default:
            assert(false);
            break;
        }
    }

    template <typename T>
    void setInstanceFieldData(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, const DataInstanceValueVariant& value)
    {
        const T typedValue = std::get<T>(value);
        ISceneDataArrayAccessor::SetDataArray<T>(&scene, dataInstance, dataField, 1u, &typedValue);
    }

    void DataInstanceHelper::SetInstanceFieldData(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, const DataInstanceValueVariant& value)
    {
        const ramses::internal::DataLayoutHandle dataLayoutHandle = scene.getLayoutOfDataInstance(dataInstance);
        const ramses::internal::DataLayout& layout = scene.getDataLayout(dataLayoutHandle);
        const ramses::internal::EDataType dataType = layout.getField(dataField).dataType;
        assert(layout.getField(dataField).elementCount == 1u);

        switch (dataType)
        {
        case ramses::internal::EDataType::Float:
            setInstanceFieldData<float>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector2F:
            setInstanceFieldData<glm::vec2>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector3F:
            setInstanceFieldData<glm::vec3>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector4F:
            setInstanceFieldData<glm::vec4>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Bool:
            setInstanceFieldData<bool>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Int32:
            setInstanceFieldData<int32_t>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector2I:
            setInstanceFieldData<glm::ivec2>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector3I:
            setInstanceFieldData<glm::ivec3>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Vector4I:
            setInstanceFieldData<glm::ivec4>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Matrix22F:
            setInstanceFieldData<glm::mat2>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Matrix33F:
            setInstanceFieldData<glm::mat3>(scene, dataInstance, dataField, value);
            break;
        case ramses::internal::EDataType::Matrix44F:
            setInstanceFieldData<glm::mat4>(scene, dataInstance, dataField, value);
            break;
        default:
            assert(false);
            break;
        }
    }
}
