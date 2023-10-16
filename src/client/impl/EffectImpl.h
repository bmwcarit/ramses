//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// client api
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/EffectInputSemantic.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

// internal
#include "impl/ResourceImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneUtils/DataLayoutCreationHelper.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "internal/glslEffectBlock/GlslParser.h"

#include <optional>
#include <string_view>
#include <string>

namespace ramses::internal
{
    class EffectInputImpl;

    class EffectImpl final : public ResourceImpl
    {
    public:
        EffectImpl(ResourceHashUsage hashUsage, SceneImpl& scene, std::string_view effectname);
        ~EffectImpl() override;

        void deinitializeFrameworkData() override;
        void initializeFromFrameworkData(const EffectInputInformationVector& uniformInputs, const EffectInputInformationVector& attributeInputs, std::optional<EDrawMode> geometryShaderInputType);

        bool serialize(IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(IInputStream& inStream, DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        size_t getUniformInputCount() const;
        size_t getAttributeInputCount() const;

        bool hasGeometryShader() const;
        bool getGeometryShaderInputType(EDrawMode& inputType) const;

        std::optional<UniformInput> getUniformInput(size_t index) const;
        std::optional<AttributeInput> getAttributeInput(size_t index) const;
        std::optional<UniformInput> findUniformInput(std::string_view inputName) const;
        std::optional<AttributeInput> findAttributeInput(std::string_view inputName) const;
        std::optional<UniformInput> findUniformInput(EEffectUniformSemantic uniformSemantic) const;
        std::optional<AttributeInput> findAttributeInput(EEffectAttributeSemantic attributeSemantic) const;

        const EffectInputInformationVector& getUniformInputInformation() const;
        const EffectInputInformationVector& getAttributeInputInformation() const;

    private:
        static const size_t InvalidInputIndex = 0xffff;

        static size_t GetEffectInputIndex(const EffectInputInformationVector& effectInputVector, std::string_view inputName);
        static size_t FindEffectInputIndex(const EffectInputInformationVector& effectInputVector, EFixedSemantics inputSemantics);
        void initializeEffectInputData(EffectInputImpl& effectInputImpl, const EffectInputInformation& effectInputInfo, size_t index) const;

        EffectInputInformationVector m_effectUniformInputs;
        EffectInputInformationVector m_effectAttributeInputs;
        std::optional<EDrawMode> m_geometryShaderInputType;

        mutable std::optional<GlslParser::Warnings> m_shaderWarnings;
    };
}
