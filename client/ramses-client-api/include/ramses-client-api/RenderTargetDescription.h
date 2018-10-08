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

    /**
    * @brief RenderTargetDescription holds all necessary information for a RenderTarget to be created.
    */
    class RAMSES_API RenderTargetDescription : public StatusObject
    {
    public:
        /**
        * @brief Constructor of RenderTargetDescription
        */
        RenderTargetDescription();

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
        status_t addRenderBuffer(const RenderBuffer& renderBuffer);

        /**
        * @brief Stores internal data for implementation specifics of RenderTargetDescription.
        */
        class RenderTargetDescriptionImpl& impl;
    };
}

#endif
