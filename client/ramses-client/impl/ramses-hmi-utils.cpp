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
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/SceneGraphIterator.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/ResourceDataPool.h"

// internal
#include "SceneImpl.h"
#include "RamsesClientImpl.h"
#include "MeshNodeImpl.h"
#include "Scene/SceneResourceUtils.h"
#include "Scene/ClientScene.h"
#include "RamsesClientImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "ResourceImpl.h"
#include "SceneDumper.h"
#include <fstream>

namespace ramses
{
    void RamsesHMIUtils::DumpUnrequiredSceneObjects(const Scene& scene)
    {
        LOG_INFO_F(ramses_internal::CONTEXT_CLIENT, [&] (ramses_internal::StringOutputStream& output) {SceneDumper sceneDumper(scene.impl); sceneDumper.dumpUnrequiredObjects(output); });
    }

    void RamsesHMIUtils::DumpUnrequiredSceneObjectsToFile(const Scene& scene, std::ostream& out)
    {
        ramses_internal::StringOutputStream output;
        SceneDumper sceneDumper(scene.impl);
        sceneDumper.dumpUnrequiredObjects(output);
        out << output.release().c_str();
    }

    bool RamsesHMIUtils::AllResourcesForSceneKnown(const Scene& scene)
    {
        SceneObjectIterator iter(scene, ERamsesObjectType_Resource);
        ramses_internal::HashSet<ramses_internal::ResourceContentHash> knownHashes;
        auto obj = iter.getNext();
        while (obj)
        {
            const ramses::Resource* res = &ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Resource>(*obj);
            knownHashes.put(res->impl.getLowlevelResourceHash());
            obj = iter.getNext();
        }

        ramses_internal::ResourceContentHashVector resourcesUsedInScene;
        ramses_internal::SceneResourceUtils::GetAllClientResourcesFromScene(resourcesUsedInScene, scene.impl.getIScene());
        for (const auto hash : resourcesUsedInScene)
        {
            if (!knownHashes.contains(hash))
            {
                return false;
            }
        }
        return true;
    }

    ResourceDataPool& RamsesHMIUtils::GetResourceDataPoolForClient(RamsesClient& client)
    {
        return client.impl.getResourceDataPool();
    }

    bool RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(Scene const& scene, std::string const& filename, bool compress)
    {
        return scene.impl.saveResources(filename, compress);
    }
}
