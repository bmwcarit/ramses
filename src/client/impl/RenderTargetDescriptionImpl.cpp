//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RenderTargetDescriptionImpl.h"
#include "impl/SceneImpl.h"
#include "impl/RenderBufferImpl.h"
#include "ramses/client/SceneObjectIterator.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    RenderTargetDescriptionImpl::RenderTargetDescriptionImpl() = default;

    RenderTargetDescriptionImpl::~RenderTargetDescriptionImpl() = default;

    void RenderTargetDescriptionImpl::validate(ValidationReportImpl& report) const
    {
        if (m_renderBuffers.empty())
        {
            report.add(EIssueType::Error, "there is no RenderBuffer added", nullptr);
        }
        else
        {
            assert(m_scene != nullptr);
            for(const auto& rb : m_renderBuffers)
            {
                if (!m_scene->getIScene().isRenderBufferAllocated(rb))
                    report.add(EIssueType::Error, "referencing one or more RenderBuffers that do not exist in scene anymore", nullptr);
            }
        }
    }

    bool RenderTargetDescriptionImpl::addRenderBuffer(const RenderBufferImpl& renderBuffer, std::string* errorMsg)
    {
        auto processError = [errorMsg](std::string_view msg) {
            LOG_ERROR(CONTEXT_CLIENT, msg);
            if (errorMsg)
                *errorMsg = msg;
            return false;
        };

        if (errorMsg)
            errorMsg->clear();

        const ramses::internal::RenderBufferHandle bufferHandle = renderBuffer.getRenderBufferHandle();
        if (contains_c(m_renderBuffers, bufferHandle))
            return processError("RenderTargetDescription::addRenderBuffer failed: trying to add a render buffer that is already contained!");

        const SceneImpl& scene = renderBuffer.getSceneImpl();
        if (m_scene != nullptr)
        {
            assert(!m_renderBuffers.empty());

            if (&scene != m_scene)
                return processError("RenderTargetDescription::addRenderBuffer failed: all render buffers must be from the same scene!");

            const ramses::internal::IScene& iscene = m_scene->getIScene();
            const ramses::internal::RenderBuffer& renderBufferData = iscene.getRenderBuffer(bufferHandle);
            const ramses::internal::RenderBuffer& existingRenderBuffer = iscene.getRenderBuffer(m_renderBuffers.front());
            if (renderBufferData.width != existingRenderBuffer.width || renderBufferData.height != existingRenderBuffer.height)
                return processError("RenderTargetDescription::addRenderBuffer failed: all render buffers must have the same resolution!");

            if (ramses::internal::IsDepthOrStencilFormat(renderBufferData.format))
            {
                for(const auto& rb : m_renderBuffers)
                {
                    if (ramses::internal::IsDepthOrStencilFormat(iscene.getRenderBuffer(rb).format))
                        return processError("RenderTargetDescription::addRenderBuffer failed: cannot add more than one depth/stencil buffer!");
                }
            }

            if (existingRenderBuffer.sampleCount != renderBufferData.sampleCount)
                return processError("RenderTargetDescription::addRenderBuffer failed: all render buffers must have same MSAA sample count!");
        }

        m_renderBuffers.push_back(bufferHandle);
        m_scene = &scene;

        return true;
    }

    const ramses::internal::RenderBufferHandleVector& RenderTargetDescriptionImpl::getRenderBuffers() const
    {
        return m_renderBuffers;
    }

}
