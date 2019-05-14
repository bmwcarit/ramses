//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_EFFECTCONFIG_H
#define RAMSES_UTILS_EFFECTCONFIG_H

#include "ramses-client-api/EffectDescription.h"
#include "Collections/Vector.h"
#include "Collections/HashSet.h"
#include "Collections/HashMap.h"
#include "Collections/String.h"

class EffectConfig
{
public:
    EffectConfig();
    bool loadFromFile(const char* filePath);

    void fillEffectDescription(ramses::EffectDescription& description) const;

private:
    typedef std::vector<ramses_internal::String> Tokens;
    typedef ramses_internal::HashSet<ramses_internal::String> CompilerDefines;
    typedef ramses_internal::HashMap<ramses_internal::String, ramses::EEffectUniformSemantic> UniformSemantics;
    typedef ramses_internal::HashMap<ramses_internal::String, ramses::EEffectAttributeSemantic> AttributeSemantics;
    typedef ramses_internal::HashMap<ramses_internal::String, ramses::EEffectUniformSemantic> UniformSemanticNameTable;
    typedef ramses_internal::HashMap<ramses_internal::String, ramses::EEffectAttributeSemantic> AttributeSemanticNameTable;

    bool parseConfigLine(int lineNumber, const ramses_internal::String& line);
    bool parseUniformSemantic(int lineNumber, const ramses_internal::String& line, const Tokens& tokens);
    bool parseAttributeSemantic(int lineNumber, const ramses_internal::String& line, const Tokens& tokens);
    bool parseDefines(int lineNumber, const ramses_internal::String& line, const Tokens& tokens);

    bool addCompilerDefine(const ramses_internal::String& define);
    bool addUniformSemantic(const ramses_internal::String& inputName, ramses::EEffectUniformSemantic semantic);
    bool addAttributeSemantic(const ramses_internal::String& inputName, ramses::EEffectAttributeSemantic semantic);

    void printErrorInLine(int lineNumber, const ramses_internal::String& line) const;
    void printAttributeSemanticList() const;
    void printUniformSemanticList() const;

    void initUniformSemanticNameTable();
    void initAttributeSemanticNameTable();
    void clear();

    ramses::EEffectUniformSemantic getUniformSemanticFromString(const ramses_internal::String& str) const;
    ramses::EEffectAttributeSemantic getAttributeSemanticFromString(const ramses_internal::String& str) const;

    CompilerDefines m_compilerDefines;
    UniformSemantics m_uniformSemantics;
    AttributeSemantics m_attributeSemaintics;

    UniformSemanticNameTable m_uniformSemanticNameTable;
    AttributeSemanticNameTable m_attributeSemanticNameTable;
};

#endif
