//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BOUNDINGSPHERECOLLECTIONIMPL_H
#define RAMSES_BOUNDINGSPHERECOLLECTIONIMPL_H

#include "ramses-hmi-utils.h"
#include "Collections/HashMap.h"
#include "Collections/Vector.h"
#include "Math3d/Vector3.h"

namespace ramses_internal
{
    class ManagedResource;
}

namespace ramses
{
    class SceneImpl;
    class NodeImpl;
    class ArrayResourceImpl;

    typedef std::vector<BoundingSphere> BoundingSphereVector;
    typedef ramses_internal::HashMap<const NodeImpl*, BoundingSphere> NodeImplBoundingSphereMap;

    class BoundingSphereCollectionImpl
    {
    public:

        BoundingSphereCollectionImpl(const SceneImpl& scene);

        void setBoundingSphere(const NodeImpl& node, const BoundingSphere sphere);
        void removeBoundingSphere(const NodeImpl& node);
        BoundingSphere computeBoundingSphereInWorldSpace(const NodeImpl& startNode) const;

        static BoundingSphere ComputeBoundingSphereForVertices(const ArrayResourceImpl& vertices);

    private:

        static BoundingSphere ComputeEnclosingSphere(const BoundingSphereVector& spheres);
        bool stopTraversal(BoundingSphereVector& spheres, const ramses::NodeImpl* node) const;
        static ramses_internal::ManagedResource GetInternalManagedResource(const ArrayResourceImpl& input);
        static ramses_internal::Vector3 GetVertexFromFloatArray(const float* data, uint32_t index);

        const SceneImpl& m_scene; // Only used for asserting that the NodeImpls* belong to this scene
        NodeImplBoundingSphereMap m_data;
    };
}

#endif
