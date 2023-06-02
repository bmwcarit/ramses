//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTIMPL_H
#define RAMSES_EFFECTIMPL_H

// client api
#include "ramses-client-api/EffectInputSemantic.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

// internal
#include "ResourceImpl.h"

// ramses framework
#include "SceneUtils/DataLayoutCreationHelper.h"
#include "SceneAPI/RenderState.h"

#include <optional>
#include <string_view>

namespace ramses
{
    class EffectInputImpl;

    class EffectImpl final : public ResourceImpl
    {
    public:
        EffectImpl(ramses_internal::ResourceHashUsage hashUsage, SceneImpl& scene, std::string_view effectname);
        ~EffectImpl() override;

        void initializeFromFrameworkData(const ramses_internal::EffectInputInformationVector& uniformInputs, const ramses_internal::EffectInputInformationVector& attributeInputs, std::optional<EDrawMode> geometryShaderInputType);

        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        size_t getUniformInputCount() const;
        size_t getAttributeInputCount() const;

        bool hasGeometryShader() const;
        status_t getGeometryShaderInputType(EDrawMode& inputType) const;

        status_t getUniformInput(size_t index, EffectInputImpl& inputImpl) const;
        status_t findUniformInput(EEffectUniformSemantic uniformSemantic, EffectInputImpl& inputImpl) const;
        status_t getAttributeInput(size_t index, EffectInputImpl& inputImpl) const;
        status_t findAttributeInput(EEffectAttributeSemantic attributeSemantic, EffectInputImpl& inputImpl) const;
        status_t findUniformInput(std::string_view inputName, EffectInputImpl& inputImpl) const;
        status_t findAttributeInput(std::string_view inputName, EffectInputImpl& inputImpl) const;

        const ramses_internal::EffectInputInformationVector& getUniformInputInformation() const;
        const ramses_internal::EffectInputInformationVector& getAttributeInputInformation() const;

    private:
        static const size_t InvalidInputIndex = 0xffff;

        size_t getEffectInputIndex(const ramses_internal::EffectInputInformationVector& effectInputVector, std::string_view inputName) const;
        size_t findEffectInputIndex(const ramses_internal::EffectInputInformationVector& effectInputVector, ramses_internal::EFixedSemantics inputSemantics) const;
        void initializeEffectInputData(EffectInputImpl& effectInputImpl, const ramses_internal::EffectInputInformation& effectInputInfo, size_t index) const;

        ramses_internal::EffectInputInformationVector m_effectUniformInputs;
        ramses_internal::EffectInputInformationVector m_effectAttributeInputs;
        std::optional<EDrawMode> m_geometryShaderInputType;
    };
}

#endif
