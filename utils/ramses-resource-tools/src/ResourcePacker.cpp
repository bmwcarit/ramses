//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesResourcePackerArguments.h"
#include "ResourcePacker.h"
#include "ConsoleUtils.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ResourceDataPool.h"
#include "ramses-client-api/Resource.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesObjectTypeUtils.h"
#include "ramses-hmi-utils.h"
#include "ResourceDataPoolImpl.h"
#include "RamsesClientImpl.h"

bool ResourcePacker::Pack(const RamsesResourcePackerArguments& arguments)
{
    const FilePaths& inputFiles = arguments.getInputResourceFiles().getFilePaths();
    if (0 == inputFiles.size())
    {
        return true;
    }

    ramses::RamsesFramework framework;
    ramses::RamsesClient* ramsesClient(framework.createClient("ramses client"));
    if (!ramsesClient)
    {
        PRINT_ERROR("failed to create ramses client.\n");
        return false;
    }

    ramses::Scene* scene(ramsesClient->createScene(ramses::sceneId_t{ 0xf00 }));
    if (!scene)
    {
        PRINT_ERROR("failed to create ramses scene.\n");
        return false;
    }

    for(const auto& file : inputFiles)
    {
        if (!ramses::RamsesHMIUtils::GetResourceDataPoolForClient(*ramsesClient).addResourceDataFile(file.c_str()))
        {
            PRINT_ERROR("ramses fail to load input resource file:\"{}\".\n", file);
            return false;
        }
    }

    std::vector<ramses::resourceId_t> resourceIDs;
    ramsesClient->impl.getResourceDataPool().impl.getAllResourceDataFileResourceIds(resourceIDs);

    ramses::RamsesObjectVector resourceObjects;
    for (auto const& id : resourceIDs)
    {
        if (auto res = ramses::RamsesHMIUtils::GetResourceDataPoolForClient(*ramsesClient).createResourceForScene(*scene, id))
            resourceObjects.push_back(res);
    }

    if (!ramses::RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(*scene, arguments.getOutputResourceFile().c_str(), arguments.getUseCompression()))
    {
        PRINT_ERROR("ramses fail to save to output resource file.\n");
        return false;
    }

    return true;
}
