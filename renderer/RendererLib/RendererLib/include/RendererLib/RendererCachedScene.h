//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCACHEDSCENE_H
#define RAMSES_RENDERERCACHEDSCENE_H

#include "RendererLib/TextureLinkCachedScene.h"
#include "RenderingPassInfo.h"

namespace ramses_internal
{
    class IResourceDeviceHandleAccessor;

    class RendererCachedScene final : public TextureLinkCachedScene
    {
    public:
        RendererCachedScene(SceneLinksManager& sceneLinksManager, const SceneInfo& sceneInfo = SceneInfo());

        void updateRenderablesAndResourceCache(const IResourceDeviceHandleAccessor& resourceAccessor, const IEmbeddedCompositingManager& embeddedCompositingManager);
        void updateRenderableWorldMatrices();
        void updateRenderableWorldMatricesWithLinks();

        void retriggerAllRenderOncePasses();
        void markAllRenderOncePassesAsRendered() const;

        virtual void                        setRenderableVisibility         (RenderableHandle renderableHandle, Bool visible) override;

        virtual void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        virtual void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        virtual void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;

        virtual void                        releaseRenderPass               (RenderPassHandle passHandle) override;
        virtual void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) override;
        virtual void                        setRenderPassEnabled            (RenderPassHandle passHandle, Bool isEnabled) override;
        virtual void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, Bool enable) override;
        virtual void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        virtual void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) override;
        virtual void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;
        virtual void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        virtual void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;

        virtual BlitPassHandle              allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        virtual void                        releaseBlitPass(BlitPassHandle passHandle) override;
        virtual void                        setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder) override;
        virtual void                        setBlitPassEnabled(BlitPassHandle passHandle, Bool isEnabled) override;

        const RenderingPassInfoVector&      getSortedRenderingPasses        () const;
        const RenderableVector&             getOrderedRenderablesForPass    (RenderPassHandle pass) const;
        const Matrix44f&                    getRenderableWorldMatrix        (RenderableHandle renderable) const;

    private:
        void updatePassRenderableSorting();
        void updateRenderablesInPass(RenderPassHandle passHandle);
        void addRenderablesFromRenderGroup(RenderableVector& orderedRenderables, RenderGroupHandle renderGroupHandle);
        Bool shouldRenderPassBeRendered(RenderPassHandle handle) const;

        RenderingPassInfoVector m_sortedRenderingPasses;
        typedef std::vector<RenderableVector> PassRenderableOrder;
        PassRenderableOrder     m_passRenderableOrder;
        mutable Bool            m_renderableOrderingDirty;

        typedef std::vector<Matrix44f> MatrixVector;
        MatrixVector            m_renderableMatrices;

        using RenderPasses = HashSet<RenderPassHandle>;
        mutable RenderPasses m_renderOncePassesToRender;
    };
}

#endif
