//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/AppearanceEnums.h"

#include "internal/glslEffectBlock/GLSlang.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"
#include "internal/SceneGraph/SceneAPI/EShaderStage.h"
#include "internal/SceneGraph/Resource/EffectInputInformation.h"

#include <optional>
#include <string>
#include <string_view>

namespace ramses::internal
{
    class GlslToEffectConverter
    {
    public:
        explicit GlslToEffectConverter(SemanticsMap semanticInputs);
        ~GlslToEffectConverter();

        [[nodiscard]] bool parseShaderProgram(const glslang::TProgram* program);
        [[nodiscard]] std::string getStatusMessage() const;

        [[nodiscard]] const EffectInputInformationVector& getUniformInputs() const;
        [[nodiscard]] const EffectInputInformationVector& getAttributeInputs() const;

        [[nodiscard]] std::optional<EDrawMode> getGeometryShaderInputType() const;

    private:
        bool parseLinkerObjectsForStage(const TIntermNode* node, EShaderStage stage);
        const glslang::TIntermSequence* getLinkerObjectSequence(const TIntermNode* node) const;
        bool handleSymbol(const glslang::TIntermSymbol* symbol, EShaderStage stage);
        bool getElementCountFromType(const glslang::TType& type, std::string_view inputName, uint32_t& elementCount) const;
        bool replaceVertexAttributeWithBufferVariant();
        bool makeUniformsUnique();
        bool checkIncompatibleUniformBufferRedifinitions() const;
        bool createEffectInputsRecursivelyIfNeeded(const glslang::TType& type, std::string_view inputName, std::optional<EffectInputInformation> uniformBuffer, uint32_t offset);
        bool createEffectInput(const glslang::TType& type, std::string_view inputName, uint32_t elementCount, std::optional<EffectInputInformation> uniformBuffer, uint32_t offset);
        bool setEffectInputType(const glslang::TType& type, EffectInputInformation& input) const;
        bool setEffectInputSemantics(const glslang::TType& type, bool uniformBufferField, EffectInputInformation& input) const;

        static std::string GetStructFieldIdentifier(std::string_view baseName, std::string_view fieldName, const int32_t arrayIndex);
        static uint32_t GetElementPaddedSizeFromType(const glslang::TType& type);
        static bool IsUniformAnonymous(std::string_view uniformName);
        static std::string MakeNameForAnonymousUniformBuffer(UniformBufferBinding binding);
        static bool IsSemanticCompatibleWithUniformBufferDefinition(const EFixedSemantics semantic, const glslang::TType& type);
        static bool IsAttributeType(const glslang::TType& type);

        const SemanticsMap m_semanticInputs;
        mutable StringOutputStream m_message;

        EffectInputInformationVector m_uniformInputs;
        EffectInputInformationVector m_attributeInputs;
        std::vector<std::reference_wrapper<const glslang::TType>> m_uniformInputsGlslangTypes;

        std::optional<EDrawMode> m_geometryShaderInputType;
    };
}
