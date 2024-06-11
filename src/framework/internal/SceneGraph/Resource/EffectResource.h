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
#include "ramses/framework/EFeatureLevel.h"
#include "SPIRVShaders.h"
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
        EffectResource(const std::string& vertexShader,
            const std::string& fragmentShader,
            const std::string& geometryShader,
            const SPIRVShaders& spirvShaders,
            std::optional<EDrawMode> geometryShaderInputType,
            EffectInputInformationVector uniformInputs,
            EffectInputInformationVector attributeInputs,
            std::string_view name,
            EFeatureLevel featureLevel);

        const char* getVertexShader() const;
        const char* getFragmentShader() const;
        const char* getGeometryShader() const;

        std::optional<EDrawMode> getGeometryShaderInputType() const;

        uint32_t getVertexShaderSPIRVSize() const;
        uint32_t getFragmentShaderSPIRVSize() const;
        uint32_t getGeometryShaderSPIRVSize() const;
        const uint32_t* getVertexShaderSPIRV() const;
        const uint32_t* getFragmentShaderSPIRV() const;
        const uint32_t* getGeometryShaderSPIRV() const;

        const EffectInputInformationVector& getUniformInputs() const;
        const EffectInputInformationVector& getAttributeInputs() const;

        DataFieldHandle getUniformDataFieldHandleByName(std::string_view name) const;
        DataFieldHandle getAttributeDataFieldHandleByName(std::string_view name) const;

        void serializeResourceMetadataToStream(IOutputStream& output) const override;
        static std::unique_ptr<IResource> CreateResourceFromMetadataStream(IInputStream& input, std::string_view name, EFeatureLevel featureLevel);

    private:
        enum EByteOffsetIndex
        {
            EByteOffsetIndex_VertexSPIRV =0,
            EByteOffsetIndex_FragmentSPIRV,
            EByteOffsetIndex_GeometrySPIRV,
            EByteOffsetIndex_VertexShader,
            EByteOffsetIndex_FragmentShader,
            EByteOffsetIndex_GeometryShader,
            EByteOffsetIndex_EndOfData
        };
        static constexpr std::size_t OffsetCount = EByteOffsetIndex_EndOfData + 1u;
        using EffectByteOffsets = std::array<uint32_t, OffsetCount>;

        EffectResource(EffectInputInformationVector&& uniformInputs,
            EffectInputInformationVector&& attributeInputs,
            std::optional<EDrawMode> geometryShaderInputType,
            std::string_view name,
            EffectByteOffsets&& byteOffsets,
            EFeatureLevel featureLevel);

        static EffectByteOffsets CreateByteOffsets(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader, const SPIRVShaders& spirvShaders);

        void setDataFieldMappingForInputs();

        static void WriteInputVector(IOutputStream& stream, const EffectInputInformationVector& inputVector, EFeatureLevel featureLevel);
        static void ReadInputVector(IInputStream& stream, EffectInputInformationVector& inputVector, EFeatureLevel featureLevel);

        EffectInputInformationVector m_uniformInputs;
        EffectInputInformationVector m_attributeInputs;
        EffectByteOffsets m_byteOffsets = { 0 };

        std::optional<EDrawMode> m_geometryShaderInputType;
        EFeatureLevel m_featureLevel = EFeatureLevel_Latest;
    };
}
