//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "glslEffectBlock/GlslParser.h"
#include "GlslLimits.h"
#include "fmt/format.h"
#include "PlatformAbstraction/FmtBase.h"
#include "EffectDescriptionImpl.h"

template <> struct fmt::formatter<glslang::TIntermSymbol> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glslang::TIntermSymbol& symbol, FormatContext& ctx)
    {
        const auto& qualifier = symbol.getQualifier();
        if (qualifier.hasLayout() && qualifier.hasAnyLocation())
        {
            return fmt::format_to(ctx.out(), "layout(location = {}) {}", qualifier.layoutLocation, symbol.getName());
        }
        else
        {
            return fmt::format_to(ctx.out(), "{}", symbol.getName());
        }
    }
};

namespace
{
    bool isInput(glslang::TIntermSymbol* symbol)
    {
        const auto storageQualifier = symbol->getType().getQualifier().storage;
        switch (storageQualifier)
        {
        case glslang::EvqVaryingIn:
            return true;
        default:
            break;
        }
        return false;
    }

    bool isOutput(glslang::TIntermSymbol* symbol)
    {
        const auto storageQualifier = symbol->getType().getQualifier().storage;
        switch (storageQualifier)
        {
        case glslang::EvqVaryingOut:
            return true;
        default:
            break;
        }
        return false;
    }

    bool isUniform(glslang::TIntermSymbol* symbol)
    {
        return (glslang::EvqUniform == symbol->getType().getQualifier().storage);
    }

    using SymbolMap  = std::unordered_map<glslang::TString, glslang::TIntermSymbol*>;

    class ShaderInterface
    {
    public:
        explicit ShaderInterface(const SymbolMap& symbolMap)
            : m_symbols(symbolMap)
        {
        }

        /**
         * Finds a linker match for the given symbol
         *
         * @param symbol symbol to find a match for
         */
        auto findMatch(const glslang::TIntermSymbol* symbol) const
        {
            const auto& qualifier = symbol->getQualifier();
            // symbol name is irrelevant if symbol has a layout location
            if (qualifier.hasLayout() && qualifier.hasAnyLocation())
            {
                auto pred = [&](const SymbolMap::value_type& it) {
                    const auto& q = it.second->getQualifier();
                    if (q.hasLayout() && q.hasAnyLocation())
                    {
                        return qualifier.layoutLocation == q.layoutLocation;
                    }
                    return false;
                };
                return std::find_if(m_symbols.begin(), m_symbols.end(), pred);
            }
            return m_symbols.find(symbol->getName());
        }

        auto begin() const
        {
            return m_symbols.begin();
        }

        auto end() const
        {
            return m_symbols.end();
        }

    private:
        const SymbolMap& m_symbols;
    };

    class UnusedVariableTraverser : public glslang::TIntermTraverser
    {
    public:
        using LoggerFunc = std::function<void(ramses_internal::EShaderWarningCategory, const std::string&)>;

        explicit UnusedVariableTraverser(LoggerFunc logger)
            : m_logger(std::move(logger))
            , m_interface(m_interfaceSymbols)
        {
        }

        const ShaderInterface& getInterface() const
        {
            return m_interface;
        }

    private:
        virtual bool visitAggregate(glslang::TVisit, glslang::TIntermAggregate* node) override
        {
            if (node->getOp() == glslang::EOpLinkerObjects)
            {
                assert(!m_link); // only expected once per shader
                m_link = true;
            }
            return true;
        }

