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

namespace ramses
{
    namespace internal
    {
        class RenderGroupImpl;
        template <typename T>
        class IteratorImpl;
    }

    class RenderPass;
    class RenderGroup;

    /**
    * @brief The RenderPassGroupIterator traverses RenderGroups in a RenderPass.
    * @ingroup CoreAPI
    */
    class RAMSES_API RenderPassGroupIterator
    {
    public:
        /**
        * @brief RenderPassGroupIterator constructor.
        *
        * @param[in] renderPass RenderPass whose RenderGroups to iterate through
        **/
        explicit RenderPassGroupIterator(const RenderPass& renderPass);

        /**
        * @brief Destructor
        **/
        ~RenderPassGroupIterator();

        /**
        * @brief Iterate through all RenderGroups.
        *        RenderGroups will be iterated over in order of adding, not in render order.
        *        Render order of a RenderGroup within RenderPass can be retrieved from its RenderPass.
        * @return Next RenderGroup, null if no more RenderGroups available
        *
        * Iterator is invalid and may no longer be used if any objects are added or removed.
        **/
        const RenderGroup* getNext();

    private:
        std::unique_ptr<internal::IteratorImpl<const internal::RenderGroupImpl*>> m_impl;
    };
}
