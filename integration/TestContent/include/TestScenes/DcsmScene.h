//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMSCENE_H
#define RAMSES_DCSMSCENE_H

#include "MultipleTrianglesScene.h"
#include "Triangle.h"
#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-client-api/MeshNode.h"

namespace ramses
{
    class DcsmProvider;
}

namespace ramses_internal
{
    class DcsmScene : public MultipleTrianglesScene, public ramses::IDcsmProviderEventHandler
    {
    public:
        DcsmScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, ramses::DcsmProvider& dcsmProvider,
            uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        virtual void contentHide(ramses::ContentID, ramses::AnimationInformation) override {}
        virtual void contentShow(ramses::ContentID, ramses::AnimationInformation) override {}
        virtual void stopOfferAccepted(ramses::ContentID, ramses::AnimationInformation) override {}
        virtual void contentSizeChange(ramses::ContentID, const ramses::CategoryInfoUpdate&, ramses::AnimationInformation) override {}
        virtual void contentReadyRequested(ramses::ContentID) override;
        virtual void contentRelease(ramses::ContentID, ramses::AnimationInformation) override {}
        virtual void dispatchHandler() override;

    private:
        ramses::DcsmProvider& m_dcsmProvider;
    };
}

#endif
