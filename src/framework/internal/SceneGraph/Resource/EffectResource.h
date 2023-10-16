//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/ResourceBase.h"
#include "internal/SceneGraph/Resource/EffectInputInformation.h"
#include <optional>

#include <string_view>
#include <string>

namespace ramses::internal
{
    class IInputStream;
    class IOutputStream;

    class EffectResource : public ResourceBase
    {
    public:
        EffectResource(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader,
            std::optional<EDrawMode> geometryShaderInputType, EffectInputInformationVector uniformInputs,
            EffectInputInformationVector attributeInputs, std::string_view name);

        const char* getVertexShader() const;
        const char* getFragmentShader() const;
        const char* getGeometryShader() const;

        std::optional<EDrawMode> getGeometryShaderInputType() const;

        const EffectInputInformationVector& getUniformInputs() const;
        const EffectInputInformationVector& getAttributeInputs() const;

        DataFieldHandle getUniformDataFieldHandleByName(std::string_view name) const;
        DataFieldHandle getAttributeDataFieldHandleByName(std::string_view name) const;

        void serializeResourceMetadataToStream(IOutputStream& output) const override;
        static std::unique_ptr<IResource> CreateResourceFromMetadataStream(IInputStream& input, std::string_view name);

    private:
        EffectResource(EffectInputInformationVector&& uniformInputs, EffectInputInformationVector&& attributeInputs, std::optional<EDrawMode> geometryShaderInputType,
            std::string_view name, uint32_t fragmentShaderOffset, uint32_t geometryShaderOffset);

        static void WriteInputVector(IOutputStream& stream, const EffectInputInformationVector& inputVector);
        static void ReadInputVector(IInputStream& stream, EffectInputInformationVector& inputVector);

        const EffectInputInformationVector m_uniformInputs;
        const EffectInputInformationVector m_attributeInputs;
        const uint32_t m_fragmentShaderOffset;
        const uint32_t m_geometryShaderOffset;
        const std::optional<EDrawMode> m_geometryShaderInputType;

    };
}
