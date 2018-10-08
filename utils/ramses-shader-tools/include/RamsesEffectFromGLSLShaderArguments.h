//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADER_TOOLS_RAMSESEFFECTFROMGLSLSHADERARGUMENTS_H
#define RAMSES_SHADER_TOOLS_RAMSESEFFECTFROMGLSLSHADERARGUMENTS_H

#include "EffectConfig.h"
#include "Arguments.h"

namespace ramses_internal
{
    class CommandLineParser;
}

class RamsesEffectFromGLSLShaderArguments : public Arguments
{
public:
    RamsesEffectFromGLSLShaderArguments();
    virtual bool parseArguments(int argc, char const*const* argv) override;

    const EffectConfig& getInEffectConfig() const;
    const ramses_internal::String& getInVertexShader() const;
    const ramses_internal::String& getInFragmentShader() const;

    const ramses_internal::String& getOutEffectName() const;
    const ramses_internal::String& getOutResourceFile() const;

    EEffectIdType getOutEffectIDType() const;
    const ramses_internal::String& getOutEffectIDFile() const;

    bool getUseCompression() const;

    virtual void printUsage() const override;

private:
    bool loadEffectIdType(const ramses_internal::CommandLineParser& parser);
    static ramses_internal::String GetEffectNameFromShaderPaths(const ramses_internal::String& vertexShaderPath, const ramses_internal::String& fragmentShaderPath);

    ramses_internal::String m_inVertexShader;
    ramses_internal::String m_inFragmentShader;
    ramses_internal::String m_outEffectName;
    ramses_internal::String m_outResourceFile;
    ramses_internal::String m_outEffectIDFile;
    EEffectIdType m_outEffectIdType;
    EffectConfig m_inEffectConfig;
    bool m_outCompression;
};

#endif
