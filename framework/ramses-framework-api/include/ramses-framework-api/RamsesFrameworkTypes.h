//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKTYPES_H
#define RAMSES_RAMSESFRAMEWORKTYPES_H

#include "ramses-framework-api/StronglyTypedValue.h"
#include "stdint.h"
#include <limits>

namespace ramses
{
    /**
    * @brief Status is a handle to the result of an API call.
    *
    * All functions of th RAMSES client API that may fail for some reason
    * return a status. This is a handle to request a full status message
    * using getStatusMessage() function.
    */
    using status_t = uint32_t;

    /**
    * @brief Status returned from RAMSES client API methods that succeeded.
    *
    */
    constexpr const status_t StatusOK = 0u;

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
        constexpr resourceId_t()
            : lowPart(0)
            , highPart(0)
        {
        }

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
        constexpr bool isValid() const
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
        uint64_t lowPart;
        /// High bits
        uint64_t highPart;
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
    enum ERamsesShellType
    {
        ERamsesShellType_None = 0,
        ERamsesShellType_Console,
        ERamsesShellType_Default
    };

    /**
    * @brief Type of Ramses Log Level
    */
    enum class ELogLevel
    {
        Off,
        Fatal,
        Error,
        Warn,
        Info,
        Debug,
        Trace
    };

    /**
    * @brief Struct used as unique id for the strongly-typed cache flag.
    */
    struct resourceCacheFlagTag {};

    /**
    * @brief Cache flag value used for passing a strong-typed flag value to a renderer.
    */
    using resourceCacheFlag_t = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), resourceCacheFlagTag>;

    /**
    * @brief Requests the render to not cache a resource. This is the default value.
    */
    constexpr const resourceCacheFlag_t ResourceCacheFlag_DoNotCache = resourceCacheFlag_t::Invalid();

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
}

#endif
