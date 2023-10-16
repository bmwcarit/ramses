//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/TextureEnums.h"

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <string_view>

namespace ramses::internal
{
    class RenderBufferImpl final : public SceneObjectImpl
    {
    public:
        RenderBufferImpl(SceneImpl& scene, std::string_view name);
        ~RenderBufferImpl() override;

        void initializeFrameworkData(uint32_t width, uint32_t height, ERenderBufferFormat bufferFormat, ERenderBufferAccessMode accessMode, uint32_t sampleCount);
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        void onValidate(ValidationReportImpl& report) const override;

        [[nodiscard]] uint32_t            getWidth() const;
        [[nodiscard]] uint32_t            getHeight() const;
        [[nodiscard]] ERenderBufferFormat getBufferFormat() const;
        [[nodiscard]] ERenderBufferAccessMode getAccessMode() const;
        [[nodiscard]] uint32_t            getSampleCount() const;

        [[nodiscard]] ramses::internal::RenderBufferHandle getRenderBufferHandle() const;

    private:
        ramses::internal::RenderBufferHandle m_renderBufferHandle;
    };
}
