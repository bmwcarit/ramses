//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERPASSGROUPITERATOR_H
#define RAMSES_RENDERPASSGROUPITERATOR_H

#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    class RenderPass;
    class RenderGroup;
    class RenderGroupImpl;
    template <typename T>
    class IteratorImpl;

    /**
    * @brief The RenderPassGroupIterator traverses RenderGroups in a RenderPass.
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
        RenderPassGroupIterator(const RenderPassGroupIterator& iterator);
        RenderPassGroupIterator& operator=(const RenderPassGroupIterator& iterator);
        IteratorImpl<const RenderGroupImpl*>* impl;
    };
}

#endif