        virtual void visitSymbol(glslang::TIntermSymbol* symbol) override
        {
            const auto storageQualifier = symbol->getType().getQualifier().storage;
            switch (storageQualifier)
            {
            case glslang::EvqConst: // constants are replaced by values in the AST
            case glslang::EvqVertexId:
            case glslang::EvqInstanceId:
                return;
            default:
                break;
            }

            if (m_link)
            {
                if (m_usedSymbols.find(symbol->getName()) == m_usedSymbols.end())
                {
                    const auto msg = fmt::format("Unused [{}]: '{}' ({})", glslang::GetStorageQualifierString(storageQualifier), symbol->getName(), symbol->getCompleteString());
                    auto category = ramses_internal::EShaderWarningCategory::UnusedVariable;
                    switch (storageQualifier)
                    {
                    case glslang::EvqUniform:
                        category = ramses_internal::EShaderWarningCategory::UnusedUniform;
                        break;
                    case glslang::EvqVaryingIn:
                    case glslang::EvqVaryingOut:
                        category = ramses_internal::EShaderWarningCategory::UnusedVarying;
                        break;
                    default:
                        break;
                    }
                    m_logger(category, msg);
                }
                m_interfaceSymbols[symbol->getName()] = symbol;
            }
            else
            {
                m_usedSymbols[symbol->getName()] = symbol;
            }
        }

        LoggerFunc m_logger;
        bool       m_link = false;
        SymbolMap  m_usedSymbols;
        SymbolMap  m_interfaceSymbols;
        ShaderInterface m_interface;
    };
}


namespace ramses_internal
{
    GlslParser::GlslParser(const String& vertexShader, const String& fragmentShader, const String& geometryShader, const std::vector<String>& compilerDefines)
    {
        m_vertexShader = std::make_unique<glslang::TShader>(EShLangVertex);
        m_fragmentShader = std::make_unique<glslang::TShader>(EShLangFragment);
        if (!geometryShader.empty())
        {
            m_geometryShader = std::make_unique<glslang::TShader>(EShLangGeometry);
        }

        const String defineString = CreateDefineString(compilerDefines);
        const bool hasGeometryShader = (m_geometryShader != nullptr);

        ShaderParts vertexShaderParts;
        ShaderParts fragmentShaderParts;
        ShaderParts geometryShaderParts;

        if (!createShaderParts(vertexShaderParts, defineString, vertexShader, "vertex shader") ||
            !createShaderParts(fragmentShaderParts, defineString, fragmentShader, "fragment shader") ||
            (hasGeometryShader && !createShaderParts(geometryShaderParts, defineString, geometryShader, "geometry shader")))
        {
            return;
        }

        TBuiltInResource glslCompilationResources;
        // Use the GLSL version to determine the resource limits in the shader. The version used in vertex
        // and fragment shader must be the same (checked later), so we just pass one of them for now.
        GlslLimits::InitCompilationResources(glslCompilationResources, vertexShaderParts.version);
        if (!parseShader(*m_vertexShader, glslCompilationResources, vertexShaderParts, "vertex shader") ||
            !parseShader(*m_fragmentShader, glslCompilationResources, fragmentShaderParts, "fragment shader") ||
            (hasGeometryShader && !parseShader(*m_geometryShader, glslCompilationResources, geometryShaderParts, "geometry shader")))
        {
            return;
        }

        linkProgram();
    }

    ramses_internal::String GlslParser::getErrors() const
    {
        return ramses_internal::String(m_errorMessages.c_str());
    }

    const glslang::TProgram* GlslParser::getProgram() const
    {
        return m_program.get();
    }

