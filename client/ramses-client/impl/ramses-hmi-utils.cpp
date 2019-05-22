//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-hmi-utils.h"

// API
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/SceneGraphIterator.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Vector3fArray.h"

// internal
#include "SceneImpl.h"
#include "MeshNodeImpl.h"
#include "BoundingSphereCollectionImpl.h"
#include "Scene/SceneResourceUtils.h"
#include "Scene/ClientScene.h"
#include "RamsesClientImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "ResourceImpl.h"
#include "SceneDumper.h"
#include <fstream>

namespace ramses
{
    class Vector3fArray;

    void RamsesHMIUtils::DumpUnrequiredSceneObjects(const Scene& scene)
    {
        LOG_INFO_F(ramses_internal::CONTEXT_CLIENT, [&] (ramses_internal::StringOutputStream& output) {SceneDumper sceneDumper(scene.impl); sceneDumper.dumpUnrequiredObjects(output); });
    }

    void RamsesHMIUtils::DumpUnrequiredSceneObjectsToFile(const Scene& scene, std::ofstream& out)
    {
        ramses_internal::StringOutputStream output;
        SceneDumper sceneDumper(scene.impl);
        sceneDumper.dumpUnrequiredObjects(output);
        out << output.release().c_str();
    }


    BoundingSphereCollection::BoundingSphereCollection(const Scene& scene) :
        impl(*new BoundingSphereCollectionImpl(scene.impl))
    {
    }

    BoundingSphereCollection::~BoundingSphereCollection()
    {
        delete &impl;
    }

    void BoundingSphereCollection::setBoundingSphere(const Node& node, const BoundingSphere sphere)
    {
        impl.setBoundingSphere(node.impl, sphere);
    }

    void BoundingSphereCollection::removeBoundingSphere(const Node& node)
    {
        impl.removeBoundingSphere(node.impl);
    }

    BoundingSphere BoundingSphereCollection::computeBoundingSphereInWorldSpace(const Node& startNode) const
    {
        return impl.computeBoundingSphereInWorldSpace(startNode.impl);
    }

    BoundingSphere BoundingSphereCollection::ComputeBoundingSphereForVertices(const Vector3fArray& vertices)
    {
        return BoundingSphereCollectionImpl::ComputeBoundingSphereForVertices(vertices.impl);
    }

    bool RamsesHMIUtils::AllResourcesForSceneKnown(const Scene& scene)
    {
        const SceneImpl& sceneImpl = scene.impl;
        const RamsesClientImpl& clientImpl = sceneImpl.getClientImpl();

        ramses::RamsesObjectVector resObjects = clientImpl.getListOfResourceObjects();
        ramses_internal::HashSet<ramses_internal::ResourceContentHash> knownHashes;
        for (const auto& obj : resObjects)
        {
            const ramses::Resource* res = &ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Resource>(*obj);
            knownHashes.put(res->impl.getLowlevelResourceHash());
        }

        ramses_internal::ResourceContentHashVector resourcesUsedInScene;
        ramses_internal::SceneResourceUtils::GetAllClientResourcesFromScene(resourcesUsedInScene, scene.impl.getIScene());
        for (const auto hash : resourcesUsedInScene)
        {
            if (!knownHashes.hasElement(hash))
            {
                return false;
            }
        }
        return true;
    }

}
