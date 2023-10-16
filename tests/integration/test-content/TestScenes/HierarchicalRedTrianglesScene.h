//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

namespace ramses
{
    class Node;
}

namespace ramses::internal
{
    class HierarchicalRedTrianglesScene : public IntegrationScene
    {
    public:
        HierarchicalRedTrianglesScene(ramses::Scene& scene, uint32_t state, const glm::vec3& position, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            THREE_ROWS_TRIANGLES = 0,
            PARTIAL_VISIBILITY,
            INVISIBLE,
            VISIBILITY_OFF,
            REENABLED_FULL_VISIBILITY,
            ROTATE_AND_SCALE,
            DELETE_MESHNODE
        };

    private:
        void destroySubTree(ramses::Node* rootNode);

        ramses::Node* m_groupNode;
        ramses::Node* m_subGroup1Node;
        ramses::Node* m_subGroup2Node;
        ramses::Node* m_subGroup3Node;
        ramses::Node& m_rotateNode1;
        ramses::Node& m_rotateNode2;
        ramses::Node& m_scaleNode1;
        ramses::Node& m_scaleNode2;
    };
}
