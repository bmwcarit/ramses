//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include <memory>
#include <string>

namespace ramses
{
    namespace internal
    {
        class RenderTargetDescriptionImpl;
    }
    class ValidationReport;
    class RenderBuffer;

    /**
    * @brief RenderTargetDescription holds all necessary information for a RenderTarget to be created.
    * @ingroup CoreAPI
    */
    class RAMSES_API RenderTargetDescription
    {
    public:
        /**
        * @brief Constructor of RenderTargetDescription
        */
        RenderTargetDescription();

        /**
        * @brief Destructor of RenderTargetDescription
        */
        ~RenderTargetDescription();

        /**
        * @copydoc #ramses::RamsesObject::validate
        */
        void validate(ValidationReport& report) const;

        /**
        * @brief Adds a RenderBuffer to the RenderTargetDescription.
        * @details Multiple color RenderBuffers and at most one depth/stencil RenderBuffer can be added.
        *          The layout of multiple color RenderBuffers corresponds to the order of adding them
        *          (first RenderBuffer added has location 0, second added has location 1, etc.).
        *          Depth/stencil RenderBuffer is a special case and its relative order to the color buffer(s) irrelevant.
        *          All added render buffers must have same MSAA sample count. Trying to add render buffers with different
        *          sample count values will fail and generate error.
        * @param[in] renderBuffer RenderBuffer to be added to the RenderTargetDescription.
        * @param[out] errorMsg In case of an error this string will be filled with description of the error. In case of success it will be empty.
        * @return true on success, false if an error occurred.
        *         Pass the optional \c errorMsg argument to get human readable description of the error, the error is also logged.
        */
        bool addRenderBuffer(const RenderBuffer& renderBuffer, std::string* errorMsg = nullptr);

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RenderTargetDescription(const RenderTargetDescription& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RenderTargetDescription(RenderTargetDescription&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RenderTargetDescription& operator=(const RenderTargetDescription& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RenderTargetDescription& operator=(RenderTargetDescription&& other) noexcept;

        /**
         * Get the internal data for implementation specifics of RenderTargetDescription.
         */
        [[nodiscard]] internal::RenderTargetDescriptionImpl& impl();

        /**
         * Get the internal data for implementation specifics of RenderTargetDescription.
         */
        [[nodiscard]] const internal::RenderTargetDescriptionImpl& impl() const;

    protected:
        /**
        * @brief Stores internal data for implementation specifics of RenderTargetDescription.
        */
        std::unique_ptr<internal::RenderTargetDescriptionImpl> m_impl;
    };
}
