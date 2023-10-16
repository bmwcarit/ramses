//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <string_view>

namespace ramses::internal
{
    class RenderTargetDescriptionImpl;

    class RenderTargetImpl final : public SceneObjectImpl
    {
    public:
        RenderTargetImpl(SceneImpl& scene, std::string_view name);
        ~RenderTargetImpl() override;

        void initializeFrameworkData(const RenderTargetDescriptionImpl& rtDesc);
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] uint32_t getWidth() const;
        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] ramses::internal::RenderTargetHandle getRenderTargetHandle() const;

    private:
        ramses::internal::RenderTargetHandle m_renderTargetHandle;
    };
}