    GlslParser::Warnings GlslParser::generateWarnings()
    {
        Warnings warnings;
        if (!m_program)
        {
            return warnings;
        }

        // find unused variables
        UnusedVariableTraverser tVertex([&](EShaderWarningCategory category, const std::string& msg) { warnings.push_back({EShaderStage::Vertex, category, msg}); });
        m_program->getIntermediate(EShLangVertex)->getTreeRoot()->traverse(&tVertex);
        UnusedVariableTraverser tFragment([&](EShaderWarningCategory category, const std::string& msg) { warnings.push_back({EShaderStage::Fragment, category, msg}); });
        m_program->getIntermediate(EShLangFragment)->getTreeRoot()->traverse(&tFragment);

        // check interface between vertex and fragment shader
        const auto& fInterface = tFragment.getInterface();
        for (const auto& v : tVertex.getInterface())
        {
            if (!isOutput(v.second) && !isUniform(v.second))
                continue;
            auto it = fInterface.findMatch(v.second);
            if (it == fInterface.end())
            {
                if (isOutput(v.second))
                {
                    warnings.push_back({EShaderStage::Vertex, EShaderWarningCategory::InterfaceMismatch,
                        fmt::format("Vertex shader output '{}' is not input in fragment shader", *v.second)});
                }
            }
            else
            {
                const auto& vType = v.second->getType();
                const auto& fType = it->second->getType();
                // operator!= only compares the type, but not the qualifier
                if (vType != fType)
                {
                    warnings.push_back( {EShaderStage::Vertex, EShaderWarningCategory::InterfaceMismatch, // extra category?
                        fmt::format("Type mismatch: '{}'. (Vertex:{}, Fragment:{})", *v.second, v.second->getCompleteString(), it->second->getCompleteString())});
                }
                else if (fType.getQualifier().precision != vType.getQualifier().precision)
                {
                    warnings.push_back( {EShaderStage::Vertex, EShaderWarningCategory::PrecisionMismatch,
                        fmt::format("Precision mismatch: '{}'. (Vertex:{}, Fragment:{})", *v.second, v.second->getCompleteString(), it->second->getCompleteString())});
                }
            }
        }

        const auto& vInterface = tVertex.getInterface();
        for (const auto& f : fInterface)
        {
            if (!isInput(f.second))
                continue;
            auto it = vInterface.findMatch(f.second);
            if (it == vInterface.end())
            {
                warnings.push_back({EShaderStage::Fragment, EShaderWarningCategory::InterfaceMismatch,
                    fmt::format("Fragment shader input '{}' is not output in vertex shader", *f.second)});
            }
        }
        return warnings;
    }

    String GlslParser::CreateDefineString(const std::vector<String>& compilerDefines)
    {
        StringOutputStream result;
        for(const auto& defineString : compilerDefines)
        {
            result << "#define " << defineString << "\n";
        }
        return String(result.release());
    }

    bool GlslParser::createShaderParts(ShaderParts& outParts, const String& defineString, const String& userShader, const String& shaderName) const
    {
        size_t versionStringStart;
        String versionString;
        if ((versionStringStart = userShader.find("#version")) != String::npos)
        {
            size_t versionStringEnd = userShader.find('\n', versionStringStart);
            if (versionStringEnd == String::npos)
            {
                m_errorMessages << "[GLSL Compiler] " << shaderName << " Shader contains #version without newline \n";
                return false;
            }

            outParts.version  = userShader.substr(versionStringStart, versionStringEnd + 1 - versionStringStart);
            outParts.userCode = userShader.substr(versionStringEnd + 1, userShader.size() - versionStringEnd - 1);
        }
        else
        {
            outParts.version = "#version 100\n";
            outParts.userCode = userShader;
        }

        outParts.defines = defineString;
        return true;
    }

    bool GlslParser::parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const String& shaderName)
    {
        const char* fragmentShaderCodeCString[] = { shaderParts.version.c_str(), shaderParts.defines.c_str(), shaderParts.userCode.c_str() };
        tShader.setStrings(fragmentShaderCodeCString, 3);
        bool parsingSuccessful = tShader.parse(&glslCompilationResources, 100, false, EShMsgDefault);
        if (!parsingSuccessful)
        {
            m_errorMessages << "[GLSL Compiler] " << shaderName << " Shader Parsing Error:\n" << tShader.getInfoLog() << "\n";
            return false;
        }
        return true;
    }

    void GlslParser::linkProgram()
    {
        auto program = std::make_unique<glslang::TProgram>();
        program->addShader(m_vertexShader.get());
        program->addShader(m_fragmentShader.get());
        if (m_geometryShader)
            program->addShader(m_geometryShader.get());

        if (!program->link(EShMsgDefault))
        {
            m_errorMessages << "[GLSL Compiler] Shader Program Linker Error:\n" << program->getInfoLog() << "\n";
            return;
        }

        if (!program->buildReflection())
        {
            m_errorMessages << "[GLSL Compiler] Shader program error in buildReflection\n";
            return;
        }
        m_program = std::move(program);
    }
}
