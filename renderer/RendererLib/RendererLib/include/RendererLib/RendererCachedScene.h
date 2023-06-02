//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCACHEDSCENE_H
#define RAMSES_RENDERERCACHEDSCENE_H

#include "RendererLib/ResourceCachedScene.h"
#include "RenderingPassInfo.h"

namespace ramses_internal
{
    class IResourceDeviceHandleAccessor;

    class RendererCachedScene final : public ResourceCachedScene
    {
    public:
        explicit RendererCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        void updateRenderablesAndResourceCache(const IResourceDeviceHandleAccessor& resourceAccessor);
        void updateRenderableWorldMatrices();
        void updateRenderableWorldMatricesWithLinks();

        void retriggerAllRenderOncePasses();
        void markAllRenderOncePassesAsRendered() const;

        /**
         * The renderer sets this to true when it applies a semantic time uniform
         * that is supposed to enable a shader based animation
         *
         * Workflow:
         * - The flag will be reset to 0 when a new flush is applied
         * - The flush will trigger rendering for the scene
         * - If a semantic uniform is accessed during rendering the flag will be set to true
         * - The scene will be re-rendered as long as the flag is true (i.e. until a new flush arrives)
         */
        void setActiveShaderAnimation(bool hasAnimation);

        /**
         * Indicates if there is an active shader animation (semantic time uniform is used)
         * The renderer should disable skub when this flag is set, otherwise the scene will not be
         * re-rendered
         */
        bool hasActiveShaderAnimation() const;

        void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visible) override;

        void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;

        void                        releaseRenderPass               (RenderPassHandle passHandle) override;
        void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) override;
        void                        setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) override;
        void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) override;
        void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) override;
        void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;
        void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;

        BlitPassHandle              allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        void                        releaseBlitPass(BlitPassHandle passHandle) override;
        void                        setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder) override;
        void                        setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled) override;

        const RenderingPassInfoVector&      getSortedRenderingPasses        () const;
        const RenderableVector&             getOrderedRenderablesForPass    (RenderPassHandle pass) const;
        const glm::mat4&                    getRenderableWorldMatrix        (RenderableHandle renderable) const;

    private:
        void updatePassRenderableSorting();
        void updateRenderablesInPass(RenderPassHandle passHandle);
        void addRenderablesFromRenderGroup(RenderableVector& orderedRenderables, RenderGroupHandle renderGroupHandle);
        bool shouldRenderPassBeRendered(RenderPassHandle handle) const;

        RenderingPassInfoVector m_sortedRenderingPasses;
        using PassRenderableOrder = std::vector<RenderableVector>;
        PassRenderableOrder     m_passRenderableOrder;
        mutable bool            m_renderableOrderingDirty;

        using MatrixVector = std::vector<glm::mat4>;
        MatrixVector            m_renderableMatrices;

        using RenderPasses = HashSet<RenderPassHandle>;
        mutable RenderPasses m_renderOncePassesToRender;

        bool m_hasActiveShaderAnimation = false;
    };

    inline void RendererCachedScene::setActiveShaderAnimation(bool hasAnimation)
    {
        m_hasActiveShaderAnimation = hasAnimation;
    }

    inline bool RendererCachedScene::hasActiveShaderAnimation() const
    {
        return m_hasActiveShaderAnimation;
    }
}

#endif
