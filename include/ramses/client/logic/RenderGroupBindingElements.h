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
#include <string_view>

namespace ramses
{
    class RenderGroup;
    class MeshNode;
}

namespace ramses::internal
{
    class RenderGroupBindingElementsImpl;
}

namespace ramses
{
    /**
     * #RenderGroupBindingElements is a helper class holding a set of references to elements to be provided when constructing #ramses::RenderGroupBinding.
     * These elements are either ramses::MeshNode or ramses::RenderGroup.
     *
     * Note that ramses::RenderGroup can contain other (nested) ramses::RenderGroup objects, in such case the parent ramses::RenderGroup corresponds
     * to the #ramses::RenderGroupBinding to be created and the nested ramses::RenderGroup is the element provided here.
     * @ingroup LogicAPI
     */
    class RAMSES_API RenderGroupBindingElements
    {
    public:
        /// Constructor of RenderGroupBindingElements.
        RenderGroupBindingElements() noexcept;
        /// Destructor of RenderGroupBindingElements.
        ~RenderGroupBindingElements() noexcept;
        /// Copy constructor
        RenderGroupBindingElements(const RenderGroupBindingElements& other);
        /// Move constructor
        RenderGroupBindingElements(RenderGroupBindingElements&& other) noexcept;
        /// Assignment operator
        RenderGroupBindingElements& operator=(const RenderGroupBindingElements& other);
        /// Move assignment operator
        RenderGroupBindingElements& operator=(RenderGroupBindingElements&& other) noexcept;

        /**
         * Add ramses::MeshNode element to control its render order when provided to #ramses::RenderGroupBinding.
         * Will fail if given element is already contained.
         *
         * @param meshNode ramses::MeshNode element to add to be exposed for render order control.
         * @param elementName This name will be used to name the input property in the created #ramses::RenderGroupBinding.
         *                    If none provided, name of the ramses::MeshNode will be used.
         * @return \c true if successful, \c false otherwise.
         */
        bool addElement(const ramses::MeshNode& meshNode, std::string_view elementName = {});

        /**
         * Add ramses::RenderGroup element to control its render order when provided to #ramses::RenderGroupBinding.
         * Will fail if given element is already contained.
         *
         * @param nestedRenderGroup ramses::RenderGroup element to add to be exposed for render order control.
         * @param elementName This name will be used to name the input property in the created #ramses::RenderGroupBinding.
         *                    If none provided, name of the ramses::RenderGroup will be used.
         * @return \c true if successful, \c false otherwise.
         */
        bool addElement(const ramses::RenderGroup& nestedRenderGroup, std::string_view elementName = {});

        /**
         * Get the internal data for implementation specifics of RenderGroupBindingElements.
         */
        [[nodiscard]] internal::RenderGroupBindingElementsImpl& impl();

        /**
         * Get the internal data for implementation specifics of RenderGroupBindingElements.
         */
        [[nodiscard]] const internal::RenderGroupBindingElementsImpl& impl() const;

    protected:
        /**
         * Implementation detail of RenderGroupBindingElements
         */
        std::unique_ptr<internal::RenderGroupBindingElementsImpl> m_impl;
    };
}
