//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "BoundingSphereCollectionImpl.h"

#include "NodeImpl.h"
#include "SceneImpl.h"
#include "Scene/ClientScene.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Math3d/Matrix44f.h"
#include "Math3d/Vector4.h"
#include "ArrayResourceImpl.h"
#include "ClientObjectImpl.h"
#include "RamsesObjectImpl.h"
#include "RamsesClientImpl.h"
#include "Resource/ArrayResource.h"

namespace ramses
{
    BoundingSphereCollectionImpl::BoundingSphereCollectionImpl(const SceneImpl& scene)
        : m_scene(scene)
    {
        UNUSED(m_scene);
    }

    void BoundingSphereCollectionImpl::setBoundingSphere(const NodeImpl& node, const BoundingSphere sphere)
    {
        assert(&node.getSceneImpl() == &m_scene);
        m_data.put(&node, sphere);
    }

    void BoundingSphereCollectionImpl::removeBoundingSphere(const NodeImpl& node)
    {
        assert(&node.getSceneImpl() == &m_scene);
        m_data.remove(&node);
    }

    BoundingSphere BoundingSphereCollectionImpl::computeBoundingSphereInWorldSpace(const NodeImpl& startNode) const
    {
        assert(&startNode.getSceneImpl() == &m_scene);

        // Collection of found bounding spheres, which needs to be combined to a final enclosing sphere
        BoundingSphereVector foundSpheres;

        std::vector<const NodeImpl*> stack;
        stack.push_back(&startNode);

        // Iterative implementation of depth-first search
        while (!stack.empty())
        {
            const NodeImpl* current = stack.back();
            stack.pop_back();

            // This will stop the DFS from going deeper
            if (!stopTraversal(foundSpheres, current))
            {
                const uint32_t childCount = current->getChildCount();

                for (uint32_t i = 0; i < childCount; i++)
                {
                    const NodeImpl* item = &current->getChildImpl(i);
                    stack.push_back(item);
                }
            }
        }

        return ComputeEnclosingSphere(foundSpheres);
    }

    // Compute a new (minimum) bounding sphere with contains all the given spheres.
    BoundingSphere BoundingSphereCollectionImpl::ComputeEnclosingSphere(const BoundingSphereVector& spheres)
    {
        if (spheres.empty())
        {
            BoundingSphere defaultSphere;
            defaultSphere.xPos = 0;
            defaultSphere.yPos = 0;
            defaultSphere.zPos = 0;
            defaultSphere.radius = 0;

            return defaultSphere;
        }

        if (spheres.size() == 1u)
        {
            return spheres[0];
        }

        ramses_internal::Vector3 min(std::numeric_limits<float>::max());
        ramses_internal::Vector3 max(std::numeric_limits<float>::min());

        // Find bounding box which covers all the bounding spheres
        for (auto it = spheres.begin(); it != spheres.end(); ++it)
        {
            ramses_internal::Vector3 spherePos((*it).xPos, (*it).yPos, (*it).zPos);
            float radius = (*it).radius;

            min.x = ramses_internal::min(min.x, spherePos.x - radius);
            min.y = ramses_internal::min(min.y, spherePos.y - radius);
            min.z = ramses_internal::min(min.z, spherePos.z - radius);

            max.x = ramses_internal::max(max.x, spherePos.x + radius);
            max.y = ramses_internal::max(max.y, spherePos.y + radius);
            max.z = ramses_internal::max(max.z, spherePos.z + radius);
        }

        BoundingSphere result;
        // Calculate center
        result.xPos = (max.x + min.x) / 2.f;
        result.yPos = (max.y + min.y) / 2.f;
        result.zPos = (max.z + min.z) / 2.f;

        // Take the largest distance and calculate the diagonal, which will result in the outer enclosing sphere for the box
        const float largest = ramses_internal::max(max.z - min.z, ramses_internal::max(max.x - min.x, max.y - min.y)) / 2.f;
        const float diagonal = largest * 1.4142135623730950488016887242097f; // sqrt(x^2 + x^2) => sqrt(2 * x^2) => sqrt(2) * x => 1.41.. * x
        result.radius = diagonal;

        return result;
    }

