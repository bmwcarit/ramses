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
#include "Common/StronglyTypedValue.h"
#include "SceneAPI/RendererSceneState.h"
#include "Collections/Vector.h"
#include "Collections/HashSet.h"
#include "Common/StronglyTypedValue.h"
#include "Common/TypedMemoryHandle.h"
#include "Common/BitForgeMacro.h"
#include "Utils/LoggingUtils.h"

#include <unordered_set>
#include <string>

namespace ramses_internal
{
    enum class EDataType;

    struct DisplayHandleTag {};
    using DisplayHandle = TypedMemoryHandle<DisplayHandleTag>;
    using DisplayHandleVector = std::vector<DisplayHandle>;

    struct OffscreenBufferHandleTag {};
    using OffscreenBufferHandle = TypedMemoryHandle<OffscreenBufferHandleTag>;
    struct StreamBufferHandleTag {};
    using StreamBufferHandle = TypedMemoryHandle<StreamBufferHandleTag>;
    struct ExternalTextureHandleTag {};
    using ExternalBufferHandle = TypedMemoryHandle<ExternalTextureHandleTag>;

    struct DmaBufferFourccFormatTag{};
    using DmaBufferFourccFormat = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), DmaBufferFourccFormatTag>;

    struct DmaBufferUsageFlagsTag{};
    using DmaBufferUsageFlags = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), DmaBufferUsageFlagsTag>;

    struct DmaBufferModifiersTag{};
    using DmaBufferModifiers = StronglyTypedValue<uint64_t, std::numeric_limits<uint64_t>::max(), DmaBufferModifiersTag>;

    struct WaylandIviSurfaceIdTag {};
    using WaylandIviSurfaceId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), WaylandIviSurfaceIdTag>;
    using WaylandIviSurfaceIdSet = std::unordered_set<WaylandIviSurfaceId>;
    using WaylandIviSurfaceIdVector = std::vector<WaylandIviSurfaceId>;

    using GenericDataPtr = void *;
    using GenericConstDataPtr = const void *;

    using NativeHandle = void *;

    using RenderableSet = HashSet<RenderableHandle>;
    using BoolVector = std::vector<bool>;
    using UInt8Vector = std::vector<uint8_t>;
    using RenderTargetHandleVector = std::vector<RenderTargetHandle>;

    struct DeviceResourceHandleTag {};
    using DeviceResourceHandle = TypedMemoryHandle<DeviceResourceHandleTag>;
    using DeviceHandleVector = std::vector<DeviceResourceHandle>;

    struct WaylandIviLayerIdTag {};
    using WaylandIviLayerId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), WaylandIviLayerIdTag>;

    struct X11WindowHandleTag {};
    using X11WindowHandle = StronglyTypedValue<unsigned long, std::numeric_limits<unsigned long>::max(), X11WindowHandleTag>;

    struct WindowsWindowHandleTag {};
    using WindowsWindowHandle = StronglyTypedValue<void *, nullptr, WindowsWindowHandleTag>;

    struct AndroidNativeWindowPtrTag {};
    using AndroidNativeWindowPtr = StronglyTypedValue<void *, nullptr, AndroidNativeWindowPtrTag>;

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
        std::string   filename;
        bool          fullScreen;
        bool          sendViaDLT;
        UInt8Vector   pixelData;
    };
    using ScreenshotInfoVector = std::vector<ScreenshotInfo>;

    using BinaryShaderFormatID = StronglyTypedValue<UInt32, 0, struct BinaryShaderFormatIDTag>;

    struct VertexBufferInfo
    {
        DeviceResourceHandle deviceHandle;
        DataFieldHandle field;
        UInt32 instancingDivisor;
        UInt32 startVertex;
        EDataType bufferDataType;
        UInt16 offsetWithinElement;
        UInt16 stride;
    };

    struct VertexArrayInfo
    {
        DeviceResourceHandle shader;
        DeviceResourceHandle indexBuffer;
        std::vector<VertexBufferInfo> vertexBuffers;
    };

    enum class ERendererLogTopic
    {
        Displays = 0,
        SceneStates,
        Resources,
        MissingResources,
        RenderQueue,
        Links,
        EmbeddedCompositor,
        EventQueue,
        All,
        PeriodicLog,
        COUNT
    };

    const std::array RendererLogTopicNames =
    {
        "Displays",
        "SceneStates",
        "Resources",
        "MissingResources",
        "RenderQueue",
        "Links",
        "EmbeddedCompositor",
        "EventQueue",
        "All",
        "PeriodicLog"
    };
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::WaylandIviLayerId)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::WindowsWindowHandle)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::AndroidNativeWindowPtr)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::BinaryShaderFormatID)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::WaylandIviSurfaceId)
MAKE_ENUM_CLASS_PRINTABLE(ramses_internal::ERendererLogTopic, "ERendererLogTopic", ramses_internal::RendererLogTopicNames, ramses_internal::ERendererLogTopic::COUNT);

#endif
