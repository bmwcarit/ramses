//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/DcsmProvider.h"
#include "fmt/format.h"
#include <vector>


namespace ramses_internal
{
    class SceneViewerDcsmHandler : public ramses::IDcsmProviderEventHandler
    {
    public:
        struct Config
        {
            ramses::ContentID content;
            ramses::Category category;
            ramses::sceneId_t scene;
        };

        using ConfigList = std::vector<Config>;

        explicit SceneViewerDcsmHandler(ramses::DcsmProvider& dcsm, const ConfigList& config)
            : m_dcsm(dcsm)
        {
            for (auto& cfg : config)
            {
                m_dcsm.offerContent(cfg.content, cfg.category, cfg.scene, ramses::EDcsmOfferingMode::LocalAndRemote);
            }
        }

        void dispatchEvents()
        {
            m_dcsm.dispatchEvents(*this);
        }

        virtual void contentHide(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
        {
        }

        virtual void contentShow(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
        {
        }

        virtual void stopOfferAccepted(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
        {
        }

        virtual void contentSizeChange(ramses::ContentID /*contentID*/, const ramses::CategoryInfoUpdate& /*categoryInfo*/, ramses::AnimationInformation /*animInfo*/) override
        {
        }

        virtual void contentReadyRequested(ramses::ContentID contentID) override
        {
            // we have our content ready before we offer it, so we automatically reply with ready here
            m_dcsm.markContentReady(contentID);
        }

        virtual void contentRelease(ramses::ContentID /*contentID*/, ramses::AnimationInformation /*animInfo*/) override
        {
        }

    private:
        ramses::DcsmProvider& m_dcsm;
    };
}
