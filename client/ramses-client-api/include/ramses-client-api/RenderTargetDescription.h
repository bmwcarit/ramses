//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERTARGETDESCRIPTION_H
#define RAMSES_RENDERTARGETDESCRIPTION_H

#include "ramses-framework-api/StatusObject.h"

namespace ramses
{
    class RenderBuffer;
    class RenderTargetDescriptionImpl;

    /**
    * @ingroup CoreAPI
    * @brief RenderTargetDescription holds all necessary information for a RenderTarget to be created.
    */
    class RenderTargetDescription : public StatusObject
    {
    public:
        /**
        * @brief Constructor of RenderTargetDescription
        */
        RAMSES_API RenderTargetDescription();

        /**
        * @brief Destructor of RenderTargetDescription
        */
        RAMSES_API ~RenderTargetDescription() override;

        /**
        * @brief Adds a RenderBuffer to the RenderTargetDescription.
        * @details Multiple color RenderBuffers and at most one depth/stencil RenderBuffer can be added.
        *          The layout of multiple color RenderBuffers corresponds to the order of adding them
        *          (first RenderBuffer added has location 0, second added has location 1, etc.).
        *          Depth/stencil RenderBuffer is a special case and its relative order to the color buffer(s) irrelevant.
        *          All added render buffers must have same MSAA sample count. Trying to add render buffers with different
        *          sample count values will fail and generate error.
        * @param[in] renderBuffer RenderBuffer to be added to the RenderTargetDescription.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t addRenderBuffer(const RenderBuffer& renderBuffer);

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RAMSES_API RenderTargetDescription(const RenderTargetDescription& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RAMSES_API RenderTargetDescription(RenderTargetDescription&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RAMSES_API RenderTargetDescription& operator=(const RenderTargetDescription& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RAMSES_API RenderTargetDescription& operator=(RenderTargetDescription&& other) noexcept;

        /**
        * @brief Stores internal data for implementation specifics of RenderTargetDescription.
        */
        std::reference_wrapper<RenderTargetDescriptionImpl> m_impl;
    };
}

#endif
