//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RenderPassImpl.h"

#include "ramses/client/Camera.h"
#include "ramses/client/RenderTarget.h"

#include "impl/SerializationContext.h"
#include "impl/CameraNodeImpl.h"
#include "impl/RenderTargetImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/ErrorReporting.h"

#include "internal/SceneGraph/Scene/ClientScene.h"

#include <string>

namespace ramses::internal
{
    RenderPassImpl::RenderPassImpl(SceneImpl& scene, std::string_view renderpassName)
        : SceneObjectImpl(scene, ERamsesObjectType::RenderPass, renderpassName)
        , m_cameraImpl(nullptr)
        , m_renderTargetImpl(nullptr)
    {
    }

    RenderPassImpl::~RenderPassImpl() = default;

    bool RenderPassImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_renderPassHandle;

        if (m_cameraImpl != nullptr)
        {
            outStream << serializationContext.getIDForObject(m_cameraImpl);
        }
        else
        {
            outStream << SerializationContext::GetObjectIDNull();
        }

        if (m_renderTargetImpl != nullptr)
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
            assert(nullptr != groupImpl);
            outStream << serializationContext.getIDForObject(groupImpl);
        }

        return true;
    }

    bool RenderPassImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        serializationContext.deserializeAndMap(inStream, m_renderPassHandle);

        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_cameraImpl);
        DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, m_renderTargetImpl);

        uint32_t numberOfGroups = 0;
        inStream >> numberOfGroups;
        assert(m_renderGroups.empty());
        m_renderGroups.reserve(numberOfGroups);

        for (uint32_t i = 0; i < numberOfGroups; ++i)
        {
            RenderGroupImpl* groupImpl = nullptr;
            DeserializationContext::ReadDependentPointerAndStoreAsID(inStream, groupImpl);
            m_renderGroups.push_back(groupImpl);
        }

        serializationContext.addForDependencyResolve(this);

        return true;
    }

    bool RenderPassImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::resolveDeserializationDependencies(serializationContext))
            return false;

        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_cameraImpl);
        serializationContext.resolveDependencyIDImplAndStoreAsPointer(m_renderTargetImpl);

        for (auto& renderGroup : m_renderGroups)
        {
            serializationContext.resolveDependencyIDImplAndStoreAsPointer(renderGroup);
        }

        return true;
    }

    void RenderPassImpl::onValidate(ValidationReportImpl& report) const
    {
        SceneObjectImpl::onValidate(report);

        if (nullptr == m_cameraImpl)
        {
            report.add(EIssueType::Warning, "renderpass does not have a camera set", &getRamsesObject());
        }

        if (m_renderGroups.empty())
        {
            report.add(EIssueType::Warning, "renderpass does not contain any rendergroups", &getRamsesObject());
        }
        else
        {
            for (const auto& renderGroup : m_renderGroups)
                report.addDependentObject(*this, *renderGroup);
        }

        if (nullptr != m_renderTargetImpl)
        {
            report.addDependentObject(*this, *m_renderTargetImpl);
        }
        else if (getClearFlags() != EClearFlag::None)
        {
            report.add(EIssueType::Warning, "renderpass has clear flags enabled whithout any rendertarget, clear flags will have no effect", &getRamsesObject());
        }
    }

    void RenderPassImpl::initializeFrameworkData()
    {
        m_renderPassHandle = getIScene().allocateRenderPass(0, {});
    }

    void RenderPassImpl::deinitializeFrameworkData()
    {
        assert(m_renderPassHandle.isValid());
        getIScene().releaseRenderPass(m_renderPassHandle);
        m_renderPassHandle = ramses::internal::RenderPassHandle::Invalid();
    }

    bool RenderPassImpl::setCamera(const CameraNodeImpl& cameraImpl)
    {
        if (!isFromTheSameSceneAs(cameraImpl))
        {
            getErrorReporting().set("RenderPass::setCamera failed - camera is not from the same scene as this RenderPass", *this);
            return false;
        }
        ValidationReportImpl cameraReport;
        cameraImpl.validate(cameraReport);
        if (!cameraReport.hasError())
        {
            m_cameraImpl = &cameraImpl;
            getIScene().setRenderPassCamera(m_renderPassHandle, cameraImpl.getCameraHandle());
        }
        else
        {
            getErrorReporting().set(fmt::format("RenderPass::setCamera failed - camera is not valid, maybe camera was not initialized:\n{}", cameraReport.toString()), *this);
            return false;
        }

        return true;
    }

    const ramses::Camera* RenderPassImpl::getCamera() const
    {
        if (m_cameraImpl != nullptr)
        {
            return &RamsesObjectTypeUtils::ConvertTo<ramses::Camera>(m_cameraImpl->getRamsesObject());
        }

        return nullptr;
    }

    ramses::Camera* RenderPassImpl::getCamera()
    {
        // non-const version of getCamera cast to its const version to avoid duplicating code
        return const_cast<ramses::Camera*>((const_cast<const RenderPassImpl&>(*this)).getCamera());
    }

    bool RenderPassImpl::setClearColor(const glm::vec4& clearColor)
    {
        getIScene().setRenderPassClearColor(m_renderPassHandle, clearColor);
        return true;
    }

    const glm::vec4& RenderPassImpl::getClearColor() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).clearColor;
    }

    bool RenderPassImpl::setClearFlags(ClearFlags clearFlags)
    {
        getIScene().setRenderPassClearFlag(m_renderPassHandle, clearFlags);
        return true;
    }

    ClearFlags RenderPassImpl::getClearFlags() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).clearFlags;
    }

    bool RenderPassImpl::addRenderGroup(const RenderGroupImpl& renderGroup, int32_t orderWithinPass)
    {
        if (!isFromTheSameSceneAs(renderGroup))
        {
            getErrorReporting().set("RenderPass::addRenderGroup failed - renderGroup is not from the same scene as this RenderPass", *this);
            return false;
        }

        if (!ramses::internal::contains_c(m_renderGroups, &renderGroup))
        {
            const ramses::internal::RenderGroupHandle renderGroupHandle = renderGroup.getRenderGroupHandle();
            getIScene().addRenderGroupToRenderPass(m_renderPassHandle, renderGroupHandle, orderWithinPass);
            m_renderGroups.push_back(&renderGroup);
        }

        return true;
    }

    bool RenderPassImpl::remove(const RenderGroupImpl& renderGroup)
    {
        auto iter = ramses::internal::find_c(m_renderGroups, &renderGroup);
        if (iter == m_renderGroups.end())
        {
            getErrorReporting().set("RenderPass::removeRenderGroup failed - could not remove RenderGroup from Renderpass because it was not contained", *this);
            return false;
        }

        removeInternal(iter);

        return true;
    }

    void RenderPassImpl::removeIfContained(const RenderGroupImpl& renderGroup)
    {
        auto iter = ramses::internal::find_c(m_renderGroups, &renderGroup);
        if (iter != m_renderGroups.end())
        {
            removeInternal(iter);
        }
    }

    bool RenderPassImpl::contains(const RenderGroupImpl& renderGroup) const
    {
        return ramses::internal::contains_c(m_renderGroups, &renderGroup);
    }

    bool RenderPassImpl::getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinPass) const
    {
        if (!contains(renderGroup))
        {
            getErrorReporting().set("RenderPass::getRenderGroupOrder failed - render group not contained in RenderPass", *this);
            return false;
        }

        const ramses::internal::RenderGroupHandle renderGroupHandle = renderGroup.getRenderGroupHandle();
        const ramses::internal::RenderPass& internalRP = getIScene().getRenderPass(m_renderPassHandle);
        for (const auto& rgEntry : internalRP.renderGroups)
        {
            if (rgEntry.renderGroup == renderGroupHandle)
            {
                orderWithinPass = rgEntry.order;
                return true;
            }
        }

        getErrorReporting().set("RenderPass::getRenderGroupOrder failed - fatal, render group not found in internal render pass", *this);
        assert(false);
        return false;
    }

    const RenderGroupVector& RenderPassImpl::getAllRenderGroups() const
    {
        return m_renderGroups;
    }

    bool RenderPassImpl::removeAllRenderGroups()
    {
        while (!m_renderGroups.empty())
        {
            if (!remove(*m_renderGroups.front()))
                return false;
        }

        return true;
    }

    ramses::internal::RenderPassHandle RenderPassImpl::getRenderPassHandle() const
    {
        return m_renderPassHandle;
    }

    bool RenderPassImpl::setRenderTarget(RenderTargetImpl* renderTargetImpl)
    {
        if (renderTargetImpl == m_renderTargetImpl)
            return true;

        ramses::internal::RenderTargetHandle rtHandle(ramses::internal::RenderTargetHandle::Invalid());
        if (nullptr != renderTargetImpl)
        {
            if (nullptr == m_cameraImpl)
            {
                getErrorReporting().set("RenderPass::setRenderTarget failed - must explicitly assign a custom camera (perspective or orthographic) before rendering to render terget.", *this);
                return false;
            }

            if (!isFromTheSameSceneAs(*renderTargetImpl))
            {
                getErrorReporting().set("RenderPass::setRenderTarget failed - renderTarget is not from the same scene as this RenderPass.", *this);
                return false;
            }

            rtHandle = renderTargetImpl->getRenderTargetHandle();
        }

        m_renderTargetImpl = renderTargetImpl;
        getIScene().setRenderPassRenderTarget(m_renderPassHandle, rtHandle);

        return true;
    }

    const ramses::RenderTarget* RenderPassImpl::getRenderTarget() const
    {
        if (m_renderTargetImpl != nullptr)
        {
            return &RamsesObjectTypeUtils::ConvertTo< ramses::RenderTarget>(m_renderTargetImpl->getRamsesObject());
        }

        return nullptr;
    }

    bool RenderPassImpl::setRenderOrder(int32_t renderOrder)
    {
        getIScene().setRenderPassRenderOrder(m_renderPassHandle, renderOrder);
        return true;
    }

    int32_t RenderPassImpl::getRenderOrder() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).renderOrder;
    }

    bool RenderPassImpl::setEnabled(bool isEnabeld)
    {
        getIScene().setRenderPassEnabled(m_renderPassHandle, isEnabeld);
        return true;
    }

    bool RenderPassImpl::isEnabled() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).isEnabled;
    }

    void RenderPassImpl::removeInternal(RenderGroupVector::iterator iter)
    {
        const ramses::internal::RenderGroupHandle renderGroupHandle = (*iter)->getRenderGroupHandle();
        getIScene().removeRenderGroupFromRenderPass(m_renderPassHandle, renderGroupHandle);
        m_renderGroups.erase(iter);
    }

    bool RenderPassImpl::setRenderOnce(bool enable)
    {
        getIScene().setRenderPassRenderOnce(m_renderPassHandle, enable);
        return true;
    }

    bool RenderPassImpl::isRenderOnce() const
    {
        return getIScene().getRenderPass(m_renderPassHandle).isRenderOnce;
    }

    bool RenderPassImpl::retriggerRenderOnce()
    {
        if (!isRenderOnce())
        {
            getErrorReporting().set("RenderPass::retriggerRenderOnce - cannot retrigger rendering of render pass that does not have render once flag set", *this);
            return false;
        }

        getIScene().retriggerRenderPassRenderOnce(m_renderPassHandle);
        return true;
    }
}
