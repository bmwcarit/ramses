//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestInstance.h"
#include "RendererTestUtils.h"
#include "DisplayConfigImpl.h"
#include "RamsesRendererImpl.h"

RendererTestInstance::RendererTestInstance(const ramses::RamsesFrameworkConfig& config)
    : LocalTestRenderer()
    , m_ramsesFramework(config)
    , m_client("test renderer client", m_ramsesFramework)
    , m_scenes(m_client)
{
}

RendererTestInstance::~RendererTestInstance()
{
    destroyRenderer(); // must be destructed before m_ramsesFramework, because it has reference to it
}

void RendererTestInstance::initializeRenderer()
{
    initializeRendererWithFramework(m_ramsesFramework);
}

void RendererTestInstance::publish(ramses::sceneId_t sceneId)
{
    ramses::Scene& clientScene = m_scenes.getScene(sceneId);
    clientScene.publish(ramses::EScenePublicationMode_LocalOnly);

    waitForPublication(sceneId);
}

void RendererTestInstance::flush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag)
{
    ramses::Scene& clientScene = m_scenes.getScene(sceneId);
    clientScene.flush(ramses::ESceneFlushMode_SynchronizedWithResources, sceneVersionTag);
}

void RendererTestInstance::flushWithTimestamp(ramses::sceneId_t sceneId, uint64_t timeStamp, ramses::sceneVersionTag_t sceneVersionTag /*= ramses::InvalidSceneVersionTag*/)
{
    ramses::Scene& clientScene = m_scenes.getScene(sceneId);
    clientScene.flushWithTimestamp(timeStamp, sceneVersionTag);
}

void RendererTestInstance::unpublish(ramses::sceneId_t sceneId)
{
    ramses::Scene& clientScene = m_scenes.getScene(sceneId);
    clientScene.unpublish();
    waitForUnpublished(sceneId);
}

ramses::status_t RendererTestInstance::validateScene(ramses::sceneId_t sceneId)
{
    ramses::Scene& clientScene = m_scenes.getScene(sceneId);
    return clientScene.validate();
}

const char* RendererTestInstance::getValidationReport(ramses::sceneId_t sceneId)
{
    ramses::Scene& clientScene = m_scenes.getScene(sceneId);
    return clientScene.getValidationReport();
}

TestScenes& RendererTestInstance::getScenesRegistry()
{
    return m_scenes;
}

const ramses::RamsesClient& RendererTestInstance::getClient() const
{
    return m_client;
}

ramses::RamsesClient& RendererTestInstance::getClient()
{
    return m_client;
}
