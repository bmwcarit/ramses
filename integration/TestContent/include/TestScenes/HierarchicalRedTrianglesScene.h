//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_HIERARCHICALREDTRIANGLESSCENE_H
#define RAMSES_HIERARCHICALREDTRIANGLESSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"

namespace ramses
{
    class Node;
    class GroupNode;
}

namespace ramses_internal
{
    class HierarchicalRedTrianglesScene : public IntegrationScene
    {
    public:
        HierarchicalRedTrianglesScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& position);

        enum
        {
            THREE_ROWS_TRIANGLES = 0,
            PARTIAL_VISIBILITY,
            NO_VISIBILITY,
            REENABLED_FULL_VISIBILITY,
            ROTATE_AND_SCALE,
            DELETE_MESHNODE
        };

    private:
        void destroySubTree(ramses::Node* rootNode);

        ramses::GroupNode* m_groupNode;
        ramses::GroupNode* m_subGroup1Node;
        ramses::GroupNode* m_subGroup2Node;
        ramses::GroupNode* m_subGroup3Node;
        ramses::GroupNode& m_rotateNode1;
        ramses::GroupNode& m_rotateNode2;
        ramses::GroupNode& m_scaleNode1;
        ramses::GroupNode& m_scaleNode2;
    };
}

#endif
