//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
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
        class MeshNodeImpl;
        template <typename T>
        class IteratorImpl;
    }

    class RenderGroup;
    class MeshNode;

    /**
    * @ingroup CoreAPI
    * @brief The RenderGroupMeshIterator traverses MeshNodes in a RenderGroup.
    */
    class RAMSES_API RenderGroupMeshIterator
    {
    public:
        /**
        * @brief RenderGroupMeshIterator constructor.
        *
        * @param[in] renderGroup RenderGroup whose MeshNodes to iterate through
        **/
        explicit RenderGroupMeshIterator(const RenderGroup& renderGroup);

        /**
        * @brief Destructor
        **/
        ~RenderGroupMeshIterator();

        /**
        * @brief Iterate through all MeshNodes.
        *        MeshNodes will be iterated over in order of adding, not in render order.
        *        Render order of a MeshNode within RenderGroup can be retrieved from its RenderGroup.
        * @return Next MeshNode, null if no more MeshNodes available
        *
        * Iterator is invalid and may no longer be used if any objects are added or removed.
        **/
        const MeshNode* getNext();

    private:
        std::unique_ptr<internal::IteratorImpl<const internal::MeshNodeImpl*>> m_impl;
    };
}