    bool BoundingSphereCollectionImpl::stopTraversal(BoundingSphereVector& spheres, const NodeImpl* node) const
    {
        // The traversal is finished if a non-visible node is encountered
        if (!node->getVisibility())
            return true;

        // ... or if a bounding sphere exists for the node, we are also finished
        auto it = m_data.find(node);

        if (it != m_data.end())
        {
            const ramses_internal::Matrix44f worldMatrix = node->getIScene().updateMatrixCache(ramses_internal::ETransformationMatrixType_World, node->getNodeHandle());

            // Using Vector4 allows us to use the Matrix transform directly
            const ramses_internal::Vector4 center(it->value.xPos, it->value.yPos, it->value.zPos, 1.f);
            const ramses_internal::Vector4 newCenter = worldMatrix * center;

            // Transform a point on the sphere, which allows us to find the new radius. This assumes that scaling is uniform.
            const ramses_internal::Vector4 pointOnSphere(center.x + it->value.radius, center.y, center.z, 1.f);
            const ramses_internal::Vector4 newPointOnSphere = worldMatrix * pointOnSphere;

            const float newRadius = (newCenter - newPointOnSphere).length();

            BoundingSphere transformedSphere;
            transformedSphere.xPos = newCenter.x;
            transformedSphere.yPos = newCenter.y;
            transformedSphere.zPos = newCenter.z;
            transformedSphere.radius = newRadius;

            spheres.push_back(transformedSphere);
            return true;
        }

        return false;
    }

    BoundingSphere BoundingSphereCollectionImpl::ComputeBoundingSphereForVertices(const ArrayResourceImpl& vertices)
    {
        assert(vertices.getElementType() == ramses_internal::EDataType_Vector3F);

        const uint32_t vertexCount = vertices.getElementCount();
        const ramses_internal::ManagedResource managedResource = GetInternalManagedResource(vertices);
        const bool failedToLoad = (NULL == managedResource.getResourceObject());

        if (failedToLoad)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "BoundingSphereCollectionImpl::GetInternalVertexResource failed, internal vertex data could not be extracted from input resource array");
        }

        if (vertexCount == 0 || failedToLoad)
        {
            BoundingSphere defaultSphere;
            defaultSphere.xPos = 0;
            defaultSphere.yPos = 0;
            defaultSphere.zPos = 0;
            defaultSphere.radius = 0;

            return defaultSphere;
        }

        const ramses_internal::ArrayResource* vertexResource = managedResource.getResourceObject()->convertTo<ramses_internal::ArrayResource>();
        // make sure resource is uncompressed
        vertexResource->decompress();

        ramses_internal::Vector3 min(std::numeric_limits<float>::max());
        ramses_internal::Vector3 max(std::numeric_limits<float>::min());

        const float* rawPtr = static_cast<const float*>(vertexResource->getData());

        // Find bounding box for all the points
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            const ramses_internal::Vector3 pos = GetVertexFromFloatArray(rawPtr, i);

            min.x = ramses_internal::min(min.x, pos.x);
            min.y = ramses_internal::min(min.y, pos.y);
            min.z = ramses_internal::min(min.z, pos.z);

            max.x = ramses_internal::max(max.x, pos.x);
            max.y = ramses_internal::max(max.y, pos.y);
            max.z = ramses_internal::max(max.z, pos.z);
        }

        // Find center of bounding box
        const ramses_internal::Vector3 center = (min + max) * 0.5f;

        float radius = 0.f;

        // Find distance to point furthest away from the center
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            const ramses_internal::Vector3 pos = GetVertexFromFloatArray(rawPtr, i);
            radius = ramses_internal::max(radius, (pos - center).length());
        }

        BoundingSphere result;
        result.xPos = center.x;
        result.yPos = center.y;
        result.zPos = center.z;
        result.radius = radius;

        return result;
    }

    ramses_internal::ManagedResource BoundingSphereCollectionImpl::GetInternalManagedResource(const ArrayResourceImpl& input)
    {
        const RamsesClientImpl& ramsesClientImpl = input.getClientImpl();
        const ramses_internal::ResourceContentHash resourceHash = input.getLowlevelResourceHash();
        ramses_internal::ManagedResource managedResource = ramsesClientImpl.getResource(resourceHash);

        if (!managedResource.getResourceObject())
        {
            managedResource = ramsesClientImpl.getClientApplication().forceLoadResource(resourceHash);
        }

        return managedResource;
    }

    ramses_internal::Vector3 BoundingSphereCollectionImpl::GetVertexFromFloatArray(const float* data, uint32_t index)
    {
        const uint32_t offset = index * 3;

        const float x = data[offset + 0];
        const float y = data[offset + 1];
        const float z = data[offset + 2];

        return ramses_internal::Vector3(x, y, z);
    }
}
