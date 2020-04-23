//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDEREREVENTHANDLER_H
#define RAMSES_IRENDEREREVENTHANDLER_H

#include "Types.h"
#include "ramses-framework-api/APIExport.h"
#include <chrono>

namespace ramses
{
    /**
    * @brief Provides an interface for handling the result of renderer events.
    *        Implementation of this interface must be passed to RamsesRenderer::dispatchEvents
    *        which will in return invoke methods of the interface according to events that occurred since last dispatching.
    */
    class RAMSES_API IRendererEventHandler
    {
    public:
        /**
        * @brief This method will be called after an offscreen buffer is created (or failed to be created) as a result of RamsesRenderer API \c createOffscreenBuffer call.
        *
        * @param displayId Display id of display that the callback refers to.
        * @param offscreenBufferId The id of the offscreen buffer that was created or failed to be created.
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed.
        */
        virtual void offscreenBufferCreated(displayId_t displayId, displayBufferId_t offscreenBufferId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after an offscreen buffer is destroyed (or failed to be destroyed) as a result of RamsesRenderer API \c destroyOffscreenBuffer call.
        *
        * @param displayId Display id of display that the callback refers to.
        * @param offscreenBufferId The id of the offscreen buffer that was destroyed or failed to be destroyed.
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed.
        */
        virtual void offscreenBufferDestroyed(displayId_t displayId, displayBufferId_t offscreenBufferId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a read back of pixels from framebuffer
        *        was finished. This is the result of RamsesRenderer::readPixels call which
        *        triggers an asynchronous read back from the internal device.
        * @param pixelData Pointer to the pixel data in uncompressed RGBA8 format.
        *                  Check result and pixelDataSize first to determine the state and size of the data.
        *                  The data is available at the pointer only during the dispatch of this event.
        *                  The pointer is nullptr in case of failure.
        * @param pixelDataSize The number of elements in the data array pixelData, ie. number of pixels * 4 (color channels).
        *                      The size is 0 in case of failure.
        * @param displayId Display id of display that the callback refers to.
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when update of warping mesh data was finished.
        *        This is the result of RamsesRenderer::updateWarpingMeshData call which
        *        triggers an asynchronous update of warping data used by internal display.
        * @param displayId Display id of display that the callback refers to.
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void warpingMeshDataUpdated(displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a display was created (or failed to create) as a result of RamsesRenderer API createDisplay call.
        *
        * @param displayId id of the display that was created and initialized (or failed in case of error).
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void displayCreated(displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a display was destroyed (or failed to destroy) as a result of RamsesRenderer API destroyDisplay call.
        *
        * @param displayId Display id of display that the callback refers to.
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void displayDestroyed(displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a key has been pressed while a display's window was focused
        * @param displayId The display on which the event occurred
        * @param eventType Specifies which type of key event has occurred
        * @param keyModifiers Modifiers used while pressing a key (bit mask of EKeyModifier)
        * @param keyCode The actual key which was pressed
        */
        virtual void keyEvent(displayId_t displayId, EKeyEvent eventType, uint32_t keyModifiers, EKeyCode keyCode) = 0;


        /**
        * @brief This method will be called when a mouse event action has occured while a display's window was focused
        * @param displayId The display on which the event occurred
        * @param eventType Specifies which kind of mouse action has occurred
        * @param mousePosX Horizontal mouse position related to window (left = 0)
        * @param mousePosY Vertical mouse position related to window (top = 0)
        */
        virtual void mouseEvent(displayId_t displayId, EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) = 0;

        /**
        * @brief This method will be called when a display's window has been resized
        * @param displayId The ramses display whose corresponding window was resized
        * @param width The new width of the window
        * @param height The new height of the window
        */
        virtual void windowResized(displayId_t displayId, uint32_t width, uint32_t height) = 0;

        /**
        * @brief This method will be called when a display's window has been moved,
        *        if the renderer uses WGL/Windows or X11/Linux as a window system.
        *
        * @param displayId The ramses display whose corresponding window was resized
        * @param windowPosX The new horizontal position of the window's upper left corner
        * @param windowPosY The new vertical position of the window's upper left corner
        */
        virtual void windowMoved(displayId_t displayId, int32_t windowPosX, int32_t windowPosY) = 0;

        /**
        * @brief This method will be called when a display's window has been closed
        * @param displayId The display on which the event occurred
        */
        virtual void windowClosed(displayId_t displayId) = 0;

        /**
        * @brief This method will be called when there were scene objects picked.
        *        A ramses::PickableObject can be 'picked' via a pick input event
        *        which is passed to renderer where the scene is rendered (see ramses::RamsesRenderer::handlePickInput).
        *
        * @param[in] sceneId ID of scene to which the picked objects belong.
        * @param[in] pickedObjects Pointer to first ID of the picked objects array.
        *                          This array is valid only for the time of calling this method.
        * @param[in] pickedObjectsCount Number of picked object IDs in the \c pickedObjects array.
        */
        virtual void objectsPicked(sceneId_t sceneId, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) = 0;

        /**
        * @brief This method will be called in period given to renderer config and provides rough performance indicators.
        *        It only works when using render thread.
        *
        * @param[in] maximumLoopTime The maximum time a loop of the renderthread took since the last event
        * @param[in] averageLooptime The average time a loop of the renderthread took since the last event
        */
        virtual void renderThreadLoopTimings(std::chrono::microseconds maximumLoopTime, std::chrono::microseconds averageLooptime) = 0;

        /**
        * @brief Empty destructor
        */
        virtual ~IRendererEventHandler() = default;
    };

    /**
    * @brief Convenience empty implementation of IRendererEventHandler that can be used to derive from
    *        when only subset of event handling methods need to be implemented.
    */
    class RAMSES_API RendererEventHandlerEmpty : public IRendererEventHandler
    {
    public:
        /**
        * @copydoc ramses::IRendererEventHandler::offscreenBufferCreated
        */
        virtual void offscreenBufferCreated(displayId_t displayId, displayBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)offscreenBufferId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::offscreenBufferDestroyed
        */
        virtual void offscreenBufferDestroyed(displayId_t displayId, displayBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)offscreenBufferId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::framebufferPixelsRead
        */
        virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, displayId_t displayId, ERendererEventResult result) override
        {
            (void)pixelData;
            (void)pixelDataSize;
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::warpingMeshDataUpdated
        */
        virtual void warpingMeshDataUpdated(displayId_t displayId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::displayCreated
        */
        virtual void displayCreated(displayId_t displayId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::displayDestroyed
        */
        virtual void displayDestroyed(displayId_t displayId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::keyEvent
        */
        virtual void keyEvent(displayId_t displayId, EKeyEvent eventType, uint32_t keyModifiers, EKeyCode keyCode) override
        {
            (void)displayId;
            (void)eventType;
            (void)keyModifiers;
            (void)keyCode;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::mouseEvent
        */
        virtual void mouseEvent(displayId_t displayId, EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override
        {
            (void)displayId;
            (void)eventType;
            (void)mousePosX;
            (void)mousePosY;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::windowResized
        */
        virtual void windowResized(displayId_t displayId, uint32_t width, uint32_t height) override
        {
            (void)displayId;
            (void)width;
            (void)height;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::windowMoved
        */
        virtual void windowMoved(displayId_t displayId, int32_t windowPosX, int32_t windowPosY) override
        {
            (void)displayId;
            (void)windowPosX;
            (void)windowPosY;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::windowClosed
        */
        virtual void windowClosed(displayId_t displayId) override
        {
            (void)displayId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::objectsPicked
        */
        virtual void objectsPicked(sceneId_t sceneId, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override
        {
            (void)sceneId;
            (void)pickedObjects;
            (void)pickedObjectsCount;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::renderThreadLoopTimings
        */
        virtual void renderThreadLoopTimings(std::chrono::microseconds maximumLoopTime, std::chrono::microseconds averageLooptime) override
        {
            (void)maximumLoopTime;
            (void)averageLooptime;
        }

    };
}

#endif
