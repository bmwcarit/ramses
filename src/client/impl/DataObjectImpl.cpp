//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/DataObjectImpl.h"
#include "impl/SceneObjectImpl.h"
#include "impl/DataTypeUtils.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/SceneUtils/ISceneDataArrayAccessor.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "impl/SerializationContext.h"

namespace ramses::internal
{
    DataObjectImpl::DataObjectImpl(SceneImpl& scene, ERamsesObjectType ramsesType, ramses::EDataType dataType, std::string_view name)
        : SceneObjectImpl{ scene, ramsesType, name }
        , m_dataType{ dataType }
    {
    }

    DataObjectImpl::~DataObjectImpl() = default;

    void DataObjectImpl::initializeFrameworkData()
    {
        ClientScene& scene = getIScene();

        // create data layout on scene
        m_layoutHandle = scene.allocateDataLayout({DataFieldInfo(DataTypeUtils::ConvertDataTypeToInternal(m_dataType))}, ResourceContentHash::Invalid(), {});

        // allocate data instance based on created layout
        m_dataReference = scene.allocateDataInstance(m_layoutHandle, {});
    }

    void DataObjectImpl::deinitializeFrameworkData()
    {
        ClientScene& scene = getIScene();

        scene.releaseDataInstance(m_dataReference);
        scene.releaseDataLayout(m_layoutHandle);

        m_dataReference = DataInstanceHandle::Invalid();
        m_layoutHandle = DataLayoutHandle::Invalid();
    }

    bool DataObjectImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << static_cast<uint32_t>(m_dataType);
        outStream << m_layoutHandle;
        outStream << m_dataReference;

        return true;
    }

    bool DataObjectImpl::deserialize(IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        uint32_t enumType = 0u;
        inStream >> enumType;
        m_dataType = static_cast<ramses::EDataType>(enumType);
        serializationContext.deserializeAndMap(inStream, m_layoutHandle);
        serializationContext.deserializeAndMap(inStream, m_dataReference);

        return true;
    }

    template <typename T>
    bool DataObjectImpl::setValue(const T& value)
    {
        if (TypeToEDataTypeTraits<T>::DataType != DataTypeUtils::ConvertDataTypeToInternal(m_dataType))
        {
            getErrorReporting().set("DataObject::setValue failed, value type does not match DataObject data type", *this);
            return false;
        }

        ISceneDataArrayAccessor::SetDataArray<T>(&getIScene(), m_dataReference, DataFieldHandle(0u), 1u, &value);
        return true;
    }

    template <typename T>
    bool DataObjectImpl::getValue(T& value) const
    {
        if (TypeToEDataTypeTraits<T>::DataType != DataTypeUtils::ConvertDataTypeToInternal(m_dataType))
        {
            getErrorReporting().set("DataObject::getValue failed, value type does not match DataObject data type", *this);
            return false;
        }

        const T* data = ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), m_dataReference, DataFieldHandle(0u));
        assert(data != nullptr);
        value = data[0];

        return true;
    }

    ramses::EDataType DataObjectImpl::getDataType() const
    {
        return m_dataType;
    }

    DataInstanceHandle DataObjectImpl::getDataReference() const
    {
        return m_dataReference;
    }

    template bool DataObjectImpl::setValue<bool>(const bool&);
    template bool DataObjectImpl::setValue<int32_t>(const int32_t&);
    template bool DataObjectImpl::setValue<float>(const float&);
    template bool DataObjectImpl::setValue<glm::vec2>(const glm::vec2&);
    template bool DataObjectImpl::setValue<glm::vec3>(const glm::vec3&);
    template bool DataObjectImpl::setValue<glm::vec4>(const glm::vec4&);
    template bool DataObjectImpl::setValue<glm::ivec2>(const glm::ivec2&);
    template bool DataObjectImpl::setValue<glm::ivec3>(const glm::ivec3&);
    template bool DataObjectImpl::setValue<glm::ivec4>(const glm::ivec4&);
    template bool DataObjectImpl::setValue<glm::mat2>(const glm::mat2&);
    template bool DataObjectImpl::setValue<glm::mat3>(const glm::mat3&);
    template bool DataObjectImpl::setValue<glm::mat4>(const glm::mat4&);

    template bool DataObjectImpl::getValue<bool>(bool&) const;
    template bool DataObjectImpl::getValue<int32_t>(int32_t&) const;
    template bool DataObjectImpl::getValue<float>(float&) const;
    template bool DataObjectImpl::getValue<glm::vec2>(glm::vec2&) const;
    template bool DataObjectImpl::getValue<glm::vec3>(glm::vec3&) const;
    template bool DataObjectImpl::getValue<glm::vec4>(glm::vec4&) const;
    template bool DataObjectImpl::getValue<glm::ivec2>(glm::ivec2&) const;
    template bool DataObjectImpl::getValue<glm::ivec3>(glm::ivec3&) const;
    template bool DataObjectImpl::getValue<glm::ivec4>(glm::ivec4&) const;
    template bool DataObjectImpl::getValue<glm::mat2>(glm::mat2&) const;
    template bool DataObjectImpl::getValue<glm::mat3>(glm::mat3&) const;
    template bool DataObjectImpl::getValue<glm::mat4>(glm::mat4&) const;
}
