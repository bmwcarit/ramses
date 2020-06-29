//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PickableObjectImpl.h"
#include "VertexDataBufferImpl.h"
#include "CameraNodeImpl.h"
#include "SceneAPI/PickableObject.h"
#include "SceneAPI/SceneTypes.h"
#include "ramses-client-api/LocalCamera.h"
#include "Scene/ClientScene.h"
#include "SerializationContext.h"
#include "RamsesObjectTypeUtils.h"
#include "Scene/Scene.h"
#include "ramses-client-api/VertexDataBuffer.h"

namespace ramses
{

    PickableObjectImpl::PickableObjectImpl(SceneImpl& scene, const char* pickableObjectName)
        : NodeImpl(scene, ERamsesObjectType_PickableObject, pickableObjectName)
    {
    }

    void PickableObjectImpl::initializeFrameworkData(const VertexDataBufferImpl& geometryBuffer, pickableObjectId_t id)
    {
        NodeImpl::initializeFrameworkData();

        assert(!m_pickableObjectHandle.isValid());
        m_geometryBufferImpl = &geometryBuffer;

        const ramses_internal::DataBufferHandle geometryBufferHandle = geometryBuffer.getDataBufferHandle();
        m_pickableObjectHandle = getIScene().allocatePickableObject(geometryBufferHandle, this->getNodeHandle(), ramses_internal::PickableObjectId{ id.getValue() });
    }

    void PickableObjectImpl::deinitializeFrameworkData()
    {
        assert(m_pickableObjectHandle.isValid());
        getIScene().releasePickableObject(m_pickableObjectHandle);
        m_pickableObjectHandle = ramses_internal::PickableObjectHandle::Invalid();

        NodeImpl::deinitializeFrameworkData();
    }

    status_t PickableObjectImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(NodeImpl::serialize(outStream, serializationContext));

        outStream << m_pickableObjectHandle;
        assert(m_geometryBufferImpl != nullptr);

        outStream << serializationContext.getIDForObject(m_geometryBufferImpl);
        if (m_cameraImpl != nullptr)
            outStream << serializationContext.getIDForObject(m_cameraImpl);
        else
            outStream << SerializationContext::GetObjectIDNull();
        return StatusOK;
    }

    status_t PickableObjectImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(NodeImpl::deserialize(inStream, serializationContext));

        inStream >> m_pickableObjectHandle;

        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_geometryBufferImpl);
        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_cameraImpl);
        serializationContext.addForDependencyResolve(this);
        return StatusOK;
    }

    status_t PickableObjectImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(NodeImpl::resolveDeserializationDependencies(serializationContext));

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_geometryBufferImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_cameraImpl);

        return StatusOK;
    }

    status_t PickableObjectImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = NodeImpl::validate(indent, visitedObjects);
        indent += IndentationStep;

        const ramses_internal::PickableObject& pickableObject = getIScene().getPickableObject(m_pickableObjectHandle);

        if (!getIScene().isDataBufferAllocated(pickableObject.geometryHandle))
        {
            addValidationMessage(EValidationSeverity_Error, indent, "pickable object references a deleted geometry buffer");
            status = getValidationErrorStatus();
        }
        else if (addValidationOfDependentObject(indent, *m_geometryBufferImpl, visitedObjects) != StatusOK)
            status = getValidationErrorStatus();

        if (!pickableObject.cameraHandle.isValid())
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "pickable object references no camera, a valid camera must be set");
            status = getValidationErrorStatus();
        }
        else if (!getIScene().isCameraAllocated(pickableObject.cameraHandle))
        {
            addValidationMessage(EValidationSeverity_Error, indent, "pickable object references a deleted camera");
            status = getValidationErrorStatus();
        }

        return status;
    }

    const VertexDataBuffer& PickableObjectImpl::getGeometryBuffer() const
    {
        assert(nullptr != m_geometryBufferImpl);
        return RamsesObjectTypeUtils::ConvertTo<VertexDataBuffer>(m_geometryBufferImpl->getRamsesObject());
    }

    status_t PickableObjectImpl::setCamera(const CameraNodeImpl& cameraImpl)
    {
        assert(cameraImpl.isOfType(ERamsesObjectType_LocalCamera));
        if (!isFromTheSameSceneAs(cameraImpl))
        {
            return addErrorEntry("PickableObject::setCamera failed - camera is not from the same scene as this PickableObject");
        }

        StatusObjectSet visitedObjects;
        const status_t cameraValidity = cameraImpl.validate(0u, visitedObjects);
        if (StatusOK == cameraValidity)
        {
            m_cameraImpl = &cameraImpl;
            getIScene().setPickableObjectCamera(m_pickableObjectHandle, cameraImpl.getCameraHandle());
        }
        else
        {
            ramses_internal::String str =
                "PickableObject::setCamera failed - camera is not valid, maybe camera was not initialized:\n";
            str += cameraImpl.getValidationReport(EValidationSeverity_Warning);
            return addErrorEntry(str.c_str());
        }

        return cameraValidity;
    }

    const LocalCamera* PickableObjectImpl::getCamera() const
    {
        if (nullptr != m_cameraImpl)
        {
            return &RamsesObjectTypeUtils::ConvertTo<LocalCamera>(m_cameraImpl->getRamsesObject());
        }

        return nullptr;
    }

    status_t PickableObjectImpl::setPickableObjectId(pickableObjectId_t id)
    {
        getIScene().setPickableObjectId(m_pickableObjectHandle, ramses_internal::PickableObjectId(id.getValue()));
        return StatusOK;
    }

    pickableObjectId_t PickableObjectImpl::getPickableObjectId() const
    {
        return pickableObjectId_t(getIScene().getPickableObject(m_pickableObjectHandle).id.getValue());
    }

    status_t PickableObjectImpl::setEnabled(bool enable)
    {
        getIScene().setPickableObjectEnabled(m_pickableObjectHandle, enable);
        return StatusOK;
    }

    bool PickableObjectImpl::isEnabled() const
    {
        return getIScene().getPickableObject(m_pickableObjectHandle).isEnabled;
    }

    ramses_internal::PickableObjectHandle PickableObjectImpl::getPickableObjectHandle() const
    {
        return m_pickableObjectHandle;
    }
}

