//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RendererSceneState.h"

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"

#include <string_view>

namespace ramses::internal
{
    class SceneReferenceImpl final : public SceneObjectImpl
    {
    public:
        SceneReferenceImpl(SceneImpl& scene, std::string_view name);

        void initializeFrameworkData(sceneId_t referencedScene);

        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        bool requestState(RendererSceneState requestedState);
        [[nodiscard]] sceneId_t getReferencedSceneId() const;
        [[nodiscard]] RendererSceneState getRequestedState() const;
        bool requestNotificationsForSceneVersionTags(bool flag);
        bool setRenderOrder(int32_t renderOrder);

        [[nodiscard]] ramses::internal::SceneReferenceHandle getSceneReferenceHandle() const;

        [[nodiscard]] RendererSceneState getReportedState() const;
        void setReportedState(RendererSceneState state);

    private:
        RendererSceneState m_reportedState = RendererSceneState::Unavailable;

        ramses::internal::SceneReferenceHandle m_sceneReferenceHandle;
    };
}
