//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERPASSIMPL_H
#define RAMSES_RENDERPASSIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    class IScene;
    class Vector4;
}

namespace ramses
{
    class Camera;
    class CameraImpl;
    class CameraNodeImpl;
    class RenderGroupImpl;
    class RenderPass;
    class RenderTarget;
    class RenderTargetImpl;

    typedef std::vector<const RenderGroupImpl*>   RenderGroupVector;

    class RenderPassImpl final : public SceneObjectImpl
    {
    public:
        RenderPassImpl(SceneImpl& scene, const char* renderpassName);
        virtual ~RenderPassImpl();

        void             initializeFrameworkData();
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent) const override;

        status_t      setCamera(const CameraNodeImpl& cameraImpl);
        const Camera* getCamera()const;

        status_t setClearColor(const ramses_internal::Vector4& clearColor);
        const ramses_internal::Vector4& getClearColor() const;

        status_t setClearFlags(uint32_t clearFlags);
        uint32_t getClearFlags() const;

        status_t                 addRenderGroup(const RenderGroupImpl& renderGroup, int32_t orderWithinPass);
        status_t                 remove(const RenderGroupImpl& renderGroup);
        void                     removeIfContained(const RenderGroupImpl& renderGroup);
        bool                     contains(const RenderGroupImpl& renderGroup) const;
        status_t                 getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinPass) const;
        const RenderGroupVector& getAllRenderGroups() const;
        status_t                 removeAllRenderGroups();

        status_t            setRenderTarget(RenderTargetImpl* renderTarget);
        const RenderTarget* getRenderTarget() const;

        status_t setRenderOrder(int32_t renderOrder);
        int32_t  getRenderOrder() const;

        status_t setEnabled(bool isEnabeld);
        bool     isEnabled() const;

        status_t setRenderOnce(bool enable);
        bool     isRenderOnce() const;
        status_t retriggerRenderOnce();

        ramses_internal::RenderPassHandle getRenderPassHandle() const;

    private:
        void removeInternal(RenderGroupVector::iterator iter);

        ramses_internal::RenderPassHandle   m_renderPassHandle;
        const CameraNodeImpl*               m_cameraImpl;
        const RenderTargetImpl*             m_renderTargetImpl;

        RenderGroupVector m_renderGroups;
    };
}

#endif
