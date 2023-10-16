//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/Core/Common/StronglyTypedValue.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/Core/Common/TypedMemoryHandle.h"
#include "internal/Core/Common/BitForgeMacro.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

#include <unordered_set>
#include <string>

namespace ramses::internal
{
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
    using RenderTargetHandleVector = std::vector<RenderTargetHandle>;

    struct DeviceResourceHandleTag {};
    using DeviceResourceHandle = TypedMemoryHandle<DeviceResourceHandleTag>;
    using DeviceHandleVector = std::vector<DeviceResourceHandle>;

    struct WaylandIviLayerIdTag {};
    using WaylandIviLayerId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), WaylandIviLayerIdTag>;

    using ramses::X11WindowHandle;

    struct WindowsWindowHandleTag {};
    using WindowsWindowHandle = StronglyTypedValue<void *, nullptr, WindowsWindowHandleTag>;

    struct AndroidNativeWindowPtrTag {};
    using AndroidNativeWindowPtr = StronglyTypedValue<void *, nullptr, AndroidNativeWindowPtrTag>;

    using ramses::IOSNativeWindowPtr;

    struct ScreenshotInfo
    {
        struct Rectangle
        {
            uint32_t x{0};
            uint32_t y{0};
            uint32_t width{0};
            uint32_t height{0};
        };
        Rectangle            rectangle;
        std::string          filename;
        bool                 fullScreen{false};
        bool                 sendViaDLT{false};
        std::vector<uint8_t> pixelData;
    };
    using ScreenshotInfoVector = std::vector<ScreenshotInfo>;

    using BinaryShaderFormatID = StronglyTypedValue<uint32_t, 0, struct BinaryShaderFormatIDTag>;

    struct VertexBufferInfo
    {
        DeviceResourceHandle deviceHandle;
        DataFieldHandle      field;
        uint32_t             instancingDivisor{0u};
        uint32_t             startVertex{0u};
        EDataType            bufferDataType{EDataType::Invalid};
        uint16_t             offsetWithinElement{0u};
        uint16_t             stride{0u};
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

template <>
struct fmt::formatter<ramses::internal::WaylandIviLayerId> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)  {
        return ctx.begin();
    }
    template<typename FormatContext>
    constexpr auto format(const ramses::internal::WaylandIviLayerId& value, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "ivi-layer:{}", value.getValue());
    }
};

template <>
struct fmt::formatter<ramses::internal::WaylandIviSurfaceId> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)  {
        return ctx.begin();
    }
    template<typename FormatContext>
    constexpr auto format(const ramses::internal::WaylandIviSurfaceId& value, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "ivi-surface:{}", value.getValue());
    }
};

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::WindowsWindowHandle)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::AndroidNativeWindowPtr)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::IOSNativeWindowPtr)
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::BinaryShaderFormatID)
MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::ERendererLogTopic, "ERendererLogTopic", ramses::internal::RendererLogTopicNames, ramses::internal::ERendererLogTopic::PeriodicLog);
