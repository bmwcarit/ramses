//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/StronglyTypedValue.h"
#include "ramses/framework/Flags.h"
#include <cstdint>
#include <limits>
#include <string>

namespace ramses
{
    /**
    * @brief Struct used as unique id for the strongly typed scene id.
    */
    struct SceneIdTag {};

    /**
     * @brief Scene identifier used to refer to scenes created using client API
     *        and then manage their mapping using renderer API
     */
    using sceneId_t = StronglyTypedValue<uint64_t, 0, struct SceneIdTag>;

    /**
    * @brief Scene version tag used to refer to content versions of a scene.
    *        A scene version may be updated along with a scene transaction.
    */
    using sceneVersionTag_t = uint64_t;

    /**
    * @brief Scene version tag used to refer to an invalid scene version
    */
    constexpr const sceneVersionTag_t InvalidSceneVersionTag = static_cast<sceneVersionTag_t>(-1);

    /// Display identifier used to refer to display in renderer API and dispatched callbacks
    using displayId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), struct DisplayIdTag>;

    /// Display buffer identifier referring to either a display's framebuffer or a created offscreen buffer.
    using displayBufferId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), struct DisplayBufferIdTag>;

    /// Stream buffer identifier referring to Wayland stream.
    using streamBufferId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), struct StreamBufferIdTag>;

    /// External buffer identifier referring to external buffer.
    using externalBufferId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), struct ExternalBufferIdTag>;

    /**
    * @brief Data identifier used to refer to data provider
    *        and then manage their linkage to data consumer using renderer API
    */
    using dataProviderId_t = StronglyTypedValue<uint32_t, 0, struct DataProviderIdTag>;

    /**
    * @brief Data link identifier used to refer to data consumer
    *        and then manage their linkage to data provider using renderer API
    */
    using dataConsumerId_t = StronglyTypedValue<uint32_t, 0, struct DataConsumerIdTag>;

    /**
    * @brief Resource identifier used to refer to a resource
    */
    struct resourceId_t
    {
        /**
         * @brief Default constructor initialized to Invalid
         */
        constexpr resourceId_t() = default;

        /**
         * @brief Construct with low and high part
         * @param low low part of resource id
         * @param high high part of resource id
         */
        constexpr resourceId_t(uint64_t low, uint64_t high)
            : lowPart(low)
            , highPart(high)
        {
        }

        /**
         * @brief Create invalid resource Id
         * @return Invalid resource id
         */
        static constexpr resourceId_t Invalid()
        {
            return resourceId_t();
        }

        /**
         * @brief Check if resource is valid
         * @return true when resource id has a valid value, false otherwise
         */
        [[nodiscard]] constexpr bool isValid() const
        {
            return *this != Invalid();
        }

        /**
        * @brief Equal compare operator
        *
        * @param[in] other resourceId to compare to
        *
        * @return true of resourceId is the same as other resourceId
        */
        constexpr bool operator==(const resourceId_t& other) const
        {
            return highPart == other.highPart && lowPart == other.lowPart;
        }

        /**
        * @brief Unequal compare operator
        *
        * @param[in] other resourceId to compare to
        *
        * @return true of resourceId is not the same as other resourceId
        */
        constexpr bool operator!=(const resourceId_t& other) const
        {
            return !(*this == other);
        }

        /// Low bits
        uint64_t lowPart{0};
        /// High bits
        uint64_t highPart{0};
    };

    /**
    * @brief Struct used as unique id for the strongly typed node Id.
    */
    struct nodeIdTag {};

    /**
    * @brief Node identifier used to refer to a node
    */
    using nodeId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), nodeIdTag>;

    /**
    * @brief Type of Ramses Shell
    */
    enum class ERamsesShellType
    {
        None,
        Console,
        Default
    };

    /**
    * @brief Type of Ramses Log Level
    */
    enum class ELogLevel
    {
        Off = 0,    ///< No logs shall be issues, no matter the severity
        Fatal = 1,  ///< Log only fatal errors
        Error = 2,  ///< Log all errors
        Warn = 3,   ///< Log warnings + errors
        Info = 4,   ///< Include general info logs in addition to warn + errors
        Debug = 5,  ///< Debug logs - use this only for debugging
        Trace = 6   ///< Verbose trace logs - use only for debugging and inspection
    };

    /**
    * The #LogHandlerFunc can be used to implement a custom log handler. The
    * function is called for each log message separately.
    * Example
    * \code{.cpp}
    *   ramses::RamsesFramework::SetLogHandler([](ELogLevel logLevel, std::string_view context, std::string_view message){
    *       std::cout << message << std::endl;
    *   });
    * \endcode
    */
    using LogHandlerFunc = std::function<void(ELogLevel, std::string_view, std::string_view)>;

    /// Dummy struct to uniquely define ramses::pickableObjectId_t
    struct pickableObjectTag {};

    /// User ID of a pickable object to be provided when creating ramses::PickableObject
    using pickableObjectId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), pickableObjectTag>;

    /// Binary shader format identifier used in ramses::IBinaryShaderCache
    using binaryShaderFormatId_t = StronglyTypedValue<uint32_t, 0, struct binaryShaderFormatIdTag>;

    /// Dummy struct to uniquely define ramses::sceneObjectId_t
    struct sceneObjectTag {};

    /// ID that is automatically given to ramses::SceneObject on creation
    using sceneObjectId_t = StronglyTypedValue<uint64_t, 0, sceneObjectTag>;

    /**
    * @brief Struct used as unique id for the strongly typed Wayland IVI Surface Id.
    */
    struct waylandIviSurfaceIdTag {};

    /**
    * @brief Id assigned to Surface in Wayland with IVI extension
    * Also known as "ivi id"
    */
    using waylandIviSurfaceId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), waylandIviSurfaceIdTag>;


    /**
    * @brief Struct used as unique id for the strongly typed Wayland IVI Layer Id.
    */
    struct waylandIviLayerIdTag {};

    /**
    * @brief Id assigned to Layer in Wayland with IVI extension
    */
    using waylandIviLayerId_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), waylandIviLayerIdTag>;

    /**
    * @brief Struct used as unique id for the strongly typed X11 window handle.
    */
    struct X11WindowHandleTag {};

    /**
    * @brief Platform handle to a X11 window
    */
    // NOLINTNEXTLINE(google-runtime-int): unsigned long is used in X11
    using X11WindowHandle = StronglyTypedValue<unsigned long, std::numeric_limits<unsigned long>::max(), X11WindowHandleTag>;

    /**
    * @brief Struct used as unique id for the strongly typed iOS window pointer.
    */
    struct IOSNativeWindowPtrTag {};

    /**
    * @brief Platform handle to an iOS window native pointer (CAMetalLayer)
    */
    using IOSNativeWindowPtr = StronglyTypedValue<void*, nullptr, IOSNativeWindowPtrTag>;

    /**
    * @brief Clear flags used to specify which components of a render target or display buffer should be cleared.
    *
    */
    enum class EClearFlag : uint32_t
    {
        None      = 0,

        Color     = 1 << 0,
        Depth     = 1 << 1,
        Stencil   = 1 << 2,

        All       = Color | Depth | Stencil
    };

    using ClearFlags = Flags<EClearFlag>;
    template <> struct is_flag<EClearFlag> : std::true_type {};

    /**
    * @brief Supported connection systems for distributed rendering
    */
    enum class EConnectionSystem : uint32_t
    {
        TCP,
        Off
    };
}
