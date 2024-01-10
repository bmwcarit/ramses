//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/SceneObject.h"

namespace ramses
{
    namespace internal
    {
        class RenderTargetImpl;
    }

    /**
     * @brief The RenderTarget can be used as an output for a RenderPass.
     * @ingroup CoreAPI
     */
    class RAMSES_API RenderTarget : public SceneObject
    {
    public:

        /**
        * @brief Returns the width of the RenderTarget in pixels
        *
        * @returns width in pixels
        */
        [[nodiscard]] uint32_t getWidth() const;

        /**
        * @brief Returns the height of the RenderTarget in pixels
        *
        * @returns height in pixels
        */
        [[nodiscard]] uint32_t getHeight() const;

        /**
         * Get the internal data for implementation specifics of RenderTarget.
         */
        [[nodiscard]] internal::RenderTargetImpl& impl();

        /**
         * Get the internal data for implementation specifics of RenderTarget.
         */
        [[nodiscard]] const internal::RenderTargetImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating RenderTarget instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor for RenderTarget.
        *
        * @param[in] impl Internal data for implementation specifics of RenderTarget (sink - instance becomes owner)
        */
        explicit RenderTarget(std::unique_ptr<internal::RenderTargetImpl> impl);

        /**
        * Stores internal data for implementation specifics of RenderTarget.
        */
        internal::RenderTargetImpl& m_impl;
    };
}
