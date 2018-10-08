//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesShaderFromGLSLShaderArguments.h"
#include "FileUtils.h"
#include "ConsoleUtils.h"

#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"

namespace
{
    const char* IN_VERTEX_SHADER_NAME = "in-vertex-shader";
    const char* IN_VERTEX_SHADER_SHORT_NAME = "iv";

    const char* IN_FRAGMENT_SHADER_NAME = "in-fragment-shader";
    const char* IN_FRAGMENT_SHADER_SHORT_NAME = "if";

    const char* IN_CONFIG_NAME = "in-config";
    const char* IN_CONFIG_SHORT_NAME = "ic";

    const char* OUT_VERTEX_SHADER_NAME = "out-vertex-shader";
    const char* OUT_VERTEX_SHADER_SHORT_NAME = "ov";

    const char* OUT_FRAGMENT_SHADER_NAME = "out-fragment-shader";
    const char* OUT_FRAGMENT_SHADER_SHORT_NAME = "of";

    const char* OUT_EFFECT_ID_TYPE_NAME = "out-effect-id-type";
    const char* OUT_EFFECT_ID_TYPE_SHORT_NAME = "ot";

    const char* OUT_EFFECT_ID_TYPE_CLIENT   = "client";
    const char* OUT_EFFECT_ID_TYPE_RENDERER = "renderer";

    const char* OUT_EFFECT_ID_NAME = "out-effect-id";
    const char* OUT_EFFECT_ID_SHORT_NAME = "oe";

    const char* OUT_EFFECT_NAME = "out-effect-name";
    const char* OUT_EFFECT_NAME_SHORT_NAME = "on";
}

RamsesShaderFromGLSLShaderArguments::RamsesShaderFromGLSLShaderArguments()
{

}

bool RamsesShaderFromGLSLShaderArguments::parseArguments(int argc, char const*const* argv)
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

    if (!LoadMandatoryArgument(parser, OUT_VERTEX_SHADER_NAME, OUT_VERTEX_SHADER_SHORT_NAME, m_outVertexShader))
    {
        return false;
    }

    if (!LoadMandatoryArgument(parser, OUT_FRAGMENT_SHADER_NAME, OUT_FRAGMENT_SHADER_SHORT_NAME, m_outFragmentShader))
    {
        return false;
    }

    if (!loadEffectIdType(parser))
    {
        return false;
    }

    if (!LoadMandatoryArgument(parser, OUT_EFFECT_ID_NAME, OUT_EFFECT_ID_SHORT_NAME, m_outEffectIDFile))
    {
        return false;
    }

    if (!LoadOptionalArgument(parser, OUT_EFFECT_NAME, OUT_EFFECT_NAME_SHORT_NAME, m_outEffectName))
    {
        m_outEffectName = "glsl shader";
    }

    return true;
}

const ramses_internal::String& RamsesShaderFromGLSLShaderArguments::getInVertexShader() const
{
    return m_inVertexShader;
}

const ramses_internal::String& RamsesShaderFromGLSLShaderArguments::getInFragmentShader() const
{
    return m_inFragmentShader;
}

const EffectConfig& RamsesShaderFromGLSLShaderArguments::getInEffectConfig() const
{
    return m_inEffectConfig;
}

const ramses_internal::String& RamsesShaderFromGLSLShaderArguments::getOutVertexShader() const
{
    return m_outVertexShader;
}

const ramses_internal::String& RamsesShaderFromGLSLShaderArguments::getOutFragmentShader() const
{
    return m_outFragmentShader;
}

EEffectIdType RamsesShaderFromGLSLShaderArguments::getOutEffectIDType() const
{
    return m_outEffectIdType;
}

const ramses_internal::String& RamsesShaderFromGLSLShaderArguments::getOutEffectIDFile() const
{
    return m_outEffectIDFile;
}

const ramses_internal::String& RamsesShaderFromGLSLShaderArguments::getOutEffectName() const
{
    return m_outEffectName;
}

void RamsesShaderFromGLSLShaderArguments::printUsage() const
{
    PRINT_HINT("usage: program --%s <filename> --%s <filename> --%s <filename> --%s <filename> --%s <filename> --%s {%s|%s} --%s <filename> --%s <optional string>\n",
        IN_VERTEX_SHADER_NAME, IN_FRAGMENT_SHADER_NAME, IN_CONFIG_NAME,
        OUT_VERTEX_SHADER_NAME, OUT_FRAGMENT_SHADER_NAME,
        OUT_EFFECT_ID_TYPE_NAME, OUT_EFFECT_ID_TYPE_CLIENT, OUT_EFFECT_ID_TYPE_RENDERER,
        OUT_EFFECT_ID_NAME,
        OUT_EFFECT_NAME);
    PRINT_HINT("usage: program -%s <filename> -%s <filename> -%s <filename> -%s <filename> -%s <filename> -%s {%s|%s} -%s <filename>  -%s <optional string>\n\n",
        IN_VERTEX_SHADER_SHORT_NAME, IN_FRAGMENT_SHADER_SHORT_NAME, IN_CONFIG_SHORT_NAME,
        OUT_VERTEX_SHADER_SHORT_NAME, OUT_FRAGMENT_SHADER_SHORT_NAME,
        OUT_EFFECT_ID_TYPE_SHORT_NAME, OUT_EFFECT_ID_TYPE_CLIENT, OUT_EFFECT_ID_TYPE_RENDERER,
        OUT_EFFECT_ID_SHORT_NAME,
        OUT_EFFECT_NAME_SHORT_NAME);
}

bool RamsesShaderFromGLSLShaderArguments::loadEffectIdType(const ramses_internal::CommandLineParser& parser)
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
