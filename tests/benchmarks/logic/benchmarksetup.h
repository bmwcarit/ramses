//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "benchmark/benchmark.h"
#include "ramses/client/ramses-client.h"
#include "ramses/client/logic/LogicEngine.h"

namespace ramses
{
    class BenchmarkSetUp
    {
    public:
        RamsesFramework m_framework{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
        RamsesClient& m_client{ *m_framework.createClient("benchmarkClient") };
        Scene& m_scene{ *m_client.createScene(SceneConfig(sceneId_t{ 123u })) };
        LogicEngine& m_logicEngine{ *m_scene.createLogicEngine() };
    };
}
