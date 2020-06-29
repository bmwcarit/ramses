//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEREFERENCEIMPL_H
#define RAMSES_SCENEREFERENCEIMPL_H

#include "ramses-framework-api/RendererSceneState.h"

// internal
#include "SceneObjectImpl.h"

// ramses framework
#include "SceneAPI/Handles.h"
#include "SceneAPI/RendererSceneState.h"

namespace ramses
{
    class SceneReferenceImpl final : public SceneObjectImpl
    {
    public:
        SceneReferenceImpl(SceneImpl& scene, const char* name);

        void initializeFrameworkData(sceneId_t referencedScene);

        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        status_t requestState(RendererSceneState requestedState);
        sceneId_t getReferencedSceneId() const;
        RendererSceneState getRequestedState() const;
        status_t requestNotificationsForSceneVersionTags(bool flag);
        status_t setRenderOrder(int32_t renderOrder);

        ramses_internal::SceneReferenceHandle getSceneReferenceHandle() const;

        static ramses_internal::RendererSceneState GetInternalSceneReferenceState(ramses::RendererSceneState state);
        static RendererSceneState GetSceneReferenceState(ramses_internal::RendererSceneState state);

        RendererSceneState getReportedState() const;
        void setReportedState(RendererSceneState state);

    private:
        RendererSceneState m_reportedState = RendererSceneState::Unavailable;

        ramses_internal::SceneReferenceHandle m_sceneReferenceHandle;
    };
}

#endif
