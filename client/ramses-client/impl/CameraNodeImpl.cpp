//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/DataVector2i.h"
#include "CameraNodeImpl.h"
#include "DataObjectImpl.h"
#include "SerializationContext.h"
#include "Scene/ClientScene.h"
#include "Math3d/CameraMatrixHelper.h"
#include "Math3d/Vector2i.h"

namespace ramses
{
    CameraNodeImpl::CameraNodeImpl(SceneImpl& scene, const char* cameraName, ERamsesObjectType cameraType)
        : NodeImpl(scene, cameraType, cameraName)
        , m_frustumInitialized(isOfType(ERamsesObjectType_RemoteCamera))
        , m_viewportInitialized(isOfType(ERamsesObjectType_RemoteCamera))
    {
    }

    CameraNodeImpl::~CameraNodeImpl()
    {
    }

    status_t CameraNodeImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(NodeImpl::serialize(outStream, serializationContext));

        outStream << m_cameraHandle;
        outStream << m_viewportDataLayout;
        outStream << m_viewportDataInstance;
        outStream << m_viewportOffsetDataReference;
        outStream << m_viewportSizeDataReference;

        return StatusOK;
    }

    status_t CameraNodeImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(NodeImpl::deserialize(inStream, serializationContext));

        inStream >> m_cameraHandle;
        inStream >> m_viewportDataLayout;
        inStream >> m_viewportDataInstance;
        inStream >> m_viewportOffsetDataReference;
        inStream >> m_viewportSizeDataReference;

        return StatusOK;
    }

    void CameraNodeImpl::initializeFrameworkData()
    {
        NodeImpl::initializeFrameworkData();

        ramses_internal::ECameraProjectionType projType = ramses_internal::ECameraProjectionType_Renderer;
        if (ERamsesObjectType_PerspectiveCamera == getType())
        {
            projType = ramses_internal::ECameraProjectionType_Perspective;
        }
        else if (ERamsesObjectType_OrthographicCamera == getType())
        {
            projType = ramses_internal::ECameraProjectionType_Orthographic;
        }

        m_viewportDataLayout = getIScene().allocateDataLayout({ {ramses_internal::EDataType_DataReference}, {ramses_internal::EDataType_DataReference} });
        m_viewportDataInstance = getIScene().allocateDataInstance(m_viewportDataLayout);
        m_viewportDataReferenceLayout = getIScene().allocateDataLayout({ {ramses_internal::EDataType_Vector2I} });
        m_viewportOffsetDataReference = getIScene().allocateDataInstance(m_viewportDataReferenceLayout);
        m_viewportSizeDataReference = getIScene().allocateDataInstance(m_viewportDataReferenceLayout);
        m_cameraHandle = getIScene().allocateCamera(projType, getNodeHandle(), m_viewportDataInstance, ramses_internal::CameraHandle::Invalid());

        getIScene().setDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportOffsetField, m_viewportOffsetDataReference);
        getIScene().setDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportSizeField, m_viewportSizeDataReference);
    }

    void CameraNodeImpl::deinitializeFrameworkData()
    {
        getIScene().releaseDataInstance(m_viewportDataInstance);
        m_viewportDataInstance = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_viewportDataLayout);
        m_viewportDataLayout = ramses_internal::DataLayoutHandle::Invalid();
        getIScene().releaseDataInstance(m_viewportOffsetDataReference);
        m_viewportOffsetDataReference = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataInstance(m_viewportSizeDataReference);
        m_viewportSizeDataReference = ramses_internal::DataInstanceHandle::Invalid();
        getIScene().releaseDataLayout(m_viewportDataReferenceLayout);
        m_viewportDataReferenceLayout = ramses_internal::DataLayoutHandle::Invalid();
        getIScene().releaseCamera(m_cameraHandle);
        m_cameraHandle = ramses_internal::CameraHandle::Invalid();

        NodeImpl::deinitializeFrameworkData();
    }

    status_t CameraNodeImpl::validate(uint32_t indent) const
    {
        status_t status = NodeImpl::validate(indent);
        indent += IndentationStep;

        // camera plane parameters are always valid,
        // camera itself maintains invalid state until some parameters are set
        if (!m_frustumInitialized)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "Camera frustum is not initialized!");
            status = getValidationErrorStatus();
        }

        const bool hasViewportData = m_viewportInitialized || (isViewportOffsetBound() && isViewportSizeBound());
        const bool viewportDataValid = m_viewportInitialized || (getViewportWidth() > 0 && getViewportHeight() > 0);
        if (!hasViewportData || !viewportDataValid)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "Camera viewport is not initialized or set to invalid values!");
            status = getValidationErrorStatus();
        }

        return status;
    }

    ramses_internal::ECameraProjectionType CameraNodeImpl::getProjectionType() const
    {
        return getIScene().getCamera(m_cameraHandle).projectionType;
    }

    status_t CameraNodeImpl::setPerspectiveFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane)
    {
        assert(isOfType(ERamsesObjectType_PerspectiveCamera));

        // Check inputs for validity
        if (fovY <= 0.0f || fovY >= 180.f)
        {
            return addErrorEntry("Camera::setPerspectiveFrustum failed - Vertical field of view must be between 0 and 180 degrees!");
        }
        if (aspectRatio <= 0.0f)
        {
            return addErrorEntry("Camera::setPerspectiveFrustum failed - Aspect Ratio must be a value greater than 0!");
        }
        if (nearPlane <= 0.0f || nearPlane >= farPlane)
        {
            return addErrorEntry("Camera::setPerspectiveFrustum failed - Near plane must be greater than 0 and less than far plane!");
        }

        const ramses_internal::ProjectionParams params =
            ramses_internal::ProjectionParams::Perspective(fovY, aspectRatio, nearPlane, farPlane);

        updateProjectionParamsOnScene(params);
        m_frustumInitialized = true;

        return StatusOK;
    }

    float CameraNodeImpl::getVerticalFieldOfView() const
    {
        assert(isOfType(ERamsesObjectType_PerspectiveCamera));
        return ramses_internal::ProjectionParams::GetPerspectiveFovY(getProjectionParams());
    }

    float CameraNodeImpl::getAspectRatio() const
    {
        assert(isOfType(ERamsesObjectType_PerspectiveCamera));
        return ramses_internal::ProjectionParams::GetAspectRatio(getProjectionParams());
    }

    status_t CameraNodeImpl::setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        if (leftPlane >= rightPlane)
        {
            return addErrorEntry("Camera::setFrustum failed - Left plane must be less than right plane!");
        }

        if (bottomPlane >= topPlane)
        {
            return addErrorEntry("Camera::setFrustum failed - Bottom plane must be less than top plane!");
        }
        if(nearPlane <= 0.0f || nearPlane >= farPlane)
        {
            return addErrorEntry("Camera::setFrustum failed - Near plane must be greater than 0 and less than far plane!");
        }

        ramses_internal::ProjectionParams params = ramses_internal::ProjectionParams::Frustum(
            getIScene().getCamera(m_cameraHandle).projectionType, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);

        updateProjectionParamsOnScene(params);
        m_frustumInitialized = true;

        return StatusOK;
    }

    float CameraNodeImpl::getNearPlane() const
    {
        return getIScene().getCamera(m_cameraHandle).frustum.nearPlane;
    }

    float CameraNodeImpl::getFarPlane() const
    {
        return getIScene().getCamera(m_cameraHandle).frustum.farPlane;
    }

    status_t CameraNodeImpl::setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));

        if (width > 0 && height > 0)
        {
            getIScene().setDataSingleVector2i(m_viewportOffsetDataReference, ramses_internal::DataFieldHandle{ 0 }, { x, y });
            getIScene().setDataSingleVector2i(m_viewportSizeDataReference, ramses_internal::DataFieldHandle{ 0 }, { int32_t(width), int32_t(height) });
            m_viewportInitialized = true;
        }
        else
        {
            return addErrorEntry("Camera::setViewport failed - width and height must not be 0!");
        }

        return StatusOK;
    }

    int32_t CameraNodeImpl::getViewportX() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        const auto vpOffsetData = getIScene().getDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportOffsetField);
        return getIScene().getDataSingleVector2i(vpOffsetData, ramses_internal::DataFieldHandle{ 0 }).x;
    }

    int32_t CameraNodeImpl::getViewportY() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        const auto vpOffsetData = getIScene().getDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportOffsetField);
        return getIScene().getDataSingleVector2i(vpOffsetData, ramses_internal::DataFieldHandle{ 0 }).y;
    }

    uint32_t CameraNodeImpl::getViewportWidth() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        const auto vpSizeData = getIScene().getDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportSizeField);
        return getIScene().getDataSingleVector2i(vpSizeData, ramses_internal::DataFieldHandle{ 0 }).x;
    }

    uint32_t CameraNodeImpl::getViewportHeight() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        const auto vpSizeData = getIScene().getDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportSizeField);
        return getIScene().getDataSingleVector2i(vpSizeData, ramses_internal::DataFieldHandle{ 0 }).y;
    }

    float CameraNodeImpl::getLeftPlane() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        return getIScene().getCamera(m_cameraHandle).frustum.leftPlane;
    }

    float CameraNodeImpl::getRightPlane() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        return getIScene().getCamera(m_cameraHandle).frustum.rightPlane;
    }

    float CameraNodeImpl::getBottomPlane() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        return getIScene().getCamera(m_cameraHandle).frustum.bottomPlane;
    }

    float CameraNodeImpl::getTopPlane() const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));
        return getIScene().getCamera(m_cameraHandle).frustum.topPlane;
    }

    ramses_internal::CameraHandle CameraNodeImpl::getCameraHandle() const
    {
        return m_cameraHandle;
    }

    status_t CameraNodeImpl::getProjectionMatrix(float(&projectionMatrix)[16]) const
    {
        assert(isOfType(ERamsesObjectType_LocalCamera));

        if (!m_frustumInitialized)
        {
            return addErrorEntry("CameraImpl::getProjectionMatrix failed - Camera frustum is not initialized!");
        }

        const ramses_internal::Matrix44f projMatrix = ramses_internal::CameraMatrixHelper::ProjectionMatrix(getProjectionParams());
        ramses_internal::PlatformMemory::Copy(projectionMatrix, projMatrix.data, sizeof(projectionMatrix));

        return StatusOK;
    }

    ramses_internal::ProjectionParams CameraNodeImpl::getProjectionParams() const
    {
        const ramses_internal::Camera& camera = getIScene().getCamera(m_cameraHandle);
        return ramses_internal::ProjectionParams::Frustum(
            camera.projectionType,
            camera.frustum.leftPlane,
            camera.frustum.rightPlane,
            camera.frustum.bottomPlane,
            camera.frustum.topPlane,
            camera.frustum.nearPlane,
            camera.frustum.farPlane);
    }

    void CameraNodeImpl::updateProjectionParamsOnScene(const ramses_internal::ProjectionParams& params)
    {
        getIScene().setCameraFrustum(m_cameraHandle, { params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane, params.nearPlane, params.farPlane });
    }

    status_t CameraNodeImpl::bindViewportOffset(const DataVector2i& offsetData)
    {
        if (!isFromTheSameSceneAs(offsetData.impl))
        {
            return addErrorEntry("Camera::bindViewportOffset failed, viewport offset data object is not from the same scene as this camera");
        }

        getIScene().setDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportOffsetField, offsetData.impl.getDataReference());
        return StatusOK;
    }

    status_t CameraNodeImpl::bindViewportSize(const DataVector2i& sizeData)
    {
        if (!isFromTheSameSceneAs(sizeData.impl))
        {
            return addErrorEntry("Camera::bindViewportSize failed, viewport size data object is not from the same scene as this camera");
        }

        getIScene().setDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportSizeField, sizeData.impl.getDataReference());
        return StatusOK;
    }

    status_t CameraNodeImpl::unbindViewportOffset()
    {
        if (isViewportOffsetBound())
            getIScene().setDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportOffsetField, m_viewportOffsetDataReference);
        return StatusOK;
    }

    status_t CameraNodeImpl::unbindViewportSize()
    {
        if (isViewportSizeBound())
            getIScene().setDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportSizeField, m_viewportSizeDataReference);
        return StatusOK;
    }

    bool CameraNodeImpl::isViewportOffsetBound() const
    {
        return getIScene().getDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportOffsetField) != m_viewportOffsetDataReference;
    }

    bool CameraNodeImpl::isViewportSizeBound() const
    {
        return getIScene().getDataReference(m_viewportDataInstance, ramses_internal::Camera::ViewportSizeField) != m_viewportSizeDataReference;
    }
}
