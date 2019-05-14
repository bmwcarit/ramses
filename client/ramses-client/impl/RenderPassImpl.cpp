//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderPassImpl.h"

#include "ramses-client-api/Camera.h"
#include "ramses-client-api/RenderTarget.h"

#include "SerializationContext.h"
#include "CameraNodeImpl.h"
#include "RenderTargetImpl.h"
#include "RenderGroupImpl.h"
#include "RamsesObjectTypeUtils.h"

#include "Scene/ClientScene.h"

namespace ramses
{
    RenderPassImpl::RenderPassImpl(SceneImpl& scene, const char* renderpassName)
        : SceneObjectImpl(scene, ERamsesObjectType_RenderPass, renderpassName)
        , m_cameraImpl(0)
        , m_renderTargetImpl(0)
    {
    }

    RenderPassImpl::~RenderPassImpl()
    {
    }

    status_t RenderPassImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_renderPassHandle;

        if (m_cameraImpl != NULL)
        {
            outStream << serializationContext.getIDForObject(m_cameraImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }

        if (m_renderTargetImpl != NULL)
        {
            outStream << serializationContext.getIDForObject(m_renderTargetImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }

        outStream << static_cast<uint32_t>(m_renderGroups.size());
        for (const auto groupImpl : m_renderGroups)
        {
            assert(0 != groupImpl);
            outStream << serializationContext.getIDForObject(groupImpl);
        }

        return StatusOK;
    }

    status_t RenderPassImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_renderPassHandle;

        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_cameraImpl);
        serializationContext.ReadDependentPointerAndStoreAsID(inStream, m_renderTargetImpl);

        uint32_t numberOfGroups = 0;
        inStream >> numberOfGroups;
        assert(m_renderGroups.empty());
        m_renderGroups.reserve(numberOfGroups);

        for (uint32_t i = 0; i < numberOfGroups; ++i)
        {
            RenderGroupImpl* groupImpl = NULL;
            serializationContext.ReadDependentPointerAndStoreAsID(inStream, groupImpl);
            m_renderGroups.push_back(groupImpl);
        }

        serializationContext.addForDependencyResolve(this);

