//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"

#include <cstdint>
#include <string_view>

namespace ramses::internal
{
    class ISystemCompositorController
    {
    public:
        virtual ~ISystemCompositorController() = default;

        /**
         *  Update internal state and do event processing.
         */
        virtual void update() = 0;

        /**
         * print out a list of known ivi surfaces
         */
        virtual void listIVISurfaces() const = 0;

        /**
         * Set the visibility of a surface.
         * @param id IVI-ID identifying the surface.
         * @param visibility New value to set.
         * @return "true", if the visibility was successfully set, and "false" otherwise.
         */
        virtual bool setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility) = 0;

        /**
         * Set the opacity of a surface.
         * @param surfaceId IVI-ID identifying the surface.
         * @param opacity Opacity value in the range 0.0 (transparent) to 1.0 (fully opaque).
         * @return true on success, false otherwise.
         */
        virtual bool setSurfaceOpacity(WaylandIviSurfaceId surfaceId, float opacity) = 0;

        /**
         * Set the output rectangle of a surface.
         * @param surfaceid IVI-ID identifying the surface.
         * @param x Position of surface along x-axis.
         * @param y Position of surface along y-axis.
         * @param width Output width of surface.
         * @param height Output height of surface.
         * @return true on success, false otherwise
         */
        virtual bool setSurfaceDestinationRectangle(WaylandIviSurfaceId surfaceId, int32_t x, int32_t y, int32_t width, int32_t height) = 0;

        /**
         * Trigger the System-Compositor (wayland, X11, android, ...) to take a screenshot and store it in a file.
         * @param fileName File name including path, for storing the screenshot.
         * @param sceenIviId IVI screen id used for screenshot
         * @return "true", if the screenshot was successfully made.
         */
        virtual bool doScreenshot(std::string_view fileName, int32_t screenIviId) = 0;

        /**
         * @brief Connect an IVI surface with a layer
         *
         * @param surfaceId IVI-ID identifying the surface
         * @param layerId IVI-ID identifying the layer
         *
         * @return \c true on success, \c false otherwise
         */
        virtual bool addSurfaceToLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) = 0;

        /**
         * @brief Disconnects an IVI surface from a layer
         *
         * @param surfaceId IVI-ID identifying the surface
         * @param layerId IVI-ID identifying the layer
         *
         * @return \c true on success, \c false otherwise
         */
        virtual bool removeSurfaceFromLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) = 0;

        /**
         * @brief Destroys an IVI surface
         * If it is added to a layer it will also be removed
         *
         * @param surfaceId IVI-ID identifying the surface
         *
         * @return \c true on success, \c false otherwise
         */
        virtual bool destroySurface(WaylandIviSurfaceId surfaceId) = 0;

        /**
         * @brief changes the visibility setting of a layer
         *
         * @param layerId The id identifying the layer.
         * @param visibility If \c true the layer will be set to visible, otherwise invisible.
         *
         * @return \c true on success \c false otherwise
         */
        virtual bool setLayerVisibility(WaylandIviLayerId layerId, bool visibility) = 0;

    };
}
