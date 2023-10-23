//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneTypes.h"

namespace ramses::internal
{
    class SceneImpl;
    class RenderBufferImpl;
    class ValidationReportImpl;

    class RenderTargetDescriptionImpl
    {
    public:
        RenderTargetDescriptionImpl();
        ~RenderTargetDescriptionImpl();

        void validate(ValidationReportImpl& report) const;

        [[nodiscard]] bool addRenderBuffer(const RenderBufferImpl& renderBuffer, std::string* errorMsg);

        [[nodiscard]] const ramses::internal::RenderBufferHandleVector& getRenderBuffers() const;

    private:
        const SceneImpl* m_scene{nullptr};
        ramses::internal::RenderBufferHandleVector m_renderBuffers{};
    };
}
