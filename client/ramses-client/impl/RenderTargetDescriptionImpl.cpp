//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderTargetDescriptionImpl.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "SceneImpl.h"
#include "RenderBufferImpl.h"
#include "Scene/ClientScene.h"

namespace ramses
{
    RenderTargetDescriptionImpl::RenderTargetDescriptionImpl()
        : m_scene(NULL)
    {
    }

    RenderTargetDescriptionImpl::~RenderTargetDescriptionImpl()
    {
    }

    status_t RenderTargetDescriptionImpl::validate(uint32_t indent) const
    {
        status_t status = StatusObjectImpl::validate(indent);
        indent += IndentationStep;

        if (m_renderBuffers.empty())
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "there is no RenderBuffer added");
            status = getValidationErrorStatus();
        }
        else
        {
            assert(m_scene != NULL);
            for(const auto& rb : m_renderBuffers)
            {
                if (!m_scene->getIScene().isRenderBufferAllocated(rb))
                {
                    addValidationMessage(EValidationSeverity_Error, indent, "referencing one or more RenderBuffers that do not exist in scene anymore");
                    status = getValidationErrorStatus();
                }
            }
        }

        return status;
    }

    status_t RenderTargetDescriptionImpl::addRenderBuffer(const RenderBufferImpl& renderBuffer)
    {
        const ramses_internal::RenderBufferHandle bufferHandle = renderBuffer.getRenderBufferHandle();
        if (contains_c(m_renderBuffers, bufferHandle))
        {
            return addErrorEntry("RenderTargetDescription::addRenderBuffer failed: trying to add a render buffer that is already contained!");
        }

        const SceneImpl& scene = renderBuffer.getSceneImpl();
        if (m_scene != NULL)
        {
            assert(!m_renderBuffers.empty());

            if (&scene != m_scene)
            {
                return addErrorEntry("RenderTargetDescription::addRenderBuffer failed: all render buffers must be from the same scene!");
            }

            const ramses_internal::IScene& iscene = m_scene->getIScene();
            const ramses_internal::RenderBuffer& renderBufferData = iscene.getRenderBuffer(bufferHandle);
            const ramses_internal::RenderBuffer& existingRenderBuffer = iscene.getRenderBuffer(m_renderBuffers.front());
            if (renderBufferData.width != existingRenderBuffer.width || renderBufferData.height != existingRenderBuffer.height)
            {
                return addErrorEntry("RenderTargetDescription::addRenderBuffer failed: all render buffers must have the same resolution!");
            }

            if (renderBufferData.type == ramses_internal::ERenderBufferType_DepthBuffer || renderBufferData.type == ramses_internal::ERenderBufferType_DepthStencilBuffer)
            {
                for(const auto& rb : m_renderBuffers)
                {
                    const ramses_internal::ERenderBufferType rbType = iscene.getRenderBuffer(rb).type;
                    if (rbType == ramses_internal::ERenderBufferType_DepthBuffer || rbType == ramses_internal::ERenderBufferType_DepthStencilBuffer)
                    {
                        return addErrorEntry("RenderTargetDescription::addRenderBuffer failed: cannot add more than one depth/stencil buffer!");
                    }
                }
            }

            if (existingRenderBuffer.sampleCount != renderBufferData.sampleCount)
            {
                return addErrorEntry("RenderTargetDescription::addRenderBuffer failed: all render buffers must have same MSAA sample count!");
            }
        }

        m_renderBuffers.push_back(bufferHandle);
        m_scene = &scene;

        return StatusOK;
    }

    const ramses_internal::RenderBufferHandleVector& RenderTargetDescriptionImpl::getRenderBuffers() const
    {
        return m_renderBuffers;
    }

}
