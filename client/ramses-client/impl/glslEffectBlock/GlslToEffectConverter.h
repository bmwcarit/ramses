//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLSLTOEFFECTCONVERTER_H
#define RAMSES_GLSLTOEFFECTCONVERTER_H

#include "glslEffectBlock/GLSlang.h"
#include "Collections/StringOutputStream.h"
#include "Collections/HashMap.h"
#include "Collections/String.h"
#include "SceneAPI/EFixedSemantics.h"
#include "Resource/EffectInputInformation.h"

namespace ramses_internal
{
    class GlslToEffectConverter
    {
    public:
        explicit GlslToEffectConverter(const HashMap<String, EFixedSemantics>& semanticInputs);
        ~GlslToEffectConverter();

        Bool parseShaderProgram(glslang::TProgram* program);
        String getStatusMessage() const;

        const EffectInputInformationVector& getUniformInputs() const;
        const EffectInputInformationVector& getAttributeInputs() const;

    private:
        enum class EShaderStage
        {
            Vertex = 1,
            Geometry = 2,
            Fragment = 4,
            Invalid
        };

        HashMap<String, EFixedSemantics> m_semanticInputs;
        mutable StringOutputStream m_message;

        EffectInputInformationVector m_uniformInputs;
        EffectInputInformationVector m_attributeInputs;

        bool parseLinkerObjectsForStage(const TIntermNode* node, EShaderStage stage);
        const glslang::TIntermSequence* getLinkerObjectSequence(const TIntermNode* node) const;
        bool handleSymbol(const glslang::TIntermSymbol* symbol, EShaderStage stage);

        bool getElementCountFromType(const glslang::TType& type, const String& inputName, uint32_t& elementCount) const;
        bool setInputTypeFromType(const glslang::TType& type, const String& inputName, EffectInputInformationVector& outputVector) const;
        bool setInputTypeFromType(const glslang::TType& type, EffectInputInformation& input) const;
        bool replaceVertexAttributeWithBufferVariant();
        bool setSemanticsOnInput(EffectInputInformation& input) const;
        bool makeUniformsUnique();
        bool createEffectInputType(const glslang::TType& type, const String& inputName, uint32_t elementCount, EffectInputInformationVector& outputVector) const;
        const String getStructFieldIdentifier(const String& baseName, const String& fieldName, const int32_t arrayIndex) const;
    };
}

#endif