        return StatusOK;
    }

    status_t RenderPassImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::resolveDeserializationDependencies(serializationContext));

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_cameraImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_renderTargetImpl);

        for (uint32_t i = 0; i < m_renderGroups.size(); ++i)
        {
            RenderGroupImpl* group = const_cast<RenderGroupImpl*>(m_renderGroups[i]);
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(group);
            m_renderGroups[i] = group;
        }

        return StatusOK;
    }

    status_t RenderPassImpl::validate(uint32_t indent) const
    {
        status_t status = SceneObjectImpl::validate(indent);
        indent += IndentationStep;

        if (0 == m_cameraImpl)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "renderpass does not have a camera set");
            status = getValidationErrorStatus();
        }

        if (0 == m_renderGroups.size())
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "renderpass does not contain any rendergroups");
            status = getValidationErrorStatus();
        }
        else
        {
            for(const auto& renderGroup : m_renderGroups)
            {
                if (addValidationOfDependentObject(indent, *renderGroup) != StatusOK)
                {
                    status = getValidationErrorStatus();
                }
            }
        }

        if (0 != m_renderTargetImpl)
        {
            if (addValidationOfDependentObject(indent, *m_renderTargetImpl) != StatusOK)
            {
                status = getValidationErrorStatus();
            }
        }
        else
        {
            if(getClearFlags() != ramses_internal::EClearFlags_None)
            {
                addValidationMessage(EValidationSeverity_Warning, indent, "renderpass has clear flags enabled whithout any rendertarget, clear flags will have no effect");
                status = getValidationErrorStatus();
            }
        }

        return status;
    }

    void RenderPassImpl::initializeFrameworkData()
    {
        m_renderPassHandle = getIScene().allocateRenderPass();
    }

    void RenderPassImpl::deinitializeFrameworkData()
    {
        assert(m_renderPassHandle.isValid());
        getIScene().releaseRenderPass(m_renderPassHandle);
        m_renderPassHandle = ramses_internal::RenderPassHandle::Invalid();
    }

    status_t RenderPassImpl::setCamera(const CameraNodeImpl& cameraImpl)
    {
        if (!isFromTheSameSceneAs(cameraImpl))
        {
            return addErrorEntry("RenderPass::setCamera failed - camera is not from the same scene as this RenderPass");
        }

        if (cameraImpl.isOfType(ERamsesObjectType_RemoteCamera) && 0 != m_renderTargetImpl)
        {
            // This is supposed to prevent accidental use of the wrong camera type. "Remote" camera will overwrite the camera settings with the renderer's camera
            // which is probably not desired behavior
            return addErrorEntry("RenderPass::setCamera failed - can't render into render target with a remote camera. Use perspective or orthographic camera instead.");
        }

        const status_t cameraValidity = cameraImpl.validate(0u);
        if (StatusOK == cameraValidity)
        {
            m_cameraImpl = &cameraImpl;
            getIScene().setRenderPassCamera(m_renderPassHandle, cameraImpl.getCameraHandle());
        }
        else
        {
            ramses_internal::String str = "RenderPass::setCamera failed - camera is not valid, maybe camera was not initialized:\n";
            str += cameraImpl.getValidationReport(EValidationSeverity_Warning);
            return addErrorEntry(str.c_str());
        }

        return cameraValidity;
    }

    const Camera* RenderPassImpl::getCamera() const
    {
        if (m_cameraImpl != NULL)
        {
            return &RamsesObjectTypeUtils::ConvertTo<Camera>(m_cameraImpl->getRamsesObject());
        }

        return NULL;
    }

    status_t RenderPassImpl::setClearColor(const ramses_internal::Vector4& clearColor)
    {
        getIScene().setRenderPassClearColor(m_renderPassHandle, clearColor);
        return StatusOK;
    }

    const ramses_internal::Vector4& RenderPassImpl::getClearColor() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).clearColor;
    }

    status_t RenderPassImpl::setClearFlags(uint32_t clearFlags)
    {
        getIScene().setRenderPassClearFlag(m_renderPassHandle, clearFlags);
        return StatusOK;
    }

    uint32_t RenderPassImpl::getClearFlags() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).clearFlags;
    }

    status_t RenderPassImpl::addRenderGroup(const RenderGroupImpl& renderGroup, int32_t orderWithinPass)
    {
        if (!isFromTheSameSceneAs(renderGroup))
        {
            return addErrorEntry("RenderPass::addRenderGroup failed - renderGroup is not from the same scene as this RenderPass");
        }

        if (!ramses_internal::contains_c(m_renderGroups, &renderGroup))
        {
            const ramses_internal::RenderGroupHandle renderGroupHandle = renderGroup.getRenderGroupHandle();
            getIScene().addRenderGroupToRenderPass(m_renderPassHandle, renderGroupHandle, orderWithinPass);
            m_renderGroups.push_back(&renderGroup);
        }

        return StatusOK;
    }

    status_t RenderPassImpl::remove(const RenderGroupImpl& renderGroup)
    {
        RenderGroupVector::iterator iter = ramses_internal::find_c(m_renderGroups, &renderGroup);
        if (iter == m_renderGroups.end())
        {
            return addErrorEntry("RenderPass::removeRenderGroup failed - could not remove RenderGroup from Renderpass because it was not contained");
        }

        removeInternal(iter);

        return StatusOK;
    }

    void RenderPassImpl::removeIfContained(const RenderGroupImpl& renderGroup)
    {
        RenderGroupVector::iterator iter = ramses_internal::find_c(m_renderGroups, &renderGroup);
        if (iter != m_renderGroups.end())
        {
            removeInternal(iter);
        }
    }

    bool RenderPassImpl::contains(const RenderGroupImpl& renderGroup) const
    {
        return ramses_internal::contains_c(m_renderGroups, &renderGroup);
    }

    ramses::status_t RenderPassImpl::getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinPass) const
    {
        if (!contains(renderGroup))
        {
            return addErrorEntry("RenderPass::getRenderGroupOrder failed - render group not contained in RenderPass");
        }

        const ramses_internal::RenderGroupHandle renderGroupHandle = renderGroup.getRenderGroupHandle();
        const ramses_internal::RenderPass& internalRP = getIScene().getRenderPass(m_renderPassHandle);
        for (const auto& rgEntry : internalRP.renderGroups)
        {
            if (rgEntry.renderGroup == renderGroupHandle)
            {
                orderWithinPass = rgEntry.order;
                return StatusOK;
            }
        }

        assert(false);
        return addErrorEntry("RenderPass::getRenderGroupOrder failed - fatal, render group not found in internal render pass");
    }

    const RenderGroupVector& RenderPassImpl::getAllRenderGroups() const
    {
        return m_renderGroups;
    }

    status_t RenderPassImpl::removeAllRenderGroups()
    {
        while (!m_renderGroups.empty())
        {
            CHECK_RETURN_ERR(remove(*m_renderGroups.front()));
        }

        return StatusOK;
    }

    ramses_internal::RenderPassHandle RenderPassImpl::getRenderPassHandle() const
    {
        return m_renderPassHandle;
    }

    status_t RenderPassImpl::setRenderTarget(RenderTargetImpl* renderTargetImpl)
    {
        if (renderTargetImpl == m_renderTargetImpl)
        {
            return StatusOK;
        }

        ramses_internal::RenderTargetHandle rtHandle(ramses_internal::RenderTargetHandle::Invalid());
        if (0 != renderTargetImpl)
        {
            if (NULL == m_cameraImpl || !m_cameraImpl->isOfType(ERamsesObjectType_LocalCamera))
            {
                //(Violin) This error message is supposed to prevent the user from rendering into a render target with a remote camera (renderer's camera)
                return addErrorEntry("RenderPass::setRenderTarget failed - must explicitly assign a custom camera (perspective or orthographic) before rendering to render terget.");
            }

            if (!isFromTheSameSceneAs(*renderTargetImpl))
            {
                return addErrorEntry("RenderPass::setRenderTarget failed - renderTarget is not from the same scene as this RenderPass.");
            }

            rtHandle = renderTargetImpl->getRenderTargetHandle();
        }

        m_renderTargetImpl = renderTargetImpl;
        getIScene().setRenderPassRenderTarget(m_renderPassHandle, rtHandle);

        return StatusOK;
    }

    const RenderTarget* RenderPassImpl::getRenderTarget() const
    {
        if (m_renderTargetImpl != NULL)
        {
            return &RamsesObjectTypeUtils::ConvertTo<RenderTarget>(m_renderTargetImpl->getRamsesObject());
        }

        return NULL;
    }

    status_t RenderPassImpl::setRenderOrder(int32_t renderOrder)
    {
        getIScene().setRenderPassRenderOrder(m_renderPassHandle, renderOrder);
        return StatusOK;
    }

    int32_t RenderPassImpl::getRenderOrder() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).renderOrder;
    }

    status_t RenderPassImpl::setEnabled(bool isEnabeld)
    {
        getIScene().setRenderPassEnabled(m_renderPassHandle, isEnabeld);
        return StatusOK;
    }

    bool RenderPassImpl::isEnabled() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).isEnabled;
    }

    void RenderPassImpl::removeInternal(RenderGroupVector::iterator iter)
    {
        const ramses_internal::RenderGroupHandle renderGroupHandle = (*iter)->getRenderGroupHandle();
        getIScene().removeRenderGroupFromRenderPass(m_renderPassHandle, renderGroupHandle);
        m_renderGroups.erase(iter);
    }

    status_t RenderPassImpl::setRenderOnce(bool enable)
    {
        getIScene().setRenderPassRenderOnce(m_renderPassHandle, enable);
        return StatusOK;
    }

    bool RenderPassImpl::isRenderOnce() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).isRenderOnce;
    }

    status_t RenderPassImpl::retriggerRenderOnce()
    {
        if (!isRenderOnce())
        {
            return addErrorEntry("RenderPass::retriggerRenderOnce - cannot retrigger rendering of render pass that does not have render once flag set");
        }

        getIScene().retriggerRenderPassRenderOnce(m_renderPassHandle);
        return StatusOK;
    }
}
