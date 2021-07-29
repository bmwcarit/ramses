//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectConfig.h"
#include "FileUtils.h"
#include "ConsoleUtils.h"
#include "Utils/StringUtils.h"

#include "PlatformAbstraction/PlatformStringUtils.h"
#include "Utils/File.h"

namespace
{
    const char* const KEY_WORD_DEFINE = "DEFINE";
    const char* const KEY_WORD_UNIFORM_SEMANTIC = "UNIFORM_SEMANTIC";
    const char* const KEY_WORD_ATTRIBUTE_SEMANTIC = "ATTRIBUTE_SEMANTIC";
}

EffectConfig::EffectConfig()
{
    initUniformSemanticNameTable();
    initAttributeSemanticNameTable();
}

bool EffectConfig::loadFromFile(const char* filePath)
{
    clear();

    if (!FileUtils::FileExists(filePath))
    {
        return false;
    }

    std::vector<ramses_internal::String> lines;
    if (!FileUtils::ReadFileLines(filePath, lines))
    {
        return false;
    }

    bool succeed = true;
    int lineNumber = 1;
    for(const auto& line : lines)
    {
        if (!parseConfigLine(lineNumber++, line))
        {
            succeed = false;
        }
    }

    if (!succeed)
    {
        clear();
    }

    return succeed;
}

bool EffectConfig::parseConfigLine(int lineNumber, const ramses_internal::String& line)
{
    if (0 == line.size())
    {
        return true;
    }

    std::vector<ramses_internal::String> tokens = ramses_internal::StringUtils::TokenizeTrimmed(line, ' ');
    if (tokens.empty())
    {
        return true;
    }

    const ramses_internal::String& firstToken = tokens[0];
    if (firstToken == ramses_internal::String(KEY_WORD_DEFINE))
    {
        return parseDefines(lineNumber, line, tokens);
    }
    else if (firstToken == ramses_internal::String(KEY_WORD_UNIFORM_SEMANTIC))
    {
        return parseUniformSemantic(lineNumber, line, tokens);
    }
    else if (firstToken == ramses_internal::String(KEY_WORD_ATTRIBUTE_SEMANTIC))
    {
        return parseAttributeSemantic(lineNumber, line, tokens);
    }
    else
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("line beginning with invalid keyword.\n");
        PRINT_HINT("expected line beginning keywords are: {}, {}, {}\n\n", KEY_WORD_DEFINE, KEY_WORD_UNIFORM_SEMANTIC, KEY_WORD_ATTRIBUTE_SEMANTIC);
        return false;
    }
}

bool EffectConfig::parseUniformSemantic(int lineNumber, const ramses_internal::String& line, const Tokens& tokens)
{
    if (3 != tokens.size())
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("invalid config for Uniform Semantic Definition\n");
        PRINT_HINT("expected Uniform Semantic Definition: {} input_name EEffectUniformSemantic_XXX\n\n", KEY_WORD_UNIFORM_SEMANTIC);
        return false;
    }

    const ramses_internal::String& inputName = tokens[1];
    const ramses_internal::String& semanticStr = tokens[2];
    ramses::EEffectUniformSemantic semantic = getUniformSemanticFromString(semanticStr);
    if (semantic == ramses::EEffectUniformSemantic::Invalid)
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("invalid key word for Uniform Semantic: {}\n", semanticStr);
        printUniformSemanticList();
        return false;
    }

    if (!addUniformSemantic(inputName, semantic))
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("duplicated inputName:{} for Uniform Semantic.\n", inputName);
        return false;
    }

    return true;
}

bool EffectConfig::parseAttributeSemantic(int lineNumber, const ramses_internal::String& line, const Tokens& tokens)
{
    if (3 != tokens.size())
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("invalid config for Attribute Semantic Definition\n");
        PRINT_HINT("expected Attribute Semantic Definition: {} input_name EEffectAttributeSemantic_XXX\n\n", KEY_WORD_ATTRIBUTE_SEMANTIC);
        return false;
    }

    const ramses_internal::String& inputName = tokens[1];
    const ramses_internal::String& semanticStr = tokens[2];
    ramses::EEffectAttributeSemantic semantic = getAttributeSemanticFromString(semanticStr);
    if (semantic == ramses::EEffectAttributeSemantic::Invalid)
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("invalid key word for Attribute Semantic: {}\n", semanticStr);
        printAttributeSemanticList();
        return false;
    }

    if (!addAttributeSemantic(inputName, semantic))
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("duplicated inputName:{} for Attribute Semantic.\n", inputName);
        return false;
    }

    return true;
}

bool EffectConfig::parseDefines(int lineNumber, const ramses_internal::String& line, const Tokens& tokens)
{
    if (2 != tokens.size())
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("invalid config for Compiler Define Definition\n");
        PRINT_HINT("expected Compiler Define Definition: {} compiler_define_name\n\n", KEY_WORD_DEFINE);
        return false;
    }

    const ramses_internal::String& defineContents = tokens[1];
    if (!addCompilerDefine(defineContents))
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("duplicated Compiler Defines:{}.\n", defineContents);
        return false;
    }

    return true;
}

