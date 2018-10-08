//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesEffectFromGLSLShaderArguments.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "ConsoleUtils.h"
#include "FileUtils.h"

namespace
{
    const char* IN_VERTEX_SHADER_NAME = "in-vertex-shader";
    const char* IN_VERTEX_SHADER_SHORT_NAME = "iv";

    const char* IN_FRAGMENT_SHADER_NAME = "in-fragment-shader";
    const char* IN_FRAGMENT_SHADER_SHORT_NAME = "if";

    const char* IN_CONFIG_NAME = "in-config";
    const char* IN_CONFIG_SHORT_NAME = "ic";

    const char* OUT_EFFECT_NAME = "out-effect-name";
    const char* OUT_EFFECT_NAME_SHORT_NAME = "on";

    const char* OUT_RESOURCE_FILE_NAME = "out-resource-file";
    const char* OUT_RESOURCE_FILE_SHORT_NAME = "or";

    const char* OUT_EFFECT_ID_TYPE_NAME = "out-effect-id-type";
    const char* OUT_EFFECT_ID_TYPE_SHORT_NAME = "ot";

    const char* OUT_EFFECT_ID_TYPE_CLIENT = "client";
    const char* OUT_EFFECT_ID_TYPE_RENDERER = "renderer";

    const char* OUT_EFFECT_ID_NAME = "out-effect-id";
    const char* OUT_EFFECT_ID_SHORT_NAME = "oe";

    const char* OUT_COMPRESSION = "out-compression";
    const char* OUT_COMPRESSION_SHORT = "oc";
}

RamsesEffectFromGLSLShaderArguments::RamsesEffectFromGLSLShaderArguments()
    : m_outEffectIdType(EEffectIdType_Client)
{

}

bool RamsesEffectFromGLSLShaderArguments::parseArguments(int argc, char const*const* argv)
{
    ramses_internal::CommandLineParser parser(argc, argv);
    if (!LoadMandatoryExistingFileArgument(parser, IN_VERTEX_SHADER_NAME, IN_VERTEX_SHADER_SHORT_NAME, m_inVertexShader))
    {
        return false;
    }

    if (!LoadMandatoryExistingFileArgument(parser, IN_FRAGMENT_SHADER_NAME, IN_FRAGMENT_SHADER_SHORT_NAME, m_inFragmentShader))
    {
        return false;
    }

    if (!LoadMandatoryEffectConfig(parser, IN_CONFIG_NAME, IN_CONFIG_SHORT_NAME, m_inEffectConfig))
    {
        return false;
    }

    if (!LoadMandatoryArgument(parser, OUT_RESOURCE_FILE_NAME, OUT_RESOURCE_FILE_SHORT_NAME, m_outResourceFile))
    {
        return false;
    }

    if (!LoadOptionalArgument(parser, OUT_EFFECT_NAME, OUT_EFFECT_NAME_SHORT_NAME, m_outEffectName))
    {
        m_outEffectName = GetEffectNameFromShaderPaths(m_inVertexShader, m_inFragmentShader);
    }

    if (LoadOptionalArgument(parser, OUT_EFFECT_ID_NAME, OUT_EFFECT_ID_SHORT_NAME, m_outEffectIDFile))
    {
        if (!loadEffectIdType(parser))
        {
            return false;
        }
    }

    m_outCompression = ramses_internal::ArgumentBool(parser, OUT_COMPRESSION_SHORT, OUT_COMPRESSION, false);

    return true;
}

const EffectConfig& RamsesEffectFromGLSLShaderArguments::getInEffectConfig() const
{
    return m_inEffectConfig;
}

const ramses_internal::String& RamsesEffectFromGLSLShaderArguments::getInVertexShader() const
{
    return m_inVertexShader;
}

const ramses_internal::String& RamsesEffectFromGLSLShaderArguments::getInFragmentShader() const
{
    return m_inFragmentShader;
}

const ramses_internal::String& RamsesEffectFromGLSLShaderArguments::getOutResourceFile() const
{
    return m_outResourceFile;
}

const ramses_internal::String& RamsesEffectFromGLSLShaderArguments::getOutEffectName() const
{
    return m_outEffectName;
}

EEffectIdType RamsesEffectFromGLSLShaderArguments::getOutEffectIDType() const
{
    return m_outEffectIdType;
}

const ramses_internal::String& RamsesEffectFromGLSLShaderArguments::getOutEffectIDFile() const
{
    return m_outEffectIDFile;
}

bool RamsesEffectFromGLSLShaderArguments::getUseCompression() const
{
    return m_outCompression;
}


void RamsesEffectFromGLSLShaderArguments::printUsage() const
{
    PRINT_HINT( "usage: program\n"
                "--%s (-%s) <filename>\n"
                "--%s (-%s) <filename>\n"
                "--%s (-%s) <filename>\n"
                "--%s (-%s) <filename>\n"
                "--%s (-%s) <optional string>\n"
                "--%s (-%s) <optional filename>\n"
                "--%s (-%s) {optional: %s|%s}\n"
                "--%s (-%s) {optional}\n\n",
                IN_VERTEX_SHADER_NAME, IN_VERTEX_SHADER_SHORT_NAME,
                IN_FRAGMENT_SHADER_NAME, IN_FRAGMENT_SHADER_SHORT_NAME,
                IN_CONFIG_NAME, IN_CONFIG_SHORT_NAME,
                OUT_RESOURCE_FILE_NAME, OUT_RESOURCE_FILE_SHORT_NAME,
                OUT_EFFECT_NAME, OUT_EFFECT_NAME_SHORT_NAME,
                OUT_EFFECT_ID_NAME, OUT_EFFECT_ID_SHORT_NAME,
                OUT_EFFECT_ID_TYPE_NAME, OUT_EFFECT_ID_TYPE_SHORT_NAME, OUT_EFFECT_ID_TYPE_CLIENT, OUT_EFFECT_ID_TYPE_RENDERER,
                OUT_COMPRESSION, OUT_COMPRESSION_SHORT);
}

ramses_internal::String RamsesEffectFromGLSLShaderArguments::GetEffectNameFromShaderPaths(const ramses_internal::String& vertexShaderPath, const ramses_internal::String& fragmentShaderPath)
{
    const ramses_internal::String vertexShaderFileName = FileUtils::GetFileName(vertexShaderPath.c_str());
    const ramses_internal::String fragmentShaderFileName = FileUtils::GetFileName(fragmentShaderPath.c_str());
    return vertexShaderFileName + ramses_internal::String("_") + fragmentShaderFileName;
}


bool RamsesEffectFromGLSLShaderArguments::loadEffectIdType(const ramses_internal::CommandLineParser& parser)
{
    ramses_internal::String inEffectIdTypeStr;
    if (!LoadMandatoryArgument(parser, OUT_EFFECT_ID_TYPE_NAME, OUT_EFFECT_ID_TYPE_SHORT_NAME, inEffectIdTypeStr))
    {
        return false;
    }

    if (inEffectIdTypeStr == ramses_internal::String(OUT_EFFECT_ID_TYPE_CLIENT))
    {
        m_outEffectIdType = EEffectIdType_Client;
    }
    else if (inEffectIdTypeStr == ramses_internal::String(OUT_EFFECT_ID_TYPE_RENDERER))
    {
        m_outEffectIdType = EEffectIdType_Renderer;
    }
    else
    {
        PRINT_HINT("valid Effect Id Type Options are as following: %s %s\n", OUT_EFFECT_ID_TYPE_CLIENT, OUT_EFFECT_ID_TYPE_RENDERER);
        return false;
    }
    return true;
}
