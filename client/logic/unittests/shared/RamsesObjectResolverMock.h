//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"

#include "internals/RamsesObjectResolver.h"

namespace rlogic::internal
{
    class RamsesObjectResolverMock : public IRamsesObjectResolver
    {
    public:
        MOCK_METHOD(ramses::Node*, findRamsesNodeInScene, (std::string_view logicNodeName, ramses::sceneObjectId_t objectId), (const, override));
        MOCK_METHOD(ramses::Appearance*, findRamsesAppearanceInScene, (std::string_view logicNodeName, ramses::sceneObjectId_t objectId), (const, override));
        MOCK_METHOD(ramses::Camera*, findRamsesCameraInScene, (std::string_view logicNodeName, ramses::sceneObjectId_t objectId), (const, override));
        MOCK_METHOD(ramses::RenderPass*, findRamsesRenderPassInScene, (std::string_view logicNodeName, ramses::sceneObjectId_t objectId), (const, override));
        MOCK_METHOD(ramses::RenderGroup*, findRamsesRenderGroupInScene, (std::string_view logicNodeName, ramses::sceneObjectId_t objectId), (const, override));
        MOCK_METHOD(ramses::SceneObject*, findRamsesSceneObjectInScene, (std::string_view logicNodeName, ramses::sceneObjectId_t objectId), (const, override));
    };
}
