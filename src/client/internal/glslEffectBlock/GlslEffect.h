//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"
#include "internal/SceneGraph/Resource/EffectResource.h"

#include "ramses/framework/ERenderBackendCompatibility.h"

#include <string>
#include <string_view>
#include <vector>

struct TBuiltInResource;
namespace glslang
{
    class TProgram;
    class TShader;
    class TPoolAllocator;
}

namespace ramses::internal
{
    class EffectResource;

    class GlslEffect
    {
    public:
        GlslEffect(std::string_view vertexShader,
            std::string_view fragmentShader,
            std::string_view geometryShader,
            std::vector<std::string> compilerDefines,
            SemanticsMap semanticInputs,
            ERenderBackendCompatibility compatibility,
            std::string_view name);

        [[nodiscard]] std::unique_ptr<EffectResource> createEffectResource(EFeatureLevel featureLevel);

        [[nodiscard]] uint32_t getShadingLanguageVersion() const;
        [[nodiscard]] std::string getEffectErrorMessages() const;

    private:
        const std::string m_vertexShader;
        const std::string m_fragmentShader;
        const std::string m_geometryShader;
        const std::vector<std::string> m_compilerDefines;
        const SemanticsMap m_semanticInputs;
        const ERenderBackendCompatibility m_renderBackendCompatibility;
        const std::string m_name;

        StringOutputStream m_errorMessages;
        uint32_t m_shadingLanguageVersion{0};

        bool extractAndCheckShaderVersions(const glslang::TProgram* program);
        bool extractAndCheckExtensions(const glslang::TProgram* program);
        static bool IsSupportedExtension(const std::string& extension);
    };

}
