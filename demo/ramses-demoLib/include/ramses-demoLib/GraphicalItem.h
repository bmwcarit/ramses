//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEMOLIB_GRAPHICALITEM_H
#define RAMSES_DEMOLIB_GRAPHICALITEM_H

#include "ramses-client.h"
#include "Collections/Vector.h"

class GraphicalItem
{
public:
    GraphicalItem(ramses::Scene& scene, ramses::RamsesClient& client, ramses::RenderGroup* renderGroup = nullptr);
    virtual ~GraphicalItem();

    void                  initOutputBuffer(uint32_t width, uint32_t height, int32_t renderOrder);
    ramses::RenderBuffer* getOutputBuffer();

protected:
    ramses::Scene&              m_scene;
    ramses::RamsesClient&       m_client;
    ramses::RenderGroup*        m_renderGroup  = nullptr;
    ramses::RenderBuffer*       m_renderBuffer = nullptr;
    ramses::RenderTarget*       m_renderTarget = nullptr;
    ramses::OrthographicCamera* m_camera       = nullptr;
    ramses::RenderPass*         m_renderPass   = nullptr;

    ramses_internal::Vector<ramses::SceneObject*>    m_sceneObjects;
    ramses_internal::Vector<const ramses::Resource*> m_clientResources;
};

#endif
