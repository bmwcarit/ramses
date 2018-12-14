//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNALRENDERERAPI_TYPES_H
#define RAMSES_INTERNALRENDERERAPI_TYPES_H

#include "SceneAPI/Handles.h"
#include "Collections/Vector.h"
#include "Collections/HashSet.h"
#include "Collections/String.h"
#include "Common/StronglyTypedValue.h"
#include "Common/TypedMemoryHandle.h"
#include "Common/BitForgeMacro.h"

namespace ramses_internal
{
    enum EAntiAliasingMethod
    {
        EAntiAliasingMethod_PlainFramebuffer = 0,
        EAntiAliasingMethod_MultiSampling,
        EAntiAliasingMethod_FXAA
    };

    enum EPostProcessingEffect
    {
        EPostProcessingEffect_None = 0,
        EPostProcessingEffect_Warping = BIT(0)
    };

    struct DisplayHandleTag {};
    typedef TypedMemoryHandle<DisplayHandleTag> DisplayHandle;
    typedef Vector<DisplayHandle> DisplayHandleVector;

    struct OffscreenBufferHandleTag {};
    typedef TypedMemoryHandle<OffscreenBufferHandleTag> OffscreenBufferHandle;

    typedef void* GenericDataPtr;
    typedef const void* GenericConstDataPtr;

    typedef void* NativeHandle;

    typedef HashSet<RenderableHandle> RenderableSet;
    typedef Vector<Bool> BoolVector;
    typedef Vector<UInt8> UInt8Vector;
    typedef Vector<RenderTargetHandle> RenderTargetHandleVector;

    struct DeviceResourceHandleTag {};
    typedef TypedMemoryHandle<DeviceResourceHandleTag> DeviceResourceHandle;
    typedef Vector<DeviceResourceHandle> DeviceHandleVector;

    struct StreamTextureSourceIdTag {};
    typedef StronglyTypedValue<UInt32, 0xFFFFFFFF, StreamTextureSourceIdTag> StreamTextureSourceId;
    static const StreamTextureSourceId InvalidStreamTextureSourceId(0xFFFFFFFF);
    typedef HashSet<StreamTextureSourceId> StreamTextureSourceIdSet;
    typedef Vector<StreamTextureSourceId> StreamTextureSourceIdVector;

    // TODO Violin this needs removing - no need for two types for stream texture id...
    typedef StreamTextureSourceId WaylandIviSurfaceId;
    static const WaylandIviSurfaceId InvalidWaylandIviSurfaceId(0xFFFFFFFF);

    struct WaylandIviLayerIdTag {};
    typedef StronglyTypedValue<UInt32, 0xFFFFFFFF, WaylandIviLayerIdTag> WaylandIviLayerId;
    static const WaylandIviLayerId InvalidWaylandIviLayerId(0xFFFFFFFF);

    struct IntegrityEglDisplayIdTag {};
    typedef StronglyTypedValue<UInt32, 0xFFFFFFFF, IntegrityEglDisplayIdTag> IntegrityEglDisplayId;
    static const IntegrityEglDisplayId InvalidIntegrityEglDisplayId(0xFFFFFFFF);

    struct WindowsWindowHandleTag {};
    typedef StronglyTypedValue<void*, nullptr, WindowsWindowHandleTag> WindowsWindowHandle;
    static const WindowsWindowHandle InvalidWindowsWindowHandle(nullptr);

    struct ScreenshotInfo
    {
        struct Rectangle
        {
            UInt32 x;
            UInt32 y;
            UInt32 width;
            UInt32 height;
        };
        Rectangle     rectangle;
        DisplayHandle display;
        String        filename;
        Bool          success;
        Bool          sendViaDLT;
        UInt8Vector   pixelData;
    };
    typedef Vector<ScreenshotInfo> ScreenshotInfoVector;
}

#endif
