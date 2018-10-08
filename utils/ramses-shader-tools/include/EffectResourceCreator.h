//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADER_TOOLS_EFFECTRESOURCECREATOR_H
#define RAMSES_SHADER_TOOLS_EFFECTRESOURCECREATOR_H

#include "Collections/String.h"

namespace ramses
{
    class EffectDescription;
    class Effect;
}

class RamsesEffectFromGLSLShaderArguments;

class EffectResourceCreator
{
public:
    static bool Create(const RamsesEffectFromGLSLShaderArguments& arguments);

private:
    static void SetEffectDescription(const RamsesEffectFromGLSLShaderArguments& arguments, ramses::EffectDescription& effectDesc);
    static void WriteoutRamsesEffectHash(const RamsesEffectFromGLSLShaderArguments& arguments, ramses::Effect& effect);
};

#endif
