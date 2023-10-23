//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/glslEffectBlock/GlslParser.h"
#include "GlslLimits.h"
#include "fmt/format.h"
#include "internal/PlatformAbstraction/FmtBase.h"
#include "impl/EffectDescriptionImpl.h"

template <> struct fmt::formatter<glslang::TIntermSymbol> : public ramses::internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glslang::TIntermSymbol& symbol, FormatContext& ctx)
    {
        const auto& qualifier = symbol.getQualifier();
        if (qualifier.hasLayout() && qualifier.hasAnyLocation())
        {
            return fmt::format_to(ctx.out(), "layout(location = {}) {}", qualifier.layoutLocation, symbol.getName());
        }
        return fmt::format_to(ctx.out(), "{}", symbol.getName());
    }
};

namespace
{
    bool isInput(glslang::TIntermSymbol* symbol)
    {
        const auto storageQualifier = symbol->getType().getQualifier().storage;
        return (storageQualifier == glslang::EvqVaryingIn);
    }

    bool isOutput(glslang::TIntermSymbol* symbol)
    {
        const auto storageQualifier = symbol->getType().getQualifier().storage;
        return (storageQualifier == glslang::EvqVaryingOut);
    }

    bool isUniform(glslang::TIntermSymbol* symbol)
    {
        return (glslang::EvqUniform == symbol->getType().getQualifier().storage);
    }

    using SymbolMap = std::unordered_map<glslang::TString, glslang::TIntermSymbol*>;

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
        [[nodiscard]] auto findMatch(const glslang::TIntermSymbol* symbol) const
        {
            const auto& qualifier = symbol->getQualifier();
            // symbol name is irrelevant if symbol has a layout location
            if (qualifier.hasLayout() && qualifier.hasAnyLocation())
            {
                auto pred = [&](const SymbolMap::value_type& it) {
                    const auto& q = it.second->getQualifier();
                    if (q.hasLayout() && q.hasAnyLocation())
                    {
                        return qualifier.layoutLocation == q.layoutLocation; // NOLINT(cppcoreguidelines-narrowing-conversions): false positive
                    }
                    return false;
                };
                return std::find_if(m_symbols.begin(), m_symbols.end(), pred);
            }
            return m_symbols.find(symbol->getName());
        }

        [[nodiscard]] auto begin() const
        {
            return m_symbols.begin();
        }

        [[nodiscard]] auto end() const
        {
            return m_symbols.end();
        }

    private:
        const SymbolMap& m_symbols;
    };

    class UnusedVariableTraverser : public glslang::TIntermTraverser
    {
    public:
        using LoggerFunc = std::function<void(ramses::internal::EShaderWarningCategory, const std::string&)>;

        explicit UnusedVariableTraverser(LoggerFunc logger)
            : m_logger(std::move(logger))
            , m_interface(m_interfaceSymbols)
        {
        }

        [[nodiscard]] const ShaderInterface& getInterface() const
        {
            return m_interface;
        }

    private:
        bool visitAggregate(glslang::TVisit /*unused*/, glslang::TIntermAggregate* node) override
        {
            if (node->getOp() == glslang::EOpLinkerObjects)
            {
                assert(!m_link); // only expected once per shader
                m_link = true;
            }
            return true;
        }

        void visitSymbol(glslang::TIntermSymbol* symbol) override
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
                    auto category = ramses::internal::EShaderWarningCategory::UnusedVariable;
                    switch (storageQualifier)
                    {
                    case glslang::EvqUniform:
                        category = ramses::internal::EShaderWarningCategory::UnusedUniform;
                        break;
                    case glslang::EvqVaryingIn:
                    case glslang::EvqVaryingOut:
                        category = ramses::internal::EShaderWarningCategory::UnusedVarying;
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

        LoggerFunc      m_logger;
        bool            m_link = false;
        SymbolMap       m_usedSymbols;
        SymbolMap       m_interfaceSymbols;
        ShaderInterface m_interface;
    };
}


