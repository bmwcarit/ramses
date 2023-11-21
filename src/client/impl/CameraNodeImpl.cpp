//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/DataObject.h"
#include "impl/CameraNodeImpl.h"
#include "impl/DataObjectImpl.h"
#include "impl/SerializationContext.h"
#include "impl/ErrorReporting.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/SceneUtils/DataInstanceHelper.h"
#include "internal/Core/Math3d/CameraMatrixHelper.h"
#include <glm/gtc/type_ptr.hpp>

namespace ramses::internal
{
    CameraNodeImpl::CameraNodeImpl(SceneImpl& scene, ERamsesObjectType cameraType, std::string_view cameraName)
        : NodeImpl(scene, cameraType, cameraName)
    {
    }

    CameraNodeImpl::~CameraNodeImpl() = default;

    bool CameraNodeImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!NodeImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_cameraHandle;
        outStream << m_dataLayout;
        outStream << m_dataInstance;
        outStream << m_viewportDataReferenceLayout;
        outStream << m_viewportOffsetDataReference;
        outStream << m_viewportSizeDataReference;
        outStream << m_frustumPlanesDataReferenceLayout;
        outStream << m_frustumPlanesDataReference;
        outStream << m_frustumNearFarDataReferenceLayout;
        outStream << m_frustumNearFarDataReference;
        outStream << m_frustumInitialized;
        outStream << m_viewportInitialized;