void EffectConfig::fillEffectDescription(ramses::EffectDescription& description) const
{
    for(const auto& compilerDefine : m_compilerDefines)
    {
        description.addCompilerDefine(compilerDefine.c_str());
    }

    for (const auto& uniformSemantic : m_uniformSemantics)
    {
        description.setUniformSemantic(uniformSemantic.key.c_str(), uniformSemantic.value);
    }

    for(const auto& attribSemantic : m_attributeSemaintics)
    {
        description.setAttributeSemantic(attribSemantic.key.c_str(), attribSemantic.value);
    }
}

bool EffectConfig::addCompilerDefine(const ramses_internal::String& define)
{
    if (m_compilerDefines.contains(define))
    {
        return false;
    }

    m_compilerDefines.put(define);
    return true;
}

bool EffectConfig::addUniformSemantic(const ramses_internal::String& inputName, ramses::EEffectUniformSemantic semantic)
{
    UniformSemantics::Iterator iter = m_uniformSemantics.find(inputName);
    if (iter != m_uniformSemantics.end())
    {
        return false;
    }

    m_uniformSemantics.put(inputName, semantic);
    return true;
}

bool EffectConfig::addAttributeSemantic(const ramses_internal::String& inputName, ramses::EEffectAttributeSemantic semantic)
{
    AttributeSemantics::Iterator iter = m_attributeSemaintics.find(inputName);
    if (iter != m_attributeSemaintics.end())
    {
        return false;
    }

    m_attributeSemaintics.put(inputName, semantic);
    return true;
}

ramses::EEffectUniformSemantic EffectConfig::getUniformSemanticFromString(const ramses_internal::String& str) const
{
    UniformSemanticNameTable::Iterator iter = m_uniformSemanticNameTable.find(str);
    if (iter == m_uniformSemanticNameTable.end())
    {
        return ramses::EEffectUniformSemantic::Invalid;
    }

    return iter->value;
}

ramses::EEffectAttributeSemantic EffectConfig::getAttributeSemanticFromString(const ramses_internal::String& str) const
{
    AttributeSemanticNameTable::Iterator iter = m_attributeSemanticNameTable.find(str);
    if (iter == m_attributeSemanticNameTable.end())
    {
        return ramses::EEffectAttributeSemantic::Invalid;
    }

    return iter->value;
}

void EffectConfig::printErrorInLine(int lineNumber, const ramses_internal::String& line) const
{
    PRINT_ERROR("effect config file error in line : #{}, \"{}\"\n", lineNumber, line);
}

void EffectConfig::printAttributeSemanticList() const
{
    PRINT_HINT("valid Attribute Semantics are as Following:\n");
    for(const auto& attribSemantic : m_attributeSemanticNameTable)
    {
        PRINT_HINT("{}\n", attribSemantic.key);
    }
    PRINT("\n");
}

void EffectConfig::printUniformSemanticList() const
{
    PRINT_HINT("valid Uniform Semantics are as Following:\n");
    for(const auto& uniformSemantic : m_uniformSemanticNameTable)
    {
        PRINT_HINT("{}\n", uniformSemantic.key);
    }
    PRINT("\n");
}

void EffectConfig::initUniformSemanticNameTable()
{
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_ProjectionMatrix", ramses::EEffectUniformSemantic::ProjectionMatrix);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_ModelMatrix", ramses::EEffectUniformSemantic::ModelMatrix);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_CameraWorldPosition", ramses::EEffectUniformSemantic::CameraWorldPosition);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_ViewMatrix", ramses::EEffectUniformSemantic::ViewMatrix);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_ModelViewMatrix", ramses::EEffectUniformSemantic::ModelViewMatrix);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_ModelViewMatrix33", ramses::EEffectUniformSemantic::ModelViewMatrix33);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_ModelViewProjectionMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_NormalMatrix", ramses::EEffectUniformSemantic::NormalMatrix);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_DisplayBufferResolution", ramses::EEffectUniformSemantic::DisplayBufferResolution);
    m_uniformSemanticNameTable.put("EEffectUniformSemantic_TextTexture", ramses::EEffectUniformSemantic::TextTexture);
}

void EffectConfig::initAttributeSemanticNameTable()
{
    m_attributeSemanticNameTable.put("EEffectAttributeSemantic_TextPositions", ramses::EEffectAttributeSemantic::TextPositions);
    m_attributeSemanticNameTable.put("EEffectAttributeSemantic_TextTextureCoordinates", ramses::EEffectAttributeSemantic::TextTextureCoordinates);
}

void EffectConfig::clear()
{
    m_compilerDefines.clear();
    m_uniformSemantics.clear();
    m_attributeSemaintics.clear();
}
