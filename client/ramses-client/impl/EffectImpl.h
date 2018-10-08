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

namespace ramses
{
    class EffectInputImpl;

    class EffectImpl final : public ResourceImpl
    {
    public:
        EffectImpl(ramses_internal::ResourceHashUsage hashUsage, RamsesClientImpl& client, const char* effectname);
        virtual ~EffectImpl();

        void initializeFromFrameworkData(const ramses_internal::EffectInputInformationVector& uniformInputs, const ramses_internal::EffectInputInformationVector& attributeInputs);

        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        uint32_t getUniformInputCount() const;
        uint32_t getAttributeInputCount() const;

        status_t getUniformInput(uint32_t index, EffectInputImpl& inputImpl) const;
        status_t findUniformInput(EEffectUniformSemantic uniformSemantic, EffectInputImpl& inputImpl) const;
        status_t getAttributeInput(uint32_t index, EffectInputImpl& inputImpl) const;
        status_t findAttributeInput(EEffectAttributeSemantic attributeSemantic, EffectInputImpl& inputImpl) const;
        status_t findUniformInput(const char* inputName, EffectInputImpl& inputImpl) const;
        status_t findAttributeInput(const char* inputName, EffectInputImpl& inputImpl) const;

        const ramses_internal::EffectInputInformationVector& getUniformInputInformation() const;
        const ramses_internal::EffectInputInformationVector& getAttributeInputInformation() const;

    private:
        static const uint32_t InvalidInputIndex = 0xffff;

        status_t inputNotFoundVerboseError(const char* sourceMethod, const char* inputType, const char* inputName) const;

        uint32_t getEffectInputIndex(const ramses_internal::EffectInputInformationVector& effectInputVector, const char* inputName) const;
        uint32_t findEffectInputIndex(const ramses_internal::EffectInputInformationVector& effectInputVector, ramses_internal::EFixedSemantics inputSemantics) const;
        void initializeEffectInputData(EffectInputImpl& effectInputImpl, const ramses_internal::EffectInputInformation& effectInputInfo, uint32_t index) const;

        ramses_internal::EffectInputInformationVector m_effectUniformInputs;
        ramses_internal::EffectInputInformationVector m_effectAttributeInputs;
    };
}

#endif
