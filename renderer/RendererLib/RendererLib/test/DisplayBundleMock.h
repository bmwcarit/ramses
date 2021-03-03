//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYBUNDLEMOCK_H
#define RAMSES_DISPLAYBUNDLEMOCK_H

#include "gmock/gmock.h"
#include "RendererLib/DisplayBundle.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "RendererAPI/IEmbeddedCompositor.h"

namespace ramses_internal
{
    class DisplayBundleMock : public IDisplayBundle
    {
    public:
        DisplayBundleMock();
        virtual ~DisplayBundleMock() override;

        MOCK_METHOD(void, doOneLoop, (ELoopMode loopMode, std::chrono::microseconds sleepTime), (override));
        MOCK_METHOD(void, pushAndConsumeCommands, (RendererCommands& cmds), (override));
        MOCK_METHOD(void, dispatchRendererEvents, (RendererEventVector& events), (override));
        MOCK_METHOD(void, dispatchSceneControlEvents, (RendererEventVector& events), (override));
        MOCK_METHOD(SceneId, findMasterSceneForReferencedScene, (SceneId refScene), (const, override));
        MOCK_METHOD(void, enableContext, (), (override));
        MOCK_METHOD(IEmbeddedCompositingManager&, getECManager, (DisplayHandle display), (override));
        MOCK_METHOD(IEmbeddedCompositor&, getEC, (DisplayHandle display), (override));
    };
}
#endif
