//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYCONFIGIMPL_H
#define RAMSES_DISPLAYCONFIGIMPL_H

#include "RendererLib/DisplayConfig.h"
#include "StatusObjectImpl.h"
#include "Utils/CommandLineParser.h"

namespace ramses
{
    class DisplayConfigImpl : public StatusObjectImpl
    {
    public:
        DisplayConfigImpl(int32_t argc, char const* const* argv);

        status_t setViewPosition(float x, float y, float z);
        status_t getViewPosition(float& x, float& y, float& z) const;
        status_t setViewRotation(float x, float y, float z);
        status_t getViewRotation(float& x, float& y, float& z) const;
        status_t setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height);
        status_t setFullscreen(bool fullscreen);
        status_t setBorderless(bool borderless);
        status_t setPerspectiveProjection(float fieldOfViewY, float aspectRatio, float nearPlane, float farPlane);
        status_t setProjection(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane, bool isOrthographic);
        status_t setMultiSampling(uint32_t numSamples);
        status_t getMultiSamplingSamples(uint32_t& numSamples) const;
        status_t enableWarpingPostEffect();
        status_t enableStereoDisplay();
        status_t setWaylandIviSurfaceID(uint32_t waylandIviSurfaceID);
        uint32_t getWaylandIviSurfaceID() const;
        status_t setWaylandIviLayerID(uint32_t waylandIviLayerID);
        uint32_t getWaylandIviLayerID() const;
        status_t setWaylandDisplay(const char* waylandDisplay);
        const char* getWaylandDisplay() const;
        status_t setIntegrityRGLDeviceUnit(uint32_t rglDeviceUnit);
        uint32_t getIntegrityRGLDeviceUnit() const;
        status_t setWindowIviVisible();
        status_t setResizable(bool resizable);
        status_t keepEffectsUploaded(bool enable);
        status_t setGPUMemoryCacheSize(uint64_t size);
        status_t setClearColor(float red, float green, float blue, float alpha);
        status_t setOffscreen(bool offscreenFlag);
        status_t setWindowsWindowHandle(void* hwnd);
        void*    getWindowsWindowHandle() const;

        virtual status_t validate(uint32_t indent = 0u) const override;

        //impl methods
        const ramses_internal::DisplayConfig& getInternalDisplayConfig() const;

    private:
        ramses_internal::DisplayConfig m_internalConfig;
    };
}

#endif
