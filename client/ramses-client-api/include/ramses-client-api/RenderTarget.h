//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERTARGET_H
#define RAMSES_RENDERTARGET_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    /**
     * @ingroup CoreAPI
     * @brief The RenderTarget can be used as an output for a RenderPass
     */
    class RenderTarget : public SceneObject
    {
    public:

        /**
        * @brief Returns the width of the RenderTarget in pixels
        *
        * @returns width in pixels
        */
        [[nodiscard]] RAMSES_API uint32_t getWidth() const;

        /**
        * @brief Returns the height of the RenderTarget in pixels
        *
        * @returns height in pixels
        */
        [[nodiscard]] RAMSES_API uint32_t getHeight() const;

        /**
        * Stores internal data for implementation specifics of RenderTarget.
        */
        class RenderTargetImpl& m_impl;

    protected:
        /**
        * @brief Scene is the factory for creating RenderTarget instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor for RenderTarget.
        *
        * @param[in] impl Internal data for implementation specifics of RenderTarget (sink - instance becomes owner)
        */
        explicit RenderTarget(std::unique_ptr<RenderTargetImpl> impl);
    };
}

#endif
