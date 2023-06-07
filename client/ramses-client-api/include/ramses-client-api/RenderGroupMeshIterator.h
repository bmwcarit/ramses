//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERGROUPMESHITERATOR_H
#define RAMSES_RENDERGROUPMESHITERATOR_H

#include "ramses-framework-api/APIExport.h"
#include <memory>

namespace ramses
{
    class RenderGroup;
    class MeshNode;
    class MeshNodeImpl;
    template <typename T>
    class IteratorImpl;

    /**
    * @ingroup CoreAPI
    * @brief The RenderGroupMeshIterator traverses MeshNodes in a RenderGroup.
    */
    class RenderGroupMeshIterator
    {
    public:
        /**
        * @brief RenderGroupMeshIterator constructor.
        *
        * @param[in] renderGroup RenderGroup whose MeshNodes to iterate through
        **/
        RAMSES_API explicit RenderGroupMeshIterator(const RenderGroup& renderGroup);

        /**
        * @brief Destructor
        **/
        RAMSES_API ~RenderGroupMeshIterator();

        /**
        * @brief Iterate through all MeshNodes.
        *        MeshNodes will be iterated over in order of adding, not in render order.
        *        Render order of a MeshNode within RenderGroup can be retrieved from its RenderGroup.
        * @return Next MeshNode, null if no more MeshNodes available
        *
        * Iterator is invalid and may no longer be used if any objects are added or removed.
        **/
        RAMSES_API const MeshNode* getNext();

    private:
        std::unique_ptr<IteratorImpl<const MeshNodeImpl*>> m_impl;
    };
}

#endif
