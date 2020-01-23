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
#include "Math3d/Vector4.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"
#include "SceneAPI/ResourceContentHash.h"

namespace ramses
{
    DataObjectImpl::DataObjectImpl(SceneImpl& scene, ERamsesObjectType type, const char* name)
        : SceneObjectImpl(scene, type, name)
        , m_dataType(GetDataTypeForDataObjectType(type))
    {
    }

    DataObjectImpl::~DataObjectImpl()
    {
        //done on deinitialization
    }

    void DataObjectImpl::initializeFrameworkData()
    {
        ramses_internal::ClientScene& scene = getIScene();

        // create data layout on scene
        m_layoutHandle = scene.allocateDataLayout({ ramses_internal::DataFieldInfo(m_dataType) }, ramses_internal::ResourceContentHash::Invalid());

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
        m_dataType = static_cast<ramses_internal::EDataType>(enumType);
        inStream >> m_layoutHandle;
        inStream >> m_dataReference;

        return StatusOK;
    }

    ramses_internal::EDataType DataObjectImpl::GetDataTypeForDataObjectType(ERamsesObjectType type)
    {
        switch (type)
        {
        case ERamsesObjectType_DataFloat:
            return ramses_internal::EDataType_Float;
        case ERamsesObjectType_DataVector2f:
            return ramses_internal::EDataType_Vector2F;
        case ERamsesObjectType_DataVector3f:
            return ramses_internal::EDataType_Vector3F;
        case ERamsesObjectType_DataVector4f:
            return ramses_internal::EDataType_Vector4F;
        case ERamsesObjectType_DataMatrix22f:
            return ramses_internal::EDataType_Matrix22F;
        case ERamsesObjectType_DataMatrix33f:
            return ramses_internal::EDataType_Matrix33F;
        case ERamsesObjectType_DataMatrix44f:
            return ramses_internal::EDataType_Matrix44F;
        case ERamsesObjectType_DataInt32:
            return ramses_internal::EDataType_Int32;
        case ERamsesObjectType_DataVector2i:
            return ramses_internal::EDataType_Vector2I;
        case ERamsesObjectType_DataVector3i:
            return ramses_internal::EDataType_Vector3I;
        case ERamsesObjectType_DataVector4i:
            return ramses_internal::EDataType_Vector4I;
        default:
            assert(false);
            return ramses_internal::EDataType_Invalid;
        }
    }

    template <typename T>
    status_t DataObjectImpl::setValue(const T& value)
    {
        assert(ramses_internal::TypeToEDataTypeTraits<T>::DataType == m_dataType);
        ramses_internal::ISceneDataArrayAccessor::SetDataArray<T>(&getIScene(), m_dataReference, ramses_internal::DataFieldHandle(0u), 1u, &value);
        return StatusOK;
    }

    template <typename T>
    status_t DataObjectImpl::getValue(T& value) const
    {
        assert(ramses_internal::TypeToEDataTypeTraits<T>::DataType == m_dataType);
        const T* data = ramses_internal::ISceneDataArrayAccessor::GetDataArray<T>(&getIScene(), m_dataReference, ramses_internal::DataFieldHandle(0u));
        assert(data != nullptr);
        value = data[0];

        return StatusOK;
    }

    ramses_internal::EDataType DataObjectImpl::getDataType() const
    {
        return m_dataType;
    }

    ramses_internal::DataInstanceHandle DataObjectImpl::getDataReference() const
    {
        return m_dataReference;
    }

    template status_t DataObjectImpl::setValue<int32_t>(const int32_t&);
    template status_t DataObjectImpl::setValue<float>(const float&);
    template status_t DataObjectImpl::setValue<ramses_internal::Vector2>(const ramses_internal::Vector2&);
    template status_t DataObjectImpl::setValue<ramses_internal::Vector3>(const ramses_internal::Vector3&);
    template status_t DataObjectImpl::setValue<ramses_internal::Vector4>(const ramses_internal::Vector4&);
    template status_t DataObjectImpl::setValue<ramses_internal::Vector2i>(const ramses_internal::Vector2i&);
    template status_t DataObjectImpl::setValue<ramses_internal::Vector3i>(const ramses_internal::Vector3i&);
    template status_t DataObjectImpl::setValue<ramses_internal::Vector4i>(const ramses_internal::Vector4i&);
    template status_t DataObjectImpl::setValue<ramses_internal::Matrix22f>(const ramses_internal::Matrix22f&);
    template status_t DataObjectImpl::setValue<ramses_internal::Matrix33f>(const ramses_internal::Matrix33f&);
    template status_t DataObjectImpl::setValue<ramses_internal::Matrix44f>(const ramses_internal::Matrix44f&);

    template status_t DataObjectImpl::getValue<int32_t>(int32_t&) const;
    template status_t DataObjectImpl::getValue<float>(float&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Vector2>(ramses_internal::Vector2&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Vector3>(ramses_internal::Vector3&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Vector4>(ramses_internal::Vector4&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Vector2i>(ramses_internal::Vector2i&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Vector3i>(ramses_internal::Vector3i&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Vector4i>(ramses_internal::Vector4i&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Matrix22f>(ramses_internal::Matrix22f&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Matrix33f>(ramses_internal::Matrix33f&) const;
    template status_t DataObjectImpl::getValue<ramses_internal::Matrix44f>(ramses_internal::Matrix44f&) const;
}
