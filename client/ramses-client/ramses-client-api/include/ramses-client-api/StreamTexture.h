//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STREAMTEXTURE_H
#define RAMSES_STREAMTEXTURE_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    /**
    * @brief StreamTexture is a special kind of texture, which holds a reference to a
    * "fallback texture" and a stream source id. The content of the StreamTexture is
    * dynamic and is determined within the renderer, based on whether the stream with
    * the specified id is available or not. If it is available, the content of the
    * StreamTexture is taken from the stream. Otherwise, the StreamTexture is replaced
    * by the "fallback texture".
    */
    class RAMSES_API StreamTexture : public SceneObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of StreamTexture.
        */
        class StreamTextureImpl& impl;

        /**
        * @brief Force usage of fallback texture even if compositing source is available
        *
        * @param[in] forceFallbackImage If set to \c true the fallback image will always be shown even if streaming source for
        *            embedded compositing is or becomes available
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t forceFallbackImage(bool forceFallbackImage);

        /**
        * @brief Get the current value, if fallback image is forced or not.
        *
        * @return If forcing of fallback image is currently enabled or not.
        */
        bool getForceFallbackImage() const;

        /**
        * @brief Get the stream source id used for compositing on the stream texture
        *
        * @return Stream source id
        */
        streamSource_t getStreamSourceId() const;

    protected:
        /**
        * @brief RamsesClient is the factory for creating StreamTexture instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of StreamTexture
        *
        * @param[in] impl Internal data for implementation specifics of StreamTexture (sink - instance becomes owner)
        */
        explicit StreamTexture(StreamTextureImpl& impl);

        /**
         * @brief Destructor of the StreamTexture
         */
        virtual ~StreamTexture();
    };
}

#endif
