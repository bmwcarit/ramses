//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include <memory>
#include <string_view>

namespace ramses
{
    class RenderGroup;
    class MeshNode;
}

namespace ramses::internal
{
    class RamsesRenderGroupBindingElementsImpl;
}

namespace ramses
{
    /**
     * RamsesRenderGroupBindingElements is a helper class holding a set of references to elements to be provided when constructing #ramses::RamsesRenderGroupBinding.
     * These elements are either ramses::MeshNode or ramses::RenderGroup.
     * Note that ramses::RenderGroup can contain other (nested) ramses::RenderGroup objects, in such case the parent ramses::RenderGroup corresponds
     * to the #ramses::RamsesRenderGroupBinding to be created and the nested ramses::RenderGroup is the element provided here.
     */
    class RamsesRenderGroupBindingElements
    {
    public:
        /// Constructor of RamsesRenderGroupBindingElements.
        RAMSES_API RamsesRenderGroupBindingElements() noexcept;
        /// Destructor of RamsesRenderGroupBindingElements.
        RAMSES_API ~RamsesRenderGroupBindingElements() noexcept;
        /// Copy constructor
        RAMSES_API RamsesRenderGroupBindingElements(const RamsesRenderGroupBindingElements& other);
        /// Move constructor
        RAMSES_API RamsesRenderGroupBindingElements(RamsesRenderGroupBindingElements&& other) noexcept;
        /// Assignment operator
        RAMSES_API RamsesRenderGroupBindingElements& operator=(const RamsesRenderGroupBindingElements& other);
        /// Move assignment operator
        RAMSES_API RamsesRenderGroupBindingElements& operator=(RamsesRenderGroupBindingElements&& other) noexcept;

        /**
         * Add ramses::MeshNode element to control its render order when provided to #ramses::RamsesRenderGroupBinding.
         * Will fail if given element is already contained.
         *
         * @param meshNode ramses::MeshNode element to add to be exposed for render order control.
         * @param elementName This name will be used to name the input property in the created #ramses::RamsesRenderGroupBinding.
         *                    If none provided, name of the ramses::MeshNode will be used.
         * @return \c true if successful, \c false otherwise.
         */
        RAMSES_API bool addElement(const ramses::MeshNode& meshNode, std::string_view elementName = {});

        /**
         * Add ramses::RenderGroup element to control its render order when provided to #ramses::RamsesRenderGroupBinding.
         * Will fail if given element is already contained.
         *
         * @param nestedRenderGroup ramses::RenderGroup element to add to be exposed for render order control.
         * @param elementName This name will be used to name the input property in the created #ramses::RamsesRenderGroupBinding.
         *                    If none provided, name of the ramses::RenderGroup will be used.
         * @return \c true if successful, \c false otherwise.
         */
        RAMSES_API bool addElement(const ramses::RenderGroup& nestedRenderGroup, std::string_view elementName = {});

        /// Implementation detail of RamsesRenderGroupBindingElements
        std::unique_ptr<internal::RamsesRenderGroupBindingElementsImpl> m_impl;
    };
}
