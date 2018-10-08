//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CUSTOM_SHADER_TO_RAMSES_SHADER_SHADERCONVERTER_H
#define RAMSES_CUSTOM_SHADER_TO_RAMSES_SHADER_SHADERCONVERTER_H

#include "FileUtils.h"
#include "RamsesShaderFromGLSLShaderArguments.h"

namespace ramses
{
    class Effect;
    class EffectDescription;
    class RamsesClient;
}

class ShaderConverter
{
public:
    static bool Convert(const RamsesShaderFromGLSLShaderArguments& arguments);

private:
    static void WriteoutRamsesShaders(const RamsesShaderFromGLSLShaderArguments& arguments, ramses::RamsesClient& ramsesClient, ramses::Effect& effect);
    static void WriteoutRamsesEffectHash(const RamsesShaderFromGLSLShaderArguments& arguments, ramses::Effect& effect);
    static void SetEffectDescription(const RamsesShaderFromGLSLShaderArguments& arguments, ramses::EffectDescription& effectDesc);
};

#endif
