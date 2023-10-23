//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Node.h"

namespace ramses
{
    namespace internal
    {
        class CameraNodeImpl;
    }

    class DataObject;

    /**
    * @ingroup CoreAPI
    * @brief   The #Camera base class is part of a scene and defines a view into the scene
    *          defined by the client application. It is also a #Node with transformation.
    * @details A valid camera for rendering must have viewport and frustum set.
    */
    class RAMSES_API Camera : public Node
    {
    public:
        /**
        * @brief   Sets camera frustum planes of the #Camera.
        * @details It is mandatory to set frustum planes using either this method or #ramses::PerspectiveCamera::setFrustum
        *          in order to have a valid camera for rendering.
        *
        *          In order to set valid frustum, all these must be true:
        *              leftPlane < rightPlane & bottomPlane < topPlane & 0 < nearPlane < farPlane.
        *
        *          Important note: if frustum planes data is bound (see #bindFrustumPlanes)
        *          the values set here will not be effective until unbound again, bound values are always overridden by values
        *          from bound data object. Bound values can only be modified via the #ramses::DataObject bound to them.
        *
        * @param[in] leftPlane Left plane of the camera frustum.
        *                      Left opening angle if camera is perspective.
        * @param[in] rightPlane Right plane of the camera frustum.
        *                       Right opening angle if camera is perspective.
        * @param[in] bottomPlane Bottom plane of the camera frustum.
        *                        Bottom opening angle if camera is perspective.
        * @param[in] topPlane Top plane of the camera frustum.
        *                     Top opening angle if camera is perspective.
        * @param[in] nearPlane Near plane of the camera frustum.
        * @param[in] farPlane Far plane of the camera frustum.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane);

        /**
        * @brief Sets the viewport to be used when rendering with this camera.
        * @details The viewport size does not have to match the size of the destination render buffer (#ramses::RenderTarget or display's framebuffer).
        *          However when the size matches, the projected camera frustum will fill destination buffer fully.
        *          The viewport is aligned with the lower left corner of the destination render buffer.
        *
        *          Important note: if viewport data is bound (see #bindViewportOffset or #bindViewportSize)
        *          the value set here will not be effective until unbound again, bound values are always overridden by values
        *          from bound data object. Bound values can only be modified via the #ramses::DataObject bound to them.
        *
        *          The viewport position (\p x and \p y) are relative offsets to the destination render buffer's lower left pixel.
        *          The viewport size (\p width and \p height) must be positive numbers in the range [1, 32768].
        *
        * @param[in] x horizontal offset of the viewport rectangle in pixels (zero = leftmost pixel)
        * @param[in] y vertical offset of the viewport rectangle in pixels (zero = bottommost pixel)
        * @param[in] width horizontal size of the viewport rectangle in pixels (min 1, max 32768)
        * @param[in] height vertical size of the viewport rectangle in pixels (min 1, max 32768)
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setViewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

        /**
        * @brief   Returns the horizontal offset of the viewport in pixels relative to lower left corner of destination render buffer.
        * @details If viewport data is bound (#bindViewportOffset) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setViewport.
        *
        * @return horizontal offset of the viewport in pixels
        */
        [[nodiscard]] int32_t getViewportX() const;

        /**
        * @brief   Returns the vertical offset of the viewport in pixels relative to lower left corner of destination render buffer.
        * @details If viewport data is bound (#bindViewportOffset) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setViewport.
        *
        * @return vertical offset of the viewport in pixels
        */
        [[nodiscard]] int32_t getViewportY() const;

        /**
        * @brief   Returns the viewport width in pixels
        * @details If viewport data is bound (#bindViewportSize) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setViewport.
        *
        * @return viewport width in pixels
        */
        [[nodiscard]] uint32_t getViewportWidth() const;

        /**
        * @brief   Returns the viewport height in pixels
        * @details If viewport data is bound (#bindViewportSize) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setViewport.
        *
        * @return viewport height in pixels
        */
        [[nodiscard]] uint32_t getViewportHeight() const;

        /**
        * @brief   Returns the left plane of the camera frustum
        * @details If frustum planes data is bound (#bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return the left plane of the Camera
        */
        [[nodiscard]] float getLeftPlane() const;

        /**
        * @brief   Returns the right plane of the camera frustum
        * @details If frustum planes data is bound (#bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return the right plane of the #Camera
        */
        [[nodiscard]] float getRightPlane() const;

        /**
        * @brief   Returns the bottom plane of the camera frustum
        * @details If frustum planes data is bound (#bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return the bottom plane of the #Camera
        */
        [[nodiscard]] float getBottomPlane() const;

        /**
        * @brief   Returns the top plane of the camera frustum
        * @details If frustum planes data is bound (#bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return the top plane of the #Camera
        */
        [[nodiscard]] float getTopPlane() const;

        /**
        * @brief Returns the near plane of the camera frustum
        * @details If frustum planes data is bound (#bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return the near plane of the #Camera
        */
        [[nodiscard]] float getNearPlane() const;

