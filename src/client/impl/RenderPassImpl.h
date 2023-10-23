//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneObjectImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "impl/DataTypesImpl.h"

#include <vector>
#include <string_view>

namespace ramses
{
    class Camera;
    class RenderPass;
    class RenderTarget;
}

namespace ramses::internal
{
    class IScene;
    class CameraImpl;
    class CameraNodeImpl;
    class RenderGroupImpl;
    class RenderTargetImpl;

    using RenderGroupVector = std::vector<const RenderGroupImpl *>;

    class RenderPassImpl final : public SceneObjectImpl
    {
    public:
        RenderPassImpl(SceneImpl& scene, std::string_view renderpassName);
        ~RenderPassImpl() override;

        void initializeFrameworkData();
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        bool setCamera(const CameraNodeImpl& cameraImpl);
        [[nodiscard]] const ramses::Camera* getCamera()const;
        [[nodiscard]] ramses::Camera* getCamera();

        bool setClearColor(const glm::vec4& clearColor);
        [[nodiscard]] const glm::vec4& getClearColor() const;

        bool setClearFlags(ClearFlags clearFlags);
        [[nodiscard]] ClearFlags getClearFlags() const;

        bool                 addRenderGroup(const RenderGroupImpl& renderGroup, int32_t orderWithinPass);
        bool                 remove(const RenderGroupImpl& renderGroup);
        void                 removeIfContained(const RenderGroupImpl& renderGroup);
        [[nodiscard]] bool   contains(const RenderGroupImpl& renderGroup) const;
        bool                 getRenderGroupOrder(const RenderGroupImpl& renderGroup, int32_t& orderWithinPass) const;
        [[nodiscard]] const RenderGroupVector& getAllRenderGroups() const;
        bool                 removeAllRenderGroups();

        bool setRenderTarget(RenderTargetImpl* renderTarget);
        [[nodiscard]] const ramses::RenderTarget* getRenderTarget() const;

        bool setRenderOrder(int32_t renderOrder);
        [[nodiscard]] int32_t getRenderOrder() const;

        bool setEnabled(bool isEnabeld);
        [[nodiscard]] bool isEnabled() const;

        bool setRenderOnce(bool enable);
        [[nodiscard]] bool isRenderOnce() const;
        bool retriggerRenderOnce();

        [[nodiscard]] RenderPassHandle getRenderPassHandle() const;

    private:
        void removeInternal(RenderGroupVector::iterator iter);

        RenderPassHandle        m_renderPassHandle;
        const CameraNodeImpl*   m_cameraImpl;
        const RenderTargetImpl* m_renderTargetImpl;

        RenderGroupVector m_renderGroups;
    };
}
