//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTRESOURCE_H
#define RAMSES_EFFECTRESOURCE_H

#include "Resource/ResourceBase.h"
#include "Resource/EffectInputInformation.h"
#include "absl/types/optional.h"

namespace ramses_internal
{
    class IInputStream;
    class IOutputStream;

    class EffectResource : public ResourceBase
    {
    public:
        EffectResource(const String& vertexShader, const String& fragmentShader, const String& geometryShader,
            absl::optional<EDrawMode> geometryShaderInputType, const EffectInputInformationVector& uniformInputs,
            const EffectInputInformationVector& attributeInputs, const String& name, ResourceCacheFlag cacheFlag);

        const char* getVertexShader() const;
        const char* getFragmentShader() const;
        const char* getGeometryShader() const;

        absl::optional<EDrawMode> getGeometryShaderInputType() const;

        const EffectInputInformationVector& getUniformInputs() const;
        const EffectInputInformationVector& getAttributeInputs() const;

        DataFieldHandle getUniformDataFieldHandleByName(const String& name) const;
        DataFieldHandle getAttributeDataFieldHandleByName(const String& name) const;

        virtual void serializeResourceMetadataToStream(IOutputStream& output) const override;
        static IResource* CreateResourceFromMetadataStream(IInputStream& input, ResourceCacheFlag cacheFlag, const String& name);

    private:
        EffectResource(const EffectInputInformationVector& uniformInputs, const EffectInputInformationVector& attributeInputs, absl::optional<EDrawMode> geometryShaderInputType,
            const String& name, UInt32 fragmentShaderOffset, UInt32 geometryShaderOffset, ResourceCacheFlag cacheFlag);

        static void WriteInputVector(IOutputStream& stream, const EffectInputInformationVector& inputVector);
        static void ReadInputVector(IInputStream& stream, EffectInputInformationVector& inputVector);

        const EffectInputInformationVector m_uniformInputs;
        const EffectInputInformationVector m_attributeInputs;
        const UInt32 m_fragmentShaderOffset;
        const UInt32 m_geometryShaderOffset;
        const absl::optional<EDrawMode> m_geometryShaderInputType;

    };
}

#endif