        /**
        * @brief Returns the far plane of the camera frustum
        * @details If frustum planes data is bound (#bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return the far plane of the #Camera
        */
        [[nodiscard]] float getFarPlane() const;

        /**
        * @brief Gets projection matrix based on camera parameters.
        *        Projection matrix can only be retrieved after all parameters
        *        were set and are valid.
        *
        * @param[out] projectionMatrix Will be filled with the projection matrix 4x4 column-major
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        [[nodiscard]] bool getProjectionMatrix(matrix44f& projectionMatrix) const;

        /**
        * @brief Binds a #ramses::DataObject to be used as source for viewport offset values.
        * @details In addition to #setViewport, which sets viewport parameters directly,
        *          a #ramses::DataObject can be bound to viewport offset and size.
        *          When a #ramses::DataObject is bound the values from it override those set using #setViewport,
        *          see #ramses::DataObject for possible use cases.
        *
        * @param[in] offsetData Data object of type #ramses::EDataType::Vector2I that will be used as source for viewport offset values
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool bindViewportOffset(const DataObject& offsetData);

        /**
        * @brief Binds a #ramses::DataObject to be used as source for viewport size values.
        *
        * @details In addition to #setViewport, which sets viewport parameters directly,
        *          a #ramses::DataObject can be bound to viewport offset and size.
        *          When a #ramses::DataObject is bound the values from it override those set using #setViewport,
        *          see #ramses::DataObject for possible use cases.
        *
        * @param[in] sizeData Data object of type #ramses::EDataType::Vector2I that will be used as source for viewport size values
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool bindViewportSize(const DataObject& sizeData);

        /**
        * @brief Binds #ramses::DataObject to be used as source for frustum planes values.
        *
        * @details In addition to #setFrustum (alternatively #ramses::PerspectiveCamera::setFrustum), which sets
        *          frustum planes directly, a #ramses::DataObject can be bound to provide values for left, right,
        *          bottom, top planes and another #ramses::DataObject for near and far planes.
        *          When a #ramses::DataObject is bound the values from it override those set using #setFrustum,
        *          see #ramses::DataObject for possible use cases.
        *          See #ramses::RamsesUtils::SetPerspectiveCameraFrustumToDataObjects providing way to conveniently
        *          set perspective frustum on data objects also with basic validity checking.
        *
        * @param[in] frustumPlanesData Data object of type #ramses::EDataType::Vector4F that will be used as source for frustum planes values.
        *                              The (x, y, z, w) floats represent (left, right, bottom, top) frustum planes.
        * @param[in] nearFarPlanesData Data object of type #ramses::EDataType::Vector2F that will be used as source for frustum planes values.
        *                              The (x, y) floats represent (near, far) frustum planes.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool bindFrustumPlanes(const DataObject& frustumPlanesData, const DataObject& nearFarPlanesData);

        /**
        * @brief Unbinds any bound #ramses::DataObject from viewport offset (see #bindViewportOffset).
        *        Does nothing if no #ramses::DataObject bound.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool unbindViewportOffset();

        /**
        * @brief Unbinds any bound #ramses::DataObject from viewport size (see #bindViewportSize).
        *        Does nothing if no #ramses::DataObject bound.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool unbindViewportSize();

        /**
        * @brief Unbinds any bound #ramses::DataObject from frustum planes (see #bindFrustumPlanes).
        *        Does nothing if no #ramses::DataObject bound.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool unbindFrustumPlanes();

        /**
        * @brief Checks if there is a #ramses::DataObject bound to viewport offset (see #bindViewportOffset).
        * @return True if there is any #ramses::DataObject bound, false otherwise.
        */
        [[nodiscard]] bool isViewportOffsetBound() const;

        /**
        * @brief Checks if there is a #ramses::DataObject bound to viewport size (see #bindViewportSize).
        * @return True if there is any #ramses::DataObject bound, false otherwise.
        */
        [[nodiscard]] bool isViewportSizeBound() const;

        /**
        * @brief Checks if there is a #ramses::DataObject bound to viewport size (see #bindFrustumPlanes).
        * @return True if there is any #ramses::DataObject bound, false otherwise.
        */
        [[nodiscard]] bool isFrustumPlanesBound() const;

        /**
         * Get the internal data for implementation specifics of Camera.
         */
        [[nodiscard]] internal::CameraNodeImpl& impl();

        /**
         * Get the internal data for implementation specifics of Camera.
         */
        [[nodiscard]] const internal::CameraNodeImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating #Camera instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor for #Camera.
        *
        * @param[in] impl Internal data for implementation specifics of #Camera (sink - instance becomes owner)
        */
        explicit Camera(std::unique_ptr<internal::CameraNodeImpl> impl);

        /**
        * Stores internal data for implementation specifics of Camera.
        */
        internal::CameraNodeImpl& m_impl;
    };
}