        return true;
    }

    bool CameraNodeImpl::deserialize(IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!NodeImpl::deserialize(inStream, serializationContext))
            return false;

        inStream >> m_cameraHandle;
        inStream >> m_dataLayout;
        inStream >> m_dataInstance;
        inStream >> m_viewportDataReferenceLayout;
        inStream >> m_viewportOffsetDataReference;
        inStream >> m_viewportSizeDataReference;
        inStream >> m_frustumPlanesDataReferenceLayout;
        inStream >> m_frustumPlanesDataReference;
        inStream >> m_frustumNearFarDataReferenceLayout;
        inStream >> m_frustumNearFarDataReference;
        inStream >> m_frustumInitialized;
        inStream >> m_viewportInitialized;

        return true;
    }

    void CameraNodeImpl::initializeFrameworkData()
    {
        NodeImpl::initializeFrameworkData();

        // main data instance with all references
        const DataFieldInfoVector dataRefFiels(4u, DataFieldInfo{ ramses::internal::EDataType::DataReference });
        m_dataLayout = getIScene().allocateDataLayout(dataRefFiels, {}, {});
        m_dataInstance = getIScene().allocateDataInstance(m_dataLayout, {});

        // VP offset and size
        m_viewportDataReferenceLayout = getIScene().allocateDataLayout({ DataFieldInfo{ramses::internal::EDataType::Vector2I} }, {}, {});
        m_viewportOffsetDataReference = getIScene().allocateDataInstance(m_viewportDataReferenceLayout, {});
        m_viewportSizeDataReference = getIScene().allocateDataInstance(m_viewportDataReferenceLayout, {});

        // frustum planes
        m_frustumPlanesDataReferenceLayout = getIScene().allocateDataLayout({ DataFieldInfo{ramses::internal::EDataType::Vector4F} }, {}, {});
        m_frustumPlanesDataReference = getIScene().allocateDataInstance(m_frustumPlanesDataReferenceLayout, {});

        // frustum near/far planes
        m_frustumNearFarDataReferenceLayout = getIScene().allocateDataLayout({ DataFieldInfo{ramses::internal::EDataType::Vector2F} }, {}, {});
        m_frustumNearFarDataReference = getIScene().allocateDataInstance(m_frustumNearFarDataReferenceLayout, {});

        // link data references to data instances
        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::ViewportOffsetField, m_viewportOffsetDataReference);
        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::ViewportSizeField, m_viewportSizeDataReference);
        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::FrustumPlanesField, m_frustumPlanesDataReference);
        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::FrustumNearFarPlanesField, m_frustumNearFarDataReference);

        // set default values, even though camera is considered invalid if all these are not set explicitly by user
        getIScene().setDataSingleVector2i(m_viewportOffsetDataReference, DataFieldHandle{ 0 }, { 0, 0 });
        getIScene().setDataSingleVector2i(m_viewportSizeDataReference, DataFieldHandle{ 0 }, { 16, 16 });
        getIScene().setDataSingleVector4f(m_frustumPlanesDataReference, DataFieldHandle{ 0 }, { -1.f, 1.f, -1.f, 1.f });
        getIScene().setDataSingleVector2f(m_frustumNearFarDataReference, DataFieldHandle{ 0 }, { 0.1f, 1.f });

        const auto projType = (getType() == ERamsesObjectType::PerspectiveCamera ? ECameraProjectionType::Perspective : ECameraProjectionType::Orthographic);

        m_cameraHandle = getIScene().allocateCamera(projType, getNodeHandle(), m_dataInstance, CameraHandle::Invalid());
    }

    void CameraNodeImpl::deinitializeFrameworkData()
    {
        getIScene().releaseDataInstance(m_dataInstance);
        m_dataInstance = DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_dataLayout);
        m_dataLayout = DataLayoutHandle::Invalid();
        getIScene().releaseDataInstance(m_viewportOffsetDataReference);
        m_viewportOffsetDataReference = DataInstanceHandle::Invalid();
        getIScene().releaseDataInstance(m_viewportSizeDataReference);
        m_viewportSizeDataReference = DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_viewportDataReferenceLayout);
        m_viewportDataReferenceLayout = DataLayoutHandle::Invalid();
        getIScene().releaseDataInstance(m_frustumPlanesDataReference);
        m_frustumPlanesDataReference = DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_frustumPlanesDataReferenceLayout);
        m_frustumPlanesDataReferenceLayout = DataLayoutHandle::Invalid();
        getIScene().releaseDataInstance(m_frustumNearFarDataReference);
        m_frustumNearFarDataReference = DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_frustumNearFarDataReferenceLayout);
        m_frustumNearFarDataReferenceLayout = DataLayoutHandle::Invalid();
        getIScene().releaseCamera(m_cameraHandle);
        m_cameraHandle = CameraHandle::Invalid();

        NodeImpl::deinitializeFrameworkData();
    }

    void CameraNodeImpl::onValidate(ValidationReportImpl& report) const
    {
        NodeImpl::onValidate(report);

        if (!m_frustumInitialized && !isFrustumPlanesBound())
            report.add(EIssueType::Error, "Camera frustum is not initialized!", &getRamsesObject());

        if (!getProjectionParams().isValid())
            report.add(EIssueType::Error, "Camera frustum invalid!", &getRamsesObject());

        if (!m_viewportInitialized && !(isViewportOffsetBound() && isViewportSizeBound()))
            report.add(EIssueType::Error, "Camera viewport is not initialized!", &getRamsesObject());

        if (getViewportWidth() == 0 || getViewportHeight() == 0)
            report.add(EIssueType::Error, "Camera viewport invalid!", &getRamsesObject());
    }

    ECameraProjectionType CameraNodeImpl::getProjectionType() const
    {
        return getIScene().getCamera(m_cameraHandle).projectionType;
    }

    bool CameraNodeImpl::setPerspectiveFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane)
    {
        assert(isOfType(ERamsesObjectType::PerspectiveCamera));

        const auto params = ProjectionParams::Perspective(fovY, aspectRatio, nearPlane, farPlane);
        if (!params.isValid())
        {
            getErrorReporting().set("PerspectiveCamera::setFrustum failed - check validity of given frustum planes", *this);
            return false;
        }

        updateProjectionParamsOnScene(params);
        m_frustumInitialized = true;

        return true;
    }

    float CameraNodeImpl::getVerticalFieldOfView() const
    {
        assert(isOfType(ERamsesObjectType::PerspectiveCamera));
        return ProjectionParams::GetPerspectiveFovY(getProjectionParams());
    }

    float CameraNodeImpl::getAspectRatio() const
    {
        assert(isOfType(ERamsesObjectType::PerspectiveCamera));
        return ProjectionParams::GetAspectRatio(getProjectionParams());
    }

    bool CameraNodeImpl::setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        const auto params = ProjectionParams::Frustum(
            getIScene().getCamera(m_cameraHandle).projectionType, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);

        if (!params.isValid())
        {
            getErrorReporting().set("Camera::setFrustum failed - check validity of given frustum planes", *this);
            return false;
        }

        updateProjectionParamsOnScene(params);
        m_frustumInitialized = true;

        return true;
    }

    float CameraNodeImpl::getNearPlane() const
    {
        return DataInstanceHelper::GetReferencedData<glm::vec2>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumNearFarPlanesField).x;
    }

    float CameraNodeImpl::getFarPlane() const
    {
        return DataInstanceHelper::GetReferencedData<glm::vec2>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumNearFarPlanesField).y;
    }

    bool CameraNodeImpl::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        // Set a sane upper limit for viewport to avoid GL_INVALID_VALUE in glViewport()
        if (width > 0 && width <= 32768u && height > 0 && height <= 32768u )
        {
            getIScene().setDataSingleVector2i(m_viewportOffsetDataReference, DataFieldHandle{ 0 }, { x, y });
            getIScene().setDataSingleVector2i(m_viewportSizeDataReference, DataFieldHandle{ 0 }, { int32_t(width), int32_t(height) });
            m_viewportInitialized = true;
        }
        else
        {
            getErrorReporting().set("Camera::setViewport failed - width and height must be within [1, 32768]!", *this);
            return false;
        }

        return true;
    }

    int32_t CameraNodeImpl::getViewportX() const
    {
        return DataInstanceHelper::GetReferencedData<glm::ivec2>(getIScene(), m_dataInstance, ramses::internal::Camera::ViewportOffsetField).x;
    }

    int32_t CameraNodeImpl::getViewportY() const
    {
        return DataInstanceHelper::GetReferencedData<glm::ivec2>(getIScene(), m_dataInstance, ramses::internal::Camera::ViewportOffsetField).y;
    }

    uint32_t CameraNodeImpl::getViewportWidth() const
    {
        return DataInstanceHelper::GetReferencedData<glm::ivec2>(getIScene(), m_dataInstance, ramses::internal::Camera::ViewportSizeField).x;
    }

    uint32_t CameraNodeImpl::getViewportHeight() const
    {
        return DataInstanceHelper::GetReferencedData<glm::ivec2>(getIScene(), m_dataInstance, ramses::internal::Camera::ViewportSizeField).y;
    }

    float CameraNodeImpl::getLeftPlane() const
    {
        return DataInstanceHelper::GetReferencedData<glm::vec4>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumPlanesField).x;
    }

    float CameraNodeImpl::getRightPlane() const
    {
        return DataInstanceHelper::GetReferencedData<glm::vec4>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumPlanesField).y;
    }

    float CameraNodeImpl::getBottomPlane() const
    {
        return DataInstanceHelper::GetReferencedData<glm::vec4>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumPlanesField).z;
    }

    float CameraNodeImpl::getTopPlane() const
    {
        return DataInstanceHelper::GetReferencedData<glm::vec4>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumPlanesField).w;
    }

    CameraHandle CameraNodeImpl::getCameraHandle() const
    {
        return m_cameraHandle;
    }

    bool CameraNodeImpl::getProjectionMatrix(matrix44f& projectionMatrix) const
    {
        if (!m_frustumInitialized)
        {
            getErrorReporting().set("CameraImpl::getProjectionMatrix failed - Camera frustum is not initialized!", *this);
            return false;
        }

        projectionMatrix = ramses::internal::CameraMatrixHelper::ProjectionMatrix(getProjectionParams());

        return true;
    }

    ProjectionParams CameraNodeImpl::getProjectionParams() const
    {
        const auto& frustumData = DataInstanceHelper::GetReferencedData<glm::vec4>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumPlanesField);
        const auto& nearFarData = DataInstanceHelper::GetReferencedData<glm::vec2>(getIScene(), m_dataInstance, ramses::internal::Camera::FrustumNearFarPlanesField);

        return ProjectionParams::Frustum(
            getIScene().getCamera(m_cameraHandle).projectionType,
            frustumData.x,
            frustumData.y,
            frustumData.z,
            frustumData.w,
            nearFarData.x,
            nearFarData.y);
    }

    void CameraNodeImpl::updateProjectionParamsOnScene(const ProjectionParams& params)
    {
        getIScene().setDataSingleVector4f(m_frustumPlanesDataReference, DataFieldHandle{ 0 }, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane });
        getIScene().setDataSingleVector2f(m_frustumNearFarDataReference, DataFieldHandle{ 0 }, { params.nearPlane, params.farPlane });
    }

    bool CameraNodeImpl::bindViewportOffset(const DataObject& offsetData)
    {
        if (offsetData.getDataType() != ramses::EDataType::Vector2I)
        {
            getErrorReporting().set("Camera::bindViewportOffset failed, data object must be of type EDataType::Vector2I", *this);
            return false;
        }
        if (!isFromTheSameSceneAs(offsetData.impl()))
        {
            getErrorReporting().set("Camera::bindViewportOffset failed, viewport offset data object is not from the same scene as this camera", *this);
            return false;
        }

        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::ViewportOffsetField, offsetData.impl().getDataReference());
        return true;
    }

    bool CameraNodeImpl::bindViewportSize(const DataObject& sizeData)
    {
        if (sizeData.getDataType() != ramses::EDataType::Vector2I)
        {
            getErrorReporting().set("Camera::bindViewportSize failed, data object must be of type EDataType::Vector2I", *this);
            return false;
        }
        if (!isFromTheSameSceneAs(sizeData.impl()))
        {
            getErrorReporting().set("Camera::bindViewportSize failed, viewport size data object is not from the same scene as this camera", *this);
            return false;
        }

        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::ViewportSizeField, sizeData.impl().getDataReference());
        return true;
    }

    bool CameraNodeImpl::bindFrustumPlanes(const DataObject& frustumPlanesData, const DataObject& nearFarData)
    {
        if (frustumPlanesData.getDataType() != ramses::EDataType::Vector4F || nearFarData.getDataType() != ramses::EDataType::Vector2F)
        {
            getErrorReporting().set("Camera::bindFrustumPlanes failed, data objects must be of type EDataType::Vector4F and EDataType::Vector2F", *this);
            return false;
        }
        if (!isFromTheSameSceneAs(frustumPlanesData.impl()) || !isFromTheSameSceneAs(nearFarData.impl()))
        {
            getErrorReporting().set("Camera::bindFrustumPlanes failed, one of the frustum planes data object is not from the same scene as this camera", *this);
            return false;
        }

        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::FrustumPlanesField, frustumPlanesData.impl().getDataReference());
        getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::FrustumNearFarPlanesField, nearFarData.impl().getDataReference());
        return true;
    }

    bool CameraNodeImpl::unbindViewportOffset()
    {
        if (isViewportOffsetBound())
            getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::ViewportOffsetField, m_viewportOffsetDataReference);
        return true;
    }

    bool CameraNodeImpl::unbindViewportSize()
    {
        if (isViewportSizeBound())
            getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::ViewportSizeField, m_viewportSizeDataReference);
        return true;
    }

    bool CameraNodeImpl::unbindFrustumPlanes()
    {
        if (isFrustumPlanesBound())
        {
            getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::FrustumPlanesField, m_frustumPlanesDataReference);
            getIScene().setDataReference(m_dataInstance, ramses::internal::Camera::FrustumNearFarPlanesField, m_frustumNearFarDataReference);
        }
        return true;
    }

    bool CameraNodeImpl::isViewportOffsetBound() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses::internal::Camera::ViewportOffsetField) != m_viewportOffsetDataReference;
    }

    bool CameraNodeImpl::isViewportSizeBound() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses::internal::Camera::ViewportSizeField) != m_viewportSizeDataReference;
    }

    bool CameraNodeImpl::isFrustumPlanesBound() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses::internal::Camera::FrustumPlanesField) != m_frustumPlanesDataReference;
    }

    DataInstanceHandle CameraNodeImpl::getViewportOffsetHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses::internal::Camera::ViewportOffsetField);
    }

    DataInstanceHandle CameraNodeImpl::getViewportSizeHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses::internal::Camera::ViewportSizeField);
    }

    DataInstanceHandle CameraNodeImpl::getFrustumPlanesHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses::internal::Camera::FrustumPlanesField);
    }

    DataInstanceHandle CameraNodeImpl::getFrustumNearFarPlanesHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses::internal::Camera::FrustumNearFarPlanesField);
    }
}
