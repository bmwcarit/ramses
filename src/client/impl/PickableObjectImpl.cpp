//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/PickableObjectImpl.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/CameraNodeImpl.h"
#include "impl/SerializationContext.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/ErrorReporting.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/Camera.h"
#include "internal/SceneGraph/SceneAPI/PickableObject.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/Scene.h"

namespace ramses::internal
{
    PickableObjectImpl::PickableObjectImpl(SceneImpl& scene, std::string_view pickableObjectName)
        : NodeImpl(scene, ERamsesObjectType::PickableObject, pickableObjectName)
    {
    }

    void PickableObjectImpl::initializeFrameworkData(const ArrayBufferImpl& geometryBuffer, pickableObjectId_t id)
    {
        NodeImpl::initializeFrameworkData();

        assert(!m_pickableObjectHandle.isValid());
        m_geometryBufferImpl = &geometryBuffer;

        const ramses::internal::DataBufferHandle geometryBufferHandle = geometryBuffer.getDataBufferHandle();
        m_pickableObjectHandle = getIScene().allocatePickableObject(geometryBufferHandle, this->getNodeHandle(), ramses::internal::PickableObjectId{ id.getValue() }, {});
    }

    void PickableObjectImpl::deinitializeFrameworkData()
    {
        assert(m_pickableObjectHandle.isValid());
        getIScene().releasePickableObject(m_pickableObjectHandle);
        m_pickableObjectHandle = ramses::internal::PickableObjectHandle::Invalid();

        NodeImpl::deinitializeFrameworkData();
    }

    bool PickableObjectImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!NodeImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_pickableObjectHandle;
        assert(m_geometryBufferImpl != nullptr);

        outStream << serializationContext.getIDForObject(m_geometryBufferImpl);
        if (m_cameraImpl != nullptr)
        {
            outStream << serializationContext.getIDForObject(m_cameraImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }
        return true;
    }

    bool PickableObjectImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!NodeImpl::deserialize(inStream, serializationContext))
            return false;

        serializationContext.deserializeAndMap(inStream, m_pickableObjectHandle);

        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_geometryBufferImpl);
        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_cameraImpl);
        serializationContext.addForDependencyResolve(this);
        return true;
    }

    bool PickableObjectImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!NodeImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_geometryBufferImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_cameraImpl);

        return true;
    }

    void PickableObjectImpl::onValidate(ValidationReportImpl& report) const
    {
        NodeImpl::onValidate(report);

        const ramses::internal::PickableObject& pickableObject = getIScene().getPickableObject(m_pickableObjectHandle);

        if (!getIScene().isDataBufferAllocated(pickableObject.geometryHandle))
        {
            report.add(EIssueType::Error, "pickable object references a deleted geometry buffer", &getRamsesObject());
        }
        else
        {
            report.addDependentObject(*this, *m_geometryBufferImpl);
        }

        if (!pickableObject.cameraHandle.isValid())
        {
            report.add(EIssueType::Warning, "pickable object references no camera, a valid camera must be set", &getRamsesObject());
        }
        else if (!getIScene().isCameraAllocated(pickableObject.cameraHandle))
        {
            report.add(EIssueType::Error, "pickable object references a deleted camera", &getRamsesObject());
        }
    }

    const ArrayBuffer& PickableObjectImpl::getGeometryBuffer() const
    {
        assert(nullptr != m_geometryBufferImpl);
        return RamsesObjectTypeUtils::ConvertTo<ArrayBuffer>(m_geometryBufferImpl->getRamsesObject());
    }

    bool PickableObjectImpl::setCamera(const CameraNodeImpl& cameraImpl)
    {
        if (!isFromTheSameSceneAs(cameraImpl))
        {
            getErrorReporting().set("PickableObject::setCamera failed - camera is not from the same scene as this PickableObject", *this);
            return false;
        }

        ValidationReportImpl cameraReport;
        cameraImpl.validate(cameraReport);
        if (!cameraReport.hasError())
        {
            m_cameraImpl = &cameraImpl;
            getIScene().setPickableObjectCamera(m_pickableObjectHandle, cameraImpl.getCameraHandle());
        }
        else
        {
            getErrorReporting().set(fmt::format("PickableObject::setCamera failed - camera is not valid, maybe camera was not initialized:\n{}", cameraReport.toString()), *this);
            return false;
        }

        return true;
    }

    const ramses::Camera* PickableObjectImpl::getCamera() const
    {
        return (m_cameraImpl ? &RamsesObjectTypeUtils::ConvertTo<ramses::Camera>(m_cameraImpl->getRamsesObject()) : nullptr);
    }

    bool PickableObjectImpl::setPickableObjectId(pickableObjectId_t id)
    {
        getIScene().setPickableObjectId(m_pickableObjectHandle, ramses::internal::PickableObjectId(id.getValue()));
        return true;
    }

    pickableObjectId_t PickableObjectImpl::getPickableObjectId() const
    {
        return pickableObjectId_t(getIScene().getPickableObject(m_pickableObjectHandle).id.getValue());
    }

    bool PickableObjectImpl::setEnabled(bool enable)
    {
        getIScene().setPickableObjectEnabled(m_pickableObjectHandle, enable);
        return true;
    }

    bool PickableObjectImpl::isEnabled() const
    {
        return getIScene().getPickableObject(m_pickableObjectHandle).isEnabled;
    }

    ramses::internal::PickableObjectHandle PickableObjectImpl::getPickableObjectHandle() const
    {
        return m_pickableObjectHandle;
    }
}