namespace ramses::internal
{
    GlslParser::GlslParser(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader, const std::vector<std::string>& compilerDefines)
    {
        m_vertexShader   = std::make_unique<glslang::TShader>(EShLangVertex);
        m_fragmentShader = std::make_unique<glslang::TShader>(EShLangFragment);
        if (!geometryShader.empty())
        {
            m_geometryShader = std::make_unique<glslang::TShader>(EShLangGeometry);
        }

        const auto defineString      = CreateDefineString(compilerDefines);
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

        TBuiltInResource glslCompilationResources{};
        // Use the GLSL version to determine the resource limits in the shader. The version used in vertex
        // and fragment shader must be the same (checked later), so we just pass one of them for now.
        GlslLimits::InitCompilationResources(glslCompilationResources, vertexShaderParts.version);
        if (!parseShader(*m_vertexShader, glslCompilationResources, vertexShaderParts, "vertex shader") ||
            !parseShader(*m_fragmentShader, glslCompilationResources, fragmentShaderParts, "fragment shader") ||
            (hasGeometryShader && !parseShader(*m_geometryShader, glslCompilationResources, geometryShaderParts, "geometry shader")))
        {
            return;
        }

        if (!linkProgram())
        {
            return;
        }

        { // merge shaders from parts
            m_vertexShaderFromParts   = MergeShaderParts(vertexShaderParts);
            m_fragmentShaderFromParts = MergeShaderParts(fragmentShaderParts);
            m_geometryShaderFromParts = MergeShaderParts(geometryShaderParts);
        }
    }

    bool GlslParser::linkProgram()
    {
        auto program = std::make_unique<glslang::TProgram>();
        program->addShader(m_vertexShader.get());
        program->addShader(m_fragmentShader.get());
        if (m_geometryShader)
            program->addShader(m_geometryShader.get());

        if (!program->link(EShMsgDefault))
        {
            m_errorMessages << "[GLSL Compiler] Shader Program Linker Error:\n" << program->getInfoLog() << "\n";
            return false;
        }

        if (!program->buildReflection())
        {
            m_errorMessages << "[GLSL Compiler] Shader program error in buildReflection\n";
            return false;
        }
        m_program = std::move(program);
        return true;
    }

    std::string GlslParser::getErrors() const
    {
        return m_errorMessages.c_str();
    }

    const glslang::TProgram* GlslParser::getProgram() const
    {
        return m_program.get();
    }

    bool GlslParser::valid() const
    {
        return m_program != nullptr;
    }

    GlslParser::Warnings GlslParser::generateWarnings() const
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
                    warnings.push_back({EShaderStage::Vertex, EShaderWarningCategory::InterfaceMismatch,
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

    std::string GlslParser::CreateDefineString(const std::vector<std::string>& compilerDefines)
    {
        StringOutputStream result;
        for (const auto& defineString : compilerDefines)
        {
            result << "#define " << defineString << "\n";
        }
        return std::string(result.release());
    }

    bool GlslParser::createShaderParts(ShaderParts& outParts, const std::string& defineString, const std::string& userShader, const std::string& shaderName) const
    {
        size_t      versionStringStart = 0;
        std::string versionString;
        if ((versionStringStart = userShader.find("#version")) != std::string::npos)
        {
            size_t versionStringEnd = userShader.find('\n', versionStringStart);
            if (versionStringEnd == std::string::npos)
            {
                m_errorMessages << "[GLSL Compiler] " << shaderName << " Shader contains #version without newline \n";
                return false;
            }

            outParts.version  = userShader.substr(versionStringStart, versionStringEnd + 1 - versionStringStart);
            outParts.userCode = userShader.substr(versionStringEnd + 1, userShader.size() - versionStringEnd - 1);
        }
        else
        {
            outParts.version  = "#version 100\n";
            outParts.userCode = userShader;
        }

        outParts.defines = defineString;
        return true;
    }

    bool GlslParser::parseShader(glslang::TShader& tShader, const TBuiltInResource& glslCompilationResources, const ShaderParts& shaderParts, const std::string& shaderName)
    {
        std::array fragmentShaderCodeCString = {shaderParts.version.c_str(), shaderParts.defines.c_str(), shaderParts.userCode.c_str()};
        tShader.setStrings(fragmentShaderCodeCString.data(), 3);
        bool parsingSuccessful = tShader.parse(&glslCompilationResources, 100, false, EShMsgDefault);
        if (!parsingSuccessful)
        {
            m_errorMessages << "[GLSL Compiler] " << shaderName << " Shader Parsing Error:\n" << tShader.getInfoLog() << "\n";
            return false;
        }
        return true;
    }

    const std::string& GlslParser::getVertexShader() const
    {
        return m_vertexShaderFromParts;
    }

    const std::string& GlslParser::getFragmentShader() const
    {
        return m_fragmentShaderFromParts;
    }

    const std::string& GlslParser::getGeometryShader() const
    {
        return m_geometryShaderFromParts;
    }

    std::string GlslParser::MergeShaderParts(const ShaderParts& shaderParts)
    {
        StringOutputStream str;
        str << shaderParts.version << shaderParts.defines << shaderParts.userCode;
        return str.release();
    }
}
