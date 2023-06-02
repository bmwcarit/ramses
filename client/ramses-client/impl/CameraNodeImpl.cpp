//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataObject.h"
#include "CameraNodeImpl.h"
#include "DataObjectImpl.h"
#include "SerializationContext.h"
#include "Scene/ClientScene.h"
#include "Math3d/CameraMatrixHelper.h"
#include <glm/gtc/type_ptr.hpp>

namespace ramses
{
    CameraNodeImpl::CameraNodeImpl(SceneImpl& scene, ERamsesObjectType cameraType, std::string_view cameraName)
        : NodeImpl(scene, cameraType, cameraName)
    {
    }

    CameraNodeImpl::~CameraNodeImpl()
    {
    }

    status_t CameraNodeImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(NodeImpl::serialize(outStream, serializationContext));

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

        return StatusOK;
    }

    status_t CameraNodeImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(NodeImpl::deserialize(inStream, serializationContext));

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

        return StatusOK;
    }

    void CameraNodeImpl::initializeFrameworkData()
    {
        NodeImpl::initializeFrameworkData();

        // main data instance with all references
        const ramses_internal::DataFieldInfoVector dataRefFiels(4u, ramses_internal::DataFieldInfo{ ramses_internal::EDataType::DataReference });
        m_dataLayout = getIScene().allocateDataLayout(dataRefFiels, {});
        m_dataInstance = getIScene().allocateDataInstance(m_dataLayout);

        // VP offset and size
        m_viewportDataReferenceLayout = getIScene().allocateDataLayout({ ramses_internal::DataFieldInfo{ramses_internal::EDataType::Vector2I} }, {});
        m_viewportOffsetDataReference = getIScene().allocateDataInstance(m_viewportDataReferenceLayout);
        m_viewportSizeDataReference = getIScene().allocateDataInstance(m_viewportDataReferenceLayout);

        // frustum planes
        m_frustumPlanesDataReferenceLayout = getIScene().allocateDataLayout({ ramses_internal::DataFieldInfo{ramses_internal::EDataType::Vector4F} }, {});
        m_frustumPlanesDataReference = getIScene().allocateDataInstance(m_frustumPlanesDataReferenceLayout);

        // frustum near/far planes
        m_frustumNearFarDataReferenceLayout = getIScene().allocateDataLayout({ ramses_internal::DataFieldInfo{ramses_internal::EDataType::Vector2F} }, {});
        m_frustumNearFarDataReference = getIScene().allocateDataInstance(m_frustumNearFarDataReferenceLayout);

        // link data references to data instances
        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::ViewportOffsetField, m_viewportOffsetDataReference);
        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::ViewportSizeField, m_viewportSizeDataReference);
        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField, m_frustumPlanesDataReference);
        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::FrustumNearFarPlanesField, m_frustumNearFarDataReference);

        // set default values, even though camera is considered invalid if all these are not set explicitly by user
        getIScene().setDataSingleVector2i(m_viewportOffsetDataReference, ramses_internal::DataFieldHandle{ 0 }, { 0, 0 });
        getIScene().setDataSingleVector2i(m_viewportSizeDataReference, ramses_internal::DataFieldHandle{ 0 }, { 16, 16 });
        getIScene().setDataSingleVector4f(m_frustumPlanesDataReference, ramses_internal::DataFieldHandle{ 0 }, { -1.f, 1.f, -1.f, 1.f });
        getIScene().setDataSingleVector2f(m_frustumNearFarDataReference, ramses_internal::DataFieldHandle{ 0 }, { 0.1f, 1.f });

        const auto projType = (getType() == ERamsesObjectType::PerspectiveCamera ? ramses_internal::ECameraProjectionType::Perspective : ramses_internal::ECameraProjectionType::Orthographic);

        m_cameraHandle = getIScene().allocateCamera(projType, getNodeHandle(), m_dataInstance, ramses_internal::CameraHandle::Invalid());
    }

    void CameraNodeImpl::deinitializeFrameworkData()
    {
        getIScene().releaseDataInstance(m_dataInstance);
        m_dataInstance = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_dataLayout);
        m_dataLayout = ramses_internal::DataLayoutHandle::Invalid();
        getIScene().releaseDataInstance(m_viewportOffsetDataReference);
        m_viewportOffsetDataReference = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataInstance(m_viewportSizeDataReference);
        m_viewportSizeDataReference = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_viewportDataReferenceLayout);
        m_viewportDataReferenceLayout = ramses_internal::DataLayoutHandle::Invalid();
        getIScene().releaseDataInstance(m_frustumPlanesDataReference);
        m_frustumPlanesDataReference = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_frustumPlanesDataReferenceLayout);
        m_frustumPlanesDataReferenceLayout = ramses_internal::DataLayoutHandle::Invalid();
        getIScene().releaseDataInstance(m_frustumNearFarDataReference);
        m_frustumNearFarDataReference = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_frustumNearFarDataReferenceLayout);
        m_frustumNearFarDataReferenceLayout = ramses_internal::DataLayoutHandle::Invalid();
        getIScene().releaseCamera(m_cameraHandle);
        m_cameraHandle = ramses_internal::CameraHandle::Invalid();

        NodeImpl::deinitializeFrameworkData();
    }

    status_t CameraNodeImpl::validate() const
    {
        status_t status = NodeImpl::validate();

        if (!m_frustumInitialized && !isFrustumPlanesBound())
            status = addValidationMessage(EValidationSeverity::Error, "Camera frustum is not initialized!");

        if (!getProjectionParams().isValid())
            status = addValidationMessage(EValidationSeverity::Error, "Camera frustum invalid!");

        if (!m_viewportInitialized && !(isViewportOffsetBound() && isViewportSizeBound()))
            status = addValidationMessage(EValidationSeverity::Error, "Camera viewport is not initialized!");

        if (getViewportWidth() == 0 || getViewportHeight() == 0)
            status = addValidationMessage(EValidationSeverity::Error, "Camera viewport invalid!");

        return status;
    }

    ramses_internal::ECameraProjectionType CameraNodeImpl::getProjectionType() const
    {
        return getIScene().getCamera(m_cameraHandle).projectionType;
    }

    status_t CameraNodeImpl::setPerspectiveFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane)
    {
        assert(isOfType(ERamsesObjectType::PerspectiveCamera));

        const auto params = ramses_internal::ProjectionParams::Perspective(fovY, aspectRatio, nearPlane, farPlane);
        if (!params.isValid())
            return addErrorEntry("PerspectiveCamera::setFrustum failed - check validity of given frustum planes");

        updateProjectionParamsOnScene(params);
        m_frustumInitialized = true;

        return StatusOK;
    }

    float CameraNodeImpl::getVerticalFieldOfView() const
    {
        assert(isOfType(ERamsesObjectType::PerspectiveCamera));
        return ramses_internal::ProjectionParams::GetPerspectiveFovY(getProjectionParams());
    }

    float CameraNodeImpl::getAspectRatio() const
    {
        assert(isOfType(ERamsesObjectType::PerspectiveCamera));
        return ramses_internal::ProjectionParams::GetAspectRatio(getProjectionParams());
    }

    status_t CameraNodeImpl::setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        const auto params = ramses_internal::ProjectionParams::Frustum(
            getIScene().getCamera(m_cameraHandle).projectionType, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);

        if (!params.isValid())
            return addErrorEntry("Camera::setFrustum failed - check validity of given frustum planes");

        updateProjectionParamsOnScene(params);
        m_frustumInitialized = true;

        return StatusOK;
    }

    float CameraNodeImpl::getNearPlane() const
    {
        const auto nearFarData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumNearFarPlanesField);
        return getIScene().getDataSingleVector2f(nearFarData, ramses_internal::DataFieldHandle{ 0 }).x;
    }

    float CameraNodeImpl::getFarPlane() const
    {
        const auto nearFarData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumNearFarPlanesField);
        return getIScene().getDataSingleVector2f(nearFarData, ramses_internal::DataFieldHandle{ 0 }).y;
    }

    status_t CameraNodeImpl::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        // Set a sane upper limit for viewport to avoid GL_INVALID_VALUE in glViewport()
        if (width > 0 && width <= 32768u && height > 0 && height <= 32768u )
        {
            getIScene().setDataSingleVector2i(m_viewportOffsetDataReference, ramses_internal::DataFieldHandle{ 0 }, { x, y });
            getIScene().setDataSingleVector2i(m_viewportSizeDataReference, ramses_internal::DataFieldHandle{ 0 }, { int32_t(width), int32_t(height) });
            m_viewportInitialized = true;
        }
        else
        {
            return addErrorEntry("Camera::setViewport failed - width and height must be within [1, 32768]!");
        }

        return StatusOK;
    }

    int32_t CameraNodeImpl::getViewportX() const
    {
        const auto vpOffsetData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportOffsetField);
        return getIScene().getDataSingleVector2i(vpOffsetData, ramses_internal::DataFieldHandle{ 0 }).x;
    }

    int32_t CameraNodeImpl::getViewportY() const
    {
        const auto vpOffsetData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportOffsetField);
        return getIScene().getDataSingleVector2i(vpOffsetData, ramses_internal::DataFieldHandle{ 0 }).y;
    }

    uint32_t CameraNodeImpl::getViewportWidth() const
    {
        const auto vpSizeData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportSizeField);
        return getIScene().getDataSingleVector2i(vpSizeData, ramses_internal::DataFieldHandle{ 0 }).x;
    }

    uint32_t CameraNodeImpl::getViewportHeight() const
    {
        const auto vpSizeData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportSizeField);
        return getIScene().getDataSingleVector2i(vpSizeData, ramses_internal::DataFieldHandle{ 0 }).y;
    }

    float CameraNodeImpl::getLeftPlane() const
    {
        const auto frustumData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField);
        return getIScene().getDataSingleVector4f(frustumData, ramses_internal::DataFieldHandle{ 0 }).x;
    }

    float CameraNodeImpl::getRightPlane() const
    {
        const auto frustumData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField);
        return getIScene().getDataSingleVector4f(frustumData, ramses_internal::DataFieldHandle{ 0 }).y;
    }

    float CameraNodeImpl::getBottomPlane() const
    {
        const auto frustumData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField);
        return getIScene().getDataSingleVector4f(frustumData, ramses_internal::DataFieldHandle{ 0 }).z;
    }

    float CameraNodeImpl::getTopPlane() const
    {
        const auto frustumData = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField);
        return getIScene().getDataSingleVector4f(frustumData, ramses_internal::DataFieldHandle{ 0 }).w;
    }

    ramses_internal::CameraHandle CameraNodeImpl::getCameraHandle() const
    {
        return m_cameraHandle;
    }

    status_t CameraNodeImpl::getProjectionMatrix(matrix44f& projectionMatrix) const
    {
        if (!m_frustumInitialized)
        {
            return addErrorEntry("CameraImpl::getProjectionMatrix failed - Camera frustum is not initialized!");
        }

        projectionMatrix = ramses_internal::CameraMatrixHelper::ProjectionMatrix(getProjectionParams());

        return StatusOK;
    }

    ramses_internal::ProjectionParams CameraNodeImpl::getProjectionParams() const
    {
        const auto frustumDataInstance = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField);
        const auto& frustumData = getIScene().getDataSingleVector4f(frustumDataInstance, ramses_internal::DataFieldHandle{ 0 });
        const auto nearFarDataInstance = getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumNearFarPlanesField);
        const auto& nearFarData = getIScene().getDataSingleVector2f(nearFarDataInstance, ramses_internal::DataFieldHandle{ 0 });

        return ramses_internal::ProjectionParams::Frustum(
            getIScene().getCamera(m_cameraHandle).projectionType,
            frustumData.x,
            frustumData.y,
            frustumData.z,
            frustumData.w,
            nearFarData.x,
            nearFarData.y);
    }

    void CameraNodeImpl::updateProjectionParamsOnScene(const ramses_internal::ProjectionParams& params)
    {
        getIScene().setDataSingleVector4f(m_frustumPlanesDataReference, ramses_internal::DataFieldHandle{ 0 }, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane });
        getIScene().setDataSingleVector2f(m_frustumNearFarDataReference, ramses_internal::DataFieldHandle{ 0 }, { params.nearPlane, params.farPlane });
    }

    status_t CameraNodeImpl::bindViewportOffset(const DataObject& offsetData)
    {
        if (offsetData.getDataType() != EDataType::Vector2I)
            return addErrorEntry("Camera::bindViewportOffset failed, data object must be of type EDataType::Vector2I");
        if (!isFromTheSameSceneAs(offsetData.m_impl))
            return addErrorEntry("Camera::bindViewportOffset failed, viewport offset data object is not from the same scene as this camera");

        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::ViewportOffsetField, offsetData.m_impl.getDataReference());
        return StatusOK;
    }

    status_t CameraNodeImpl::bindViewportSize(const DataObject& sizeData)
    {
        if (sizeData.getDataType() != EDataType::Vector2I)
            return addErrorEntry("Camera::bindViewportSize failed, data object must be of type EDataType::Vector2I");
        if (!isFromTheSameSceneAs(sizeData.m_impl))
            return addErrorEntry("Camera::bindViewportSize failed, viewport size data object is not from the same scene as this camera");

        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::ViewportSizeField, sizeData.m_impl.getDataReference());
        return StatusOK;
    }

    status_t CameraNodeImpl::bindFrustumPlanes(const DataObject& frustumPlanesData, const DataObject& nearFarData)
    {
        if (frustumPlanesData.getDataType() != EDataType::Vector4F || nearFarData.getDataType() != EDataType::Vector2F)
            return addErrorEntry("Camera::bindFrustumPlanes failed, data objects must be of type EDataType::Vector4F and EDataType::Vector2F");
        if (!isFromTheSameSceneAs(frustumPlanesData.m_impl) || !isFromTheSameSceneAs(nearFarData.m_impl))
            return addErrorEntry("Camera::bindFrustumPlanes failed, one of the frustum planes data object is not from the same scene as this camera");

        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField, frustumPlanesData.m_impl.getDataReference());
        getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::FrustumNearFarPlanesField, nearFarData.m_impl.getDataReference());
        return StatusOK;
    }

    status_t CameraNodeImpl::unbindViewportOffset()
    {
        if (isViewportOffsetBound())
            getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::ViewportOffsetField, m_viewportOffsetDataReference);
        return StatusOK;
    }

    status_t CameraNodeImpl::unbindViewportSize()
    {
        if (isViewportSizeBound())
            getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::ViewportSizeField, m_viewportSizeDataReference);
        return StatusOK;
    }

    status_t CameraNodeImpl::unbindFrustumPlanes()
    {
        if (isFrustumPlanesBound())
        {
            getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField, m_frustumPlanesDataReference);
            getIScene().setDataReference(m_dataInstance, ramses_internal::Camera::FrustumNearFarPlanesField, m_frustumNearFarDataReference);
        }
        return StatusOK;
    }

    bool CameraNodeImpl::isViewportOffsetBound() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportOffsetField) != m_viewportOffsetDataReference;
    }

    bool CameraNodeImpl::isViewportSizeBound() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportSizeField) != m_viewportSizeDataReference;
    }

    bool CameraNodeImpl::isFrustumPlanesBound() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField) != m_frustumPlanesDataReference;
    }

    ramses_internal::DataInstanceHandle CameraNodeImpl::getViewportOffsetHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportOffsetField);
    }

    ramses_internal::DataInstanceHandle CameraNodeImpl::getViewportSizeHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::ViewportSizeField);
    }

    ramses_internal::DataInstanceHandle CameraNodeImpl::getFrustrumPlanesHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumPlanesField);
    }

    ramses_internal::DataInstanceHandle CameraNodeImpl::getFrustrumNearFarPlanesHandle() const
    {
        return getIScene().getDataReference(m_dataInstance, ramses_internal::Camera::FrustumNearFarPlanesField);
    }
}
