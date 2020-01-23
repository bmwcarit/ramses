//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectResourceCreator.h"
#include "ConsoleUtils.h"
#include "RamsesEffectFromGLSLShaderArguments.h"

#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/ResourceFileDescription.h"

#include "RamsesClientImpl.h"
#include "ResourceFileDescriptionImpl.h"
#include "ramses-client-api/Effect.h"
#include "EffectImpl.h"
#include "FileUtils.h"

bool EffectResourceCreator::Create(const RamsesEffectFromGLSLShaderArguments& arguments)
{
    ramses::RamsesFramework framework;
    ramses::RamsesClient* ramsesClient(framework.createClient("ramses client"));
    if (!ramsesClient)
    {
        PRINT_ERROR("Failed to create ramses client.\n");
        return false;
    }

    ramses::EffectDescription effectDesc;
    SetEffectDescription(arguments, effectDesc);

    const ramses_internal::String& effectName = arguments.getOutEffectName();
    ramses::Effect* effect = ramsesClient->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, effectName.c_str());
    if (!effect)
    {
        PRINT_ERROR("ramses can not create effect from given files.\n");
        return false;
    }

    ramses::ResourceFileDescription resourceFileDesc(arguments.getOutResourceFile().c_str());
    resourceFileDesc.impl->m_resources.push_back(effect);

    ramses::status_t status = ramsesClient->impl.saveResources(resourceFileDesc, arguments.getUseCompression());
    if (ramses::StatusOK != status)
    {
        PRINT_ERROR("ramses fails to save effect to resource file.\n");
        return false;
    }

    if (arguments.getOutEffectIDFile().size() > 0)
    {
        WriteoutRamsesEffectHash(arguments, *effect);
    }

    return true;
}

void EffectResourceCreator::SetEffectDescription(const RamsesEffectFromGLSLShaderArguments& arguments, ramses::EffectDescription& effectDesc)
{
    effectDesc.setVertexShaderFromFile(arguments.getInVertexShader().c_str());
    effectDesc.setFragmentShaderFromFile(arguments.getInFragmentShader().c_str());
    arguments.getInEffectConfig().fillEffectDescription(effectDesc);
}

void EffectResourceCreator::WriteoutRamsesEffectHash(const RamsesEffectFromGLSLShaderArguments& arguments, ramses::Effect& effect)
{
    if (arguments.getOutEffectIDType() == EEffectIdType_Renderer)
    {
        const ramses_internal::ResourceContentHash hash = effect.impl.getLowlevelResourceHash();
        FileUtils::WriteHashToFile(hash, arguments.getOutEffectIDFile().c_str());
    }
    else if (arguments.getOutEffectIDType() == EEffectIdType_Client)
    {
        const auto effectId = effect.impl.getResourceId();
        const ramses_internal::ResourceContentHash hash = { effectId.lowPart, effectId.highPart };
        FileUtils::WriteHashToFile(hash, arguments.getOutEffectIDFile().c_str());
    }
}
