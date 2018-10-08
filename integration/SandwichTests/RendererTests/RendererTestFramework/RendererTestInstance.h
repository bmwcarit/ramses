//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERTESTINSTANCE_H
#define RAMSES_RENDERERTESTINSTANCE_H

#include "LocalTestRenderer.h"
#include "TestScenes.h"

class RendererTestInstance : public LocalTestRenderer
{
public:
    explicit RendererTestInstance(const ramses::RamsesFrameworkConfig& config);
    virtual ~RendererTestInstance() override;

    void initializeRenderer();

    void             publish(ramses::sceneId_t sceneId);
    void             flush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag = ramses::InvalidSceneVersionTag);
    void             flushWithTimestamp(ramses::sceneId_t sceneId, uint64_t timeStamp, ramses::sceneVersionTag_t sceneVersionTag = ramses::InvalidSceneVersionTag);
    void             unpublish(ramses::sceneId_t sceneId);

    ramses::status_t validateScene(ramses::sceneId_t sceneId);
    const char*      getValidationReport(ramses::sceneId_t sceneId);

    const TestScenes& getScenesRegistry() const;
    TestScenes& getScenesRegistry();

    const ramses::RamsesClient& getClient() const;
    ramses::RamsesClient& getClient();

private:
    ramses::RamsesFramework        m_ramsesFramework;
    ramses::RamsesClient           m_client;

    TestScenes m_scenes;
};

#endif
