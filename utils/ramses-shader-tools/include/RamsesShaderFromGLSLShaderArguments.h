//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADER_TOOL_RAMSESSHADERFROMGLSLSHADERARGUMENTS_H
#define RAMSES_SHADER_TOOL_RAMSESSHADERFROMGLSLSHADERARGUMENTS_H

#include "EffectConfig.h"
#include "Arguments.h"

namespace ramses_internal
{
    class CommandLineParser;
}

class RamsesShaderFromGLSLShaderArguments : public Arguments
{
public:
    RamsesShaderFromGLSLShaderArguments();
    virtual bool parseArguments(int argc, char const*const* argv) override;

    const EffectConfig& getInEffectConfig() const;
    const ramses_internal::String& getInVertexShader() const;
    const ramses_internal::String& getInFragmentShader() const;

    const ramses_internal::String& getOutVertexShader() const;
    const ramses_internal::String& getOutFragmentShader() const;
    EEffectIdType getOutEffectIDType() const;
    const ramses_internal::String& getOutEffectIDFile() const;
    const ramses_internal::String& getOutEffectName() const;

    virtual void printUsage() const override;

private:
    bool loadEffectIdType(const ramses_internal::CommandLineParser& parser);

    ramses_internal::String m_inVertexShader;
    ramses_internal::String m_inFragmentShader;
    ramses_internal::String m_outVertexShader;
    ramses_internal::String m_outFragmentShader;
    ramses_internal::String m_outEffectIDFile;
    EffectConfig m_inEffectConfig;
    EEffectIdType m_outEffectIdType;
    ramses_internal::String m_outEffectName;
};

#endif
