//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneObjectImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <string_view>

namespace ramses
{
    class RenderBuffer;
}

namespace ramses::internal
{
    class RenderBufferImpl;

    class BlitPassImpl final : public SceneObjectImpl
    {
    public:
        BlitPassImpl(SceneImpl& scene, std::string_view blitpassName);
        ~BlitPassImpl() override;

        void initializeFrameworkData(const RenderBufferImpl& sourceRenderBuffer, const RenderBufferImpl& destinationRenderBuffer);
        void deinitializeFrameworkData() override;
        bool serialize(IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        [[nodiscard]] const ramses::RenderBuffer& getSourceRenderBuffer() const;
        [[nodiscard]] const ramses::RenderBuffer& getDestinationRenderBuffer() const;

        bool setBlittingRegion(uint32_t sourceX, uint32_t sourceY, uint32_t destinationX, uint32_t destinationY, uint32_t width, uint32_t height);
        void getBlittingRegion(uint32_t& sourceX, uint32_t& sourceY, uint32_t& destinationX, uint32_t& destinationY, uint32_t& width, uint32_t& height) const;

        bool setRenderOrder(int32_t renderOrder);
        [[nodiscard]] int32_t getRenderOrder() const;

        bool setEnabled(bool isEnabeld);
        [[nodiscard]] bool isEnabled() const;

        [[nodiscard]] BlitPassHandle getBlitPassHandle() const;

    private:
        BlitPassHandle m_blitPassHandle;
        const RenderBufferImpl* m_sourceRenderBufferImpl = nullptr;
        const RenderBufferImpl* m_destinationRenderBufferImpl = nullptr;
    };
}
