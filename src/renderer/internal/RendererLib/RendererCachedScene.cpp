//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/RenderableComparator.h"
#include "RenderingPassOrderComparator.h"
#include <algorithm>

namespace ramses::internal
{
    RendererCachedScene::RendererCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo)
        : BaseT(sceneLinksManager, sceneInfo)
        , m_renderableOrderingDirty(true)
    {
    }

    void RendererCachedScene::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visible)
    {
        BaseT::setRenderableVisibility(renderableHandle, visible);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        BaseT::releaseRenderGroup(groupHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order)
    {
        BaseT::addRenderableToRenderGroup(groupHandle, renderableHandle, order);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        BaseT::removeRenderableFromRenderGroup(groupHandle, renderableHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::releaseRenderPass(RenderPassHandle passHandle)
    {
        m_renderOncePassesToRender.remove(passHandle);
        BaseT::releaseRenderPass(passHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setRenderPassRenderOrder(RenderPassHandle passHandle, int32_t renderOrder)
    {
        BaseT::setRenderPassRenderOrder(passHandle, renderOrder);
        m_renderableOrderingDirty = true;
    }

    BlitPassHandle RendererCachedScene::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle)
    {
        const BlitPassHandle blitPass = BaseT::allocateBlitPass(sourceRenderBufferHandle, destinationRenderBufferHandle, passHandle);
        m_renderableOrderingDirty = true;

        return blitPass;
    }

    void RendererCachedScene::releaseBlitPass(BlitPassHandle passHandle)
    {
        BaseT::releaseBlitPass(passHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setBlitPassRenderOrder(BlitPassHandle passHandle, int32_t renderOrder)
    {
        BaseT::setBlitPassRenderOrder(passHandle, renderOrder);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled)
    {
        BaseT::setBlitPassEnabled(passHandle, isEnabled);
        m_renderableOrderingDirty = true;
    }

    TextureBufferHandle RendererCachedScene::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle)
    {
        auto resultHandle = BaseT::allocateTextureBuffer(textureFormat, mipMapDimensions, handle);
        m_textureBufferUpdates.resize(getTextureBufferCount());
        auto& update = m_textureBufferUpdates[resultHandle.asMemoryHandle()];
        update.resize(mipMapDimensions.size());
        return resultHandle;
    }

    void RendererCachedScene::releaseTextureBuffer(TextureBufferHandle handle)
    {
        TextureLinkCachedScene::releaseTextureBuffer(handle);
        m_textureBufferUpdates[handle.asMemoryHandle()].clear();
    }

    void RendererCachedScene::updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data)
    {
        BaseT::updateTextureBuffer(handle, mipLevel, x, y, width, height, data);
        assert(handle.asMemoryHandle() < m_textureBufferUpdates.size());
        auto& update = m_textureBufferUpdates[handle.asMemoryHandle()];
        assert(mipLevel < update.size());
        auto& area = update[mipLevel];
        area = area.getBoundingQuad(Quad{static_cast<int32_t>(x), static_cast<int32_t>(y), static_cast<int32_t>(width), static_cast<int32_t>(height)});
    }

    void RendererCachedScene::setRenderPassEnabled(RenderPassHandle passHandle, bool isEnabled)
    {
        BaseT::setRenderPassEnabled(passHandle, isEnabled);
        if (isEnabled && BaseT::getRenderPass(passHandle).isRenderOnce)
        {
            m_renderOncePassesToRender.put(passHandle);
        }
        else
        {
            m_renderOncePassesToRender.remove(passHandle);
        }
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::setRenderPassRenderOnce(RenderPassHandle passHandle, bool enable)
    {
        BaseT::setRenderPassRenderOnce(passHandle, enable);
        if (enable && BaseT::getRenderPass(passHandle).isEnabled)
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
        BaseT::retriggerRenderPassRenderOnce(passHandle);
        if (BaseT::getRenderPass(passHandle).isEnabled)
        {
            m_renderOncePassesToRender.put(passHandle);
            m_renderableOrderingDirty = true;
        }
    }

    void RendererCachedScene::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order)
    {
        BaseT::addRenderGroupToRenderPass(passHandle, groupHandle, order);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        BaseT::removeRenderGroupFromRenderPass(passHandle, groupHandle);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order)
    {
        BaseT::addRenderGroupToRenderGroup(groupHandleParent, groupHandleChild, order);
        m_renderableOrderingDirty = true;
    }

    void RendererCachedScene::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        BaseT::removeRenderGroupFromRenderGroup(groupHandleParent, groupHandleChild);
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

    void RendererCachedScene::updateRenderablesAndResourceCache(const IResourceDeviceHandleAccessor& resourceAccessor)
    {
        updateRenderableResources(resourceAccessor);
        updatePassRenderableSorting();
    }

    void RendererCachedScene::updatePassRenderableSorting()
    {
        if (m_renderableOrderingDirty)
        {
            m_sortedRenderingPasses.clear();

            const uint32_t totalNumberOfRenderPasses = BaseT::getRenderPassCount();
            const uint32_t totalNumberOfBlitPasses = BaseT::getBlitPassCount();

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
                if (BaseT::isBlitPassAllocated(passHandle))
                {
                    const BlitPass& blitPass = BaseT::getBlitPass(passHandle);
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

    const glm::mat4& RendererCachedScene::getRenderableWorldMatrix(RenderableHandle renderable) const
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
        if (scene.getRenderable(renderable).visibilityMode == EVisibilityMode::Visible)
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

        auto renderablesIterator = orderedGroupRenderables.begin();
        auto renderGroupIterator = orderedRenderGroups.begin();
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
        m_renderableMatrices.resize(BaseT::getRenderableCount());
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
        m_renderableMatrices.resize(BaseT::getRenderableCount());
        for (const auto& renderables : m_passRenderableOrder)
        {
            for (const auto renderable : renderables)
            {
                assert(renderable.isValid());
                const NodeHandle node = BaseT::getRenderable(renderable).node;
                assert(node.isValid());
                m_renderableMatrices[renderable.asMemoryHandle()] = updateMatrixCacheWithLinks(ETransformationMatrixType_World, node);
            }
        }
    }

    bool RendererCachedScene::shouldRenderPassBeRendered(RenderPassHandle handle) const
    {
        if (!BaseT::isRenderPassAllocated(handle))
        {
            return false;
        }

        const RenderPass& rp = BaseT::getRenderPass(handle);
        if (!rp.camera.isValid() || !rp.isEnabled)
        {
            return false;
        }

        return !rp.isRenderOnce || m_renderOncePassesToRender.contains(handle);
    }

    void RendererCachedScene::retriggerAllRenderOncePasses()
    {
        const uint32_t numRPs = BaseT::getRenderPassCount();
        for (RenderPassHandle handle(0u); handle < numRPs; ++handle)
        {
            if (BaseT::isRenderPassAllocated(handle))
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
        if (m_renderOncePassesToRender.size() > 0u)
        {
            // some render once passes were rendered, remove them from list
            // and force update of cached render pass list for next update
            m_renderOncePassesToRender.clear();
            m_renderableOrderingDirty = true;
        }
    }
}
