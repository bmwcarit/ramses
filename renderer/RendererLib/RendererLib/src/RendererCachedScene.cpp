//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/RenderableComparator.h"
#include "FrameBufferInfo.h"
#include "RenderingPassOrderComparator.h"
#include <algorithm>

namespace ramses_internal
{
    RendererCachedScene::RendererCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : TextureLinkCachedScene(sceneLinksManager, sceneInfo)
        , m_renderableOrderingDirty(true)
    {
    }

    void RendererCachedScene::setRenderableVisibility(RenderableHandle renderableHandle, Bool visible)
    {
        TextureLinkCachedScene::setRenderableVisibility(renderableHandle, visible);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        TextureLinkCachedScene::releaseRenderGroup(groupHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order)
    {
        TextureLinkCachedScene::addRenderableToRenderGroup(groupHandle, renderableHandle, order);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        TextureLinkCachedScene::removeRenderableFromRenderGroup(groupHandle, renderableHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::releaseRenderPass(RenderPassHandle passHandle)
    {
        m_renderOncePassesToRender.remove(passHandle);
        TextureLinkCachedScene::releaseRenderPass(passHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setRenderPassRenderOrder(RenderPassHandle passHandle, Int32 renderOrder)
    {
        TextureLinkCachedScene::setRenderPassRenderOrder(passHandle, renderOrder);
        m_renderableOrderingDirty = true;
    }

    BlitPassHandle RendererCachedScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle /*= BlitPassHandle::Invalid()*/)
    {
        const BlitPassHandle blitPass = TextureLinkCachedScene::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);
        m_renderableOrderingDirty = true;

        return blitPass;
    }

    void RendererCachedScene::releaseBlitPass(BlitPassHandle passHandle)
    {
        TextureLinkCachedScene::releaseBlitPass(passHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder)
    {
        TextureLinkCachedScene::setBlitPassRenderOrder(passHandle, renderOrder);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setBlitPassEnabled(BlitPassHandle passHandle, Bool isEnabled)
    {
        TextureLinkCachedScene::setBlitPassEnabled(passHandle, isEnabled);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setRenderPassEnabled(RenderPassHandle passHandle, Bool isEnabled)
    {
        TextureLinkCachedScene::setRenderPassEnabled(passHandle, isEnabled);
        if (isEnabled && TextureLinkCachedScene::getRenderPass(passHandle).isRenderOnce)
        {
            m_renderOncePassesToRender.put(passHandle);
        }
        else
        {
            m_renderOncePassesToRender.remove(passHandle);
        }
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setRenderPassRenderOnce(RenderPassHandle passHandle, Bool enable)
    {
        TextureLinkCachedScene::setRenderPassRenderOnce(passHandle, enable);
        if (enable && TextureLinkCachedScene::getRenderPass(passHandle).isEnabled)
        {
            m_renderOncePassesToRender.put(passHandle);
        }
        else
        {
            m_renderOncePassesToRender.remove(passHandle);
        }
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::retriggerRenderPassRenderOnce(RenderPassHandle passHandle)
    {
        TextureLinkCachedScene::retriggerRenderPassRenderOnce(passHandle);
        if (TextureLinkCachedScene::getRenderPass(passHandle).isEnabled)
        {
            m_renderOncePassesToRender.put(passHandle);
            m_renderableOrderingDirty = true;
        }
    }

    void RendererCachedScene::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order)
    {
        TextureLinkCachedScene::addRenderGroupToRenderPass(passHandle, groupHandle, order);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        TextureLinkCachedScene::removeRenderGroupFromRenderPass(passHandle, groupHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order)
    {
        TextureLinkCachedScene::addRenderGroupToRenderGroup(groupHandleParent, groupHandleChild, order);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        TextureLinkCachedScene::removeRenderGroupFromRenderGroup(groupHandleParent, groupHandleChild);
        m_renderableOrderingDirty = true;
    }

    const RenderingPassInfoVector& RendererCachedScene::getSortedRenderingPasses() const
    {
        return m_sortedRenderingPasses;
    }

    const RenderableVector& RendererCachedScene::getOrderedRenderablesForPass(RenderPassHandle pass) const
    {
        assert(pass.asMemoryHandle() < m_passRenderableOrder.size());
        return m_passRenderableOrder[pass.asMemoryHandle()];
    }

    void RendererCachedScene::updateRenderablesAndResourceCache(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager)
    {
        updateRenderableResources(resourceAccessor, embeddedCompositingManager);
        updatePassRenderableSorting();
    }

    void RendererCachedScene::updatePassRenderableSorting()
    {
        if (m_renderableOrderingDirty)
        {
            m_sortedRenderingPasses.clear();

            const UInt32 totalNumberOfRenderPasses = TextureLinkCachedScene::getRenderPassCount();
            const UInt32 totalNumberOfBlitPasses = TextureLinkCachedScene::getBlitPassCount();

            //add render passes
            m_passRenderableOrder.resize(totalNumberOfRenderPasses);
            for (RenderPassHandle passHandle(0); passHandle < totalNumberOfRenderPasses; ++passHandle)
            {
                m_passRenderableOrder[passHandle.asMemoryHandle()].clear();
                if (shouldRenderPassBeRendered(passHandle))
                    m_sortedRenderingPasses.emplace_back(passHandle);
            }

            //add blit passes
            for (BlitPassHandle passHandle(0); passHandle < totalNumberOfBlitPasses; ++passHandle)
            {
                if (TextureLinkCachedScene::isBlitPassAllocated(passHandle))
                {
                    const BlitPass& blitPass = TextureLinkCachedScene::getBlitPass(passHandle);
                    if (blitPass.isEnabled)
                    {
                        assert(blitPass.sourceRenderBuffer.isValid());
                        assert(blitPass.destinationRenderBuffer.isValid());
                        m_sortedRenderingPasses.emplace_back(passHandle);
                    }
                }
            }

            //sort
            RenderingPassOrderComparator comparator(*this);
            std::sort(m_sortedRenderingPasses.begin(), m_sortedRenderingPasses.end(), comparator);

            //update renderables according to sorted render passes
            for (const auto& pass : m_sortedRenderingPasses)
            {
                if (ERenderingPassType::RenderPass == pass.getType())
                    updateRenderablesInPass(pass.getRenderPassHandle());
            }

            m_renderableOrderingDirty = false;
        }
    }

    const Matrix44f& RendererCachedScene::getRenderableWorldMatrix(RenderableHandle renderable) const
    {
        assert(renderable.asMemoryHandle() < m_renderableMatrices.size());
        return m_renderableMatrices[renderable.asMemoryHandle()];
    }

    void RendererCachedScene::updateRenderablesInPass(RenderPassHandle passHandle)
    {
        RenderableVector& orderedRenderables = m_passRenderableOrder[passHandle.asMemoryHandle()];

        // we sort in-place in scene's RenderPass, although we don't have to but it might speed up sorting if topology/order changes frequently
        RenderGroupOrderVector& orderedRenderGroups = getRenderPassInternal(passHandle).renderGroups;

        std::sort(orderedRenderGroups.begin(), orderedRenderGroups.end());

        for(const auto& renderGroup : orderedRenderGroups)
        {
            addRenderablesFromRenderGroup(orderedRenderables, renderGroup.renderGroup);
        }
    }

    static void AddRenderable(const IScene& scene, RenderableVector& orderedRenderables, RenderableHandle renderable)
    {
        if (scene.getRenderable(renderable).isVisible)
        {
            orderedRenderables.push_back(renderable);
        }
    }

    void RendererCachedScene::addRenderablesFromRenderGroup(RenderableVector& orderedRenderables, RenderGroupHandle renderGroupHandle)
    {
        assert(isRenderGroupAllocated(renderGroupHandle));

        RenderGroup& renderGroup = getRenderGroupInternal(renderGroupHandle);
        // we sort in-place in scene's TopologyRenderGroup, although we don't have to but it might speed up sorting if topology/order changes frequently
        RenderableOrderVector& orderedGroupRenderables = renderGroup.renderables;
        RenderGroupOrderVector& orderedRenderGroups = renderGroup.renderGroups;

        RenderableComparator renderableComp(*this);
        std::sort(orderedGroupRenderables.begin(), orderedGroupRenderables.end(), renderableComp);
        std::sort(orderedRenderGroups.begin(), orderedRenderGroups.end());

        RenderableOrderVector::iterator renderablesIterator = orderedGroupRenderables.begin();
        RenderGroupOrderVector::iterator renderGroupIterator = orderedRenderGroups.begin();
        while (renderablesIterator != orderedGroupRenderables.end()
            || renderGroupIterator != orderedRenderGroups.end())
        {
            if (renderGroupIterator == orderedRenderGroups.end())
            {
                AddRenderable(*this, orderedRenderables, renderablesIterator->renderable);
                ++renderablesIterator;
            }
            else if (renderablesIterator == orderedGroupRenderables.end())
            {
                addRenderablesFromRenderGroup(orderedRenderables, renderGroupIterator->renderGroup);
                ++renderGroupIterator;
            }
            else
            {
                if (renderablesIterator->order < renderGroupIterator->order)
                {
                    AddRenderable(*this, orderedRenderables, renderablesIterator->renderable);
                    ++renderablesIterator;
                }
                else
                {
                    addRenderablesFromRenderGroup(orderedRenderables, renderGroupIterator->renderGroup);
                    ++renderGroupIterator;
                }
            }
        }
    }

    void RendererCachedScene::updateRenderableWorldMatrices()
    {
        m_renderableMatrices.resize(TextureLinkCachedScene::getRenderableCount());
        for (const auto& renderables : m_passRenderableOrder)
        {
            for (const auto renderable : renderables)
            {
                assert(renderable.isValid());
                const NodeHandle node = getRenderable(renderable).node;
                assert(node.isValid());
                m_renderableMatrices[renderable.asMemoryHandle()] = updateMatrixCache(ETransformationMatrixType_World, node);
            }
        }
    }

    void RendererCachedScene::updateRenderableWorldMatricesWithLinks()
    {
        m_renderableMatrices.resize(TextureLinkCachedScene::getRenderableCount());
        for (const auto& renderables : m_passRenderableOrder)
        {
            for (const auto renderable : renderables)
            {
                assert(renderable.isValid());
                const NodeHandle node = TextureLinkCachedScene::getRenderable(renderable).node;
                assert(node.isValid());
                m_renderableMatrices[renderable.asMemoryHandle()] = updateMatrixCacheWithLinks(ETransformationMatrixType_World, node);
            }
        }
    }

    Bool RendererCachedScene::shouldRenderPassBeRendered(RenderPassHandle handle) const
    {
        if (!TextureLinkCachedScene::isRenderPassAllocated(handle))
        {
            return false;
        }

        const RenderPass& rp = TextureLinkCachedScene::getRenderPass(handle);
        if (!rp.camera.isValid() || !rp.isEnabled)
        {
            return false;
        }

        return !rp.isRenderOnce || m_renderOncePassesToRender.hasElement(handle);
    }

    void RendererCachedScene::retriggerAllRenderOncePasses()
    {
        const UInt32 numRPs = TextureLinkCachedScene::getRenderPassCount();
        for (RenderPassHandle handle(0u); handle < numRPs; ++handle)
        {
            if (TextureLinkCachedScene::isRenderPassAllocated(handle))
            {
                const auto& rp = getRenderPass(handle);
                if (rp.isEnabled && rp.isRenderOnce)
                {
                    m_renderOncePassesToRender.put(handle);
                }
            }
        }

        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::markAllRenderOncePassesAsRendered() const
    {
        if (m_renderOncePassesToRender.count() > 0u)
        {
            // some render once passes were rendered, remove them from list
            // and force update of cached render pass list for next update
            m_renderOncePassesToRender.clear();
            m_renderableOrderingDirty = true;
        }
    }
}
