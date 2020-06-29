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
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesClientImpl.h"
#include "ResourceFileDescriptionImpl.h"
#include "RamsesObjectTypeUtils.h"

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

    for(const auto& file : inputFiles)
    {
        ramses::ResourceFileDescription fileDescription(file.c_str());
        if (ramses::StatusOK != ramsesClient->loadResources(fileDescription))
        {
            PRINT_ERROR("ramses fail to load input resource file:\"{}\".\n", file);
            return false;
        }
    }

    ramses::ResourceFileDescription outputFile(arguments.getOutputResourceFile().c_str());
    const ramses::RamsesObjectVector resourceObjects = ramsesClient->impl.getListOfResourceObjects();
    for(const auto& resObj : resourceObjects)
    {
        const ramses::Resource& resource = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Resource>(*resObj);
        outputFile.impl->m_resources.push_back(&resource);
    }

    const ramses::status_t savingStatus = ramsesClient->saveResources(outputFile, arguments.getUseCompression());
    if (ramses::StatusOK != savingStatus)
    {
        PRINT_ERROR("ramses fail to save to output resource file.\n");
        return false;
    }

    return true;
}
