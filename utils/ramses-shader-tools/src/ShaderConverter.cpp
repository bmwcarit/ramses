//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ShaderConverter.h"
#include "EffectConfig.h"
#include "ConsoleUtils.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Effect.h"
#include "Resource/EffectResource.h"
#include "EffectImpl.h"
#include "RamsesClientImpl.h"

bool ShaderConverter::Convert(const RamsesShaderFromGLSLShaderArguments& arguments)
{
    ramses::RamsesFramework framework;
    ramses::RamsesClient ramsesClient("ramses client", framework);

    ramses::EffectDescription effectDesc;
    SetEffectDescription(arguments, effectDesc);

    ramses::Effect* effect = ramsesClient.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, arguments.getOutEffectName().c_str());
    if (!effect)
    {
        PRINT_ERROR("ramses can not create effect from given files.\n");
        return false;
    }

    WriteoutRamsesEffectHash(arguments, *effect);
    WriteoutRamsesShaders(arguments, ramsesClient, *effect);
    return true;
}

void ShaderConverter::SetEffectDescription(const RamsesShaderFromGLSLShaderArguments& arguments, ramses::EffectDescription& effectDesc)
{
    effectDesc.setVertexShaderFromFile(arguments.getInVertexShader().c_str());
    effectDesc.setFragmentShaderFromFile(arguments.getInFragmentShader().c_str());
    arguments.getInEffectConfig().fillEffectDescription(effectDesc);
}

void ShaderConverter::WriteoutRamsesEffectHash(const RamsesShaderFromGLSLShaderArguments& arguments, ramses::Effect& effect)
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

void ShaderConverter::WriteoutRamsesShaders(const RamsesShaderFromGLSLShaderArguments& arguments, ramses::RamsesClient& ramsesClient, ramses::Effect& effect)
{
    const ramses_internal::EffectResource* effectData = ramsesClient.impl.getResourceData<ramses_internal::EffectResource>(effect.impl.getLowlevelResourceHash());
    assert(effectData != NULL);
    effectData->decompress();

    const ramses_internal::String outVertexShader(effectData->getVertexShader());
    FileUtils::WriteTextToFile(outVertexShader, arguments.getOutVertexShader().c_str());

    const ramses_internal::String outFragmentShader(effectData->getFragmentShader());
    FileUtils::WriteTextToFile(outFragmentShader, arguments.getOutFragmentShader().c_str());
}
