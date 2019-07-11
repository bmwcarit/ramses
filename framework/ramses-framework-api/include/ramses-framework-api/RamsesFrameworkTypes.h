//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKTYPES_H
#define RAMSES_RAMSESFRAMEWORKTYPES_H

#include "stdint.h"
#include "ramses-framework-api/StronglyTypedValue.h"

namespace ramses
{
    /**
    * @brief Status is a handle to the result of an API call.
    *
    * All functions of th RAMSES client API that may fail for some reason
    * return a status. This is a handle to request a full status message
    * using getStatusMessage() function.
    */
    typedef uint32_t status_t;

    /**
    * @brief Status returned from RAMSES client API methods that succeeded.
    *
    */
    const status_t StatusOK = 0u;

    /**
    * @brief Scene identifier used to refer to scenes created using client API
    *        and then manage their mapping using renderer API
    */
    typedef uint64_t sceneId_t;

    /**
    * @brief Scene version tag used to refer to content versions of a scene.
    *        A scene version may be updated along with a scene transaction.
    */
    typedef uint64_t sceneVersionTag_t;

    /**
    * @brief Scene version tag used to refer to an invalid scene version
    */
    const sceneVersionTag_t InvalidSceneVersionTag = static_cast<sceneVersionTag_t>(-1);

    /**
    * @brief Display identifier used to refer to display in renderer events during dispatching
    */
    typedef uint32_t displayId_t;

    /**
    * @brief Display id used to refer to non-existing display
    */
    const displayId_t InvalidDisplayId = static_cast<displayId_t>(-1);

    /**
    * @brief Offscreen buffer identifier referring to a buffer created on a display.
    *        The \c offscreenBufferId_t is valid in the scope of display, therefore has to be
    *        always used together with \c diplayId_t in RamsesRenderer API.
    */
    typedef uint32_t offscreenBufferId_t;

    /**
    * @brief Offscreen buffer id used to refer to non-existing buffer
    */
    const displayId_t InvalidOffscreenBufferId = static_cast<offscreenBufferId_t>(-1);

    /**
    * @brief Data identifier used to refer to data provider
    *        and then manage their linkage to data consumer using renderer API
    */
    typedef uint32_t dataProviderId_t;

    /**
    * @brief Data link identifier used to refer to data consumer
    *        and then manage their linkage to data provider using renderer API
    */
    typedef uint32_t dataConsumerId_t;

    /**
    * @brief Resource identifier used to refer to a resource
    */
    struct resourceId_t
    {
        /// Low bits
        uint64_t lowPart;
        /// High bits
        uint64_t highPart;

        /**
        * @brief Equal compare operator
        *
        * @param[in] other resourceId to compare to
        *
        * @return true of resourceId is the same as other resourceId
        */
        bool operator==(const resourceId_t& other) const
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
        bool operator!=(const resourceId_t& other) const
        {
            return !(*this == other);
        }
    };

    /**
    * @brief Invalid resource identifier
    */
    const resourceId_t InvalidResourceId = {0, 0};

    /**
    * @brief Struct used as unique id for the strongly typed node Id.
    */
    struct nodeIdTag {};

    /**
    * @brief Node identifier used to refer to a node
    */
    typedef StronglyTypedValue<uint32_t, nodeIdTag> nodeId_t;

    /**
    * @brief Struct used as unique id for the strongly typed stream Id.
    */
    struct streamIdTag {};

    /**
    * @brief The "ivi ID" of a stream source attached to a stream texture
    * The value refers to a surface ID in Wayland (extended by IVI extensions)
    */
    typedef StronglyTypedValue<uint32_t, streamIdTag> streamSource_t;

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
    * @brief Struct used as unique id for the strongly-typed cache flag.
    */
    struct resourceCacheFlagTag {};

    /**
    * @brief Cache flag value used for passing a strong-typed flag value to a renderer.
    */
    typedef StronglyTypedValue<uint32_t, resourceCacheFlagTag> resourceCacheFlag_t;

    /**
    * @brief Requests the render to not cache a resource. This is the default value.
    */
    const resourceCacheFlag_t ResourceCacheFlag_DoNotCache = resourceCacheFlag_t(static_cast<uint32_t>(-1));
}

#endif
