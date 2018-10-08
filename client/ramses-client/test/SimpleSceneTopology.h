//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SIMPLESCENETOPOLOGY_H
#define RAMSES_SIMPLESCENETOPOLOGY_H

#include "gtest/gtest.h"
#include "ClientTestUtils.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/GroupNode.h"
#include "ramses-client-api/VisibilityNode.h"

#include "MeshNodeImpl.h"

namespace ramses
{
    class SimpleSceneTopology : public LocalTestClientWithScene, public ::testing::Test
    {
    protected:
        /*
        Test fixture provides this scene graph :

                           m_root
                        /          \
                    m_vis1          m_vis2
                    /      \          /    \
             m_mesh1a  m_mesh1b  m_mesh2a  m_mesh2b
        */

        SimpleSceneTopology()
            : m_root(*m_scene.createGroupNode("root"))
            , m_vis1(*m_scene.createVisibilityNode("vis1"))
            , m_vis2(*m_scene.createVisibilityNode("vis2"))
            , m_mesh1a(*m_scene.createMeshNode("mesh1a"))
            , m_mesh1b(*m_scene.createMeshNode("mesh1b"))
            , m_mesh2a(*m_scene.createMeshNode("mesh2a"))
            , m_mesh2b(*m_scene.createMeshNode("mesh2b"))
        {
            EXPECT_EQ(StatusOK, m_root.addChild(m_vis1));
            EXPECT_EQ(StatusOK, m_root.addChild(m_vis2));
            EXPECT_EQ(StatusOK, m_vis1.addChild(m_mesh1a));
            EXPECT_EQ(StatusOK, m_vis1.addChild(m_mesh1b));
            EXPECT_EQ(StatusOK, m_vis2.addChild(m_mesh2a));
            EXPECT_EQ(StatusOK, m_vis2.addChild(m_mesh2b));

            //Every Mesh should be visible by default
            EXPECT_TRUE(m_mesh1a.impl.getFlattenedVisibility());
            EXPECT_TRUE(m_mesh1b.impl.getFlattenedVisibility());
            EXPECT_TRUE(m_mesh2a.impl.getFlattenedVisibility());
            EXPECT_TRUE(m_mesh2b.impl.getFlattenedVisibility());

        }

        GroupNode& m_root;
        VisibilityNode& m_vis1;
        VisibilityNode& m_vis2;
        MeshNode& m_mesh1a;
        MeshNode& m_mesh1b;
        MeshNode& m_mesh2a;
        MeshNode& m_mesh2b;
    };
}

#endif
