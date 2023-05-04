//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DataObjectImpl.h"
#include "SceneObjectImpl.h"
#include "Scene/ClientScene.h"
#include "SceneUtils/ISceneDataArrayAccessor.h"
#include "SceneAPI/ResourceContentHash.h"
#include "DataTypeUtils.h"

namespace ramses
{
    DataObjectImpl::DataObjectImpl(SceneImpl& scene, ERamsesObjectType ramsesType, EDataType dataType, const char* name)
        : SceneObjectImpl{ scene, ramsesType, name }
        , m_dataType{ dataType }
    {
    }

    DataObjectImpl::~DataObjectImpl() = default;

    void DataObjectImpl::initializeFrameworkData()
    {
        ramses_internal::ClientScene& scene = getIScene();

        // create data layout on scene
        m_layoutHandle = scene.allocateDataLayout({ ramses_internal::DataFieldInfo(DataTypeUtils::ConvertDataTypeToInternal(m_dataType)) }, ramses_internal::ResourceContentHash::Invalid());

        // allocate data instance based on created layout
        m_dataReference = scene.allocateDataInstance(m_layoutHandle);
    }

    void DataObjectImpl::deinitializeFrameworkData()
    {
        ramses_internal::ClientScene& scene = getIScene();

        scene.releaseDataInstance(m_dataReference);
        scene.releaseDataLayout(m_layoutHandle);

        m_dataReference = ramses_internal::DataInstanceHandle::Invalid();
        m_layoutHandle = ramses_internal::DataLayoutHandle::Invalid();
    }

    status_t DataObjectImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << static_cast<uint32_t>(m_dataType);
        outStream << m_layoutHandle;
        outStream << m_dataReference;

        return StatusOK;
    }

    status_t DataObjectImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        uint32_t enumType = 0u;
        inStream >> enumType;
        m_dataType = static_cast<EDataType>(enumType);
        inStream >> m_layoutHandle;
        inStream >> m_dataReference;

        return StatusOK;
    }

    template <typename T>
    status_t DataObjectImpl::setValue(const T& value)
    {
        if (ramses_internal::TypeToEDataTypeTraits<T>::DataType != DataTypeUtils::ConvertDataTypeToInternal(m_dataType))
            return addErrorEntry("DataObject::setValue failed, value type does not match DataObject data type");

        ramses_internal::ISceneDataArrayAccessor::SetDataArray<T>(&getIScene(), m_dataReference, ramses_internal::DataFieldHandle(0u), 1u, &value);
        return StatusOK;
    }

    template <typename T>
    status_t DataObjectImpl::getValue(T& value) const
    {
        if (ramses_internal::TypeToEDataTypeTraits<T>::DataType != DataTypeUtils::ConvertDataTypeToInternal(m_dataType))
            return addErrorEntry("DataObject::getValue failed, value type does not match DataObject data type");

        const T* data = ramses_internal::ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), m_dataReference, ramses_internal::DataFieldHandle(0u));
        assert(data != nullptr);
        value = data[0];

        return StatusOK;
    }

    EDataType DataObjectImpl::getDataType() const
    {
        return m_dataType;
    }

    ramses_internal::DataInstanceHandle DataObjectImpl::getDataReference() const
    {
        return m_dataReference;
    }

    template status_t DataObjectImpl::setValue<int32_t>(const int32_t&);
    template status_t DataObjectImpl::setValue<float>(const float&);
    template status_t DataObjectImpl::setValue<glm::vec2>(const glm::vec2&);
    template status_t DataObjectImpl::setValue<glm::vec3>(const glm::vec3&);
    template status_t DataObjectImpl::setValue<glm::vec4>(const glm::vec4&);
    template status_t DataObjectImpl::setValue<glm::ivec2>(const glm::ivec2&);
    template status_t DataObjectImpl::setValue<glm::ivec3>(const glm::ivec3&);
    template status_t DataObjectImpl::setValue<glm::ivec4>(const glm::ivec4&);
    template status_t DataObjectImpl::setValue<glm::mat2>(const glm::mat2&);
    template status_t DataObjectImpl::setValue<glm::mat3>(const glm::mat3&);
    template status_t DataObjectImpl::setValue<glm::mat4>(const glm::mat4&);

    template status_t DataObjectImpl::getValue<int32_t>(int32_t&) const;
    template status_t DataObjectImpl::getValue<float>(float&) const;
    template status_t DataObjectImpl::getValue<glm::vec2>(glm::vec2&) const;
    template status_t DataObjectImpl::getValue<glm::vec3>(glm::vec3&) const;
    template status_t DataObjectImpl::getValue<glm::vec4>(glm::vec4&) const;
    template status_t DataObjectImpl::getValue<glm::ivec2>(glm::ivec2&) const;
    template status_t DataObjectImpl::getValue<glm::ivec3>(glm::ivec3&) const;
    template status_t DataObjectImpl::getValue<glm::ivec4>(glm::ivec4&) const;
    template status_t DataObjectImpl::getValue<glm::mat2>(glm::mat2&) const;
    template status_t DataObjectImpl::getValue<glm::mat3>(glm::mat3&) const;
    template status_t DataObjectImpl::getValue<glm::mat4>(glm::mat4&) const;
}
