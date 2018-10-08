//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERPASSGROUPTEST_H
#define RAMSES_RENDERPASSGROUPTEST_H

#include "PerformanceTestBase.h"
#include "Collections/Vector.h"

class RenderPassGroupTest : public PerformanceTestBase
{
public:

    enum
    {
        RenderPassGroupTest_FlatGroups_Creation = 0,
        RenderPassGroupTest_FlatGroups_Destruction,
        RenderPassGroupTest_NestedGroups_Creation,
        RenderPassGroupTest_NestedGroups_Destruction,
    };

    RenderPassGroupTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void preUpdate() override;
    virtual void update() override;

private:

    static const uint32_t MeshCount = 10 * 1000;
    static const uint32_t RenderGroupCount = 512;
    static const uint32_t RenderPassCount = 32;

    void createThings();
    void destroyThings();

    void addRenderGroups();
    void addRenderPasses();
    void shuffleGroupsAndPasses();

    void assignGroupsAndPasses();
    void assignNestedGroup(ramses::RenderGroup& parent, ramses_internal::Vector<ramses::RenderGroup*>& unassigned);

    ramses::Scene* m_scene;
    ramses_internal::Vector<ramses::MeshNode*> m_meshes;
    ramses_internal::Vector<ramses::RenderGroup*> m_groups;
    ramses_internal::Vector<ramses::RenderPass*> m_passes;

    bool m_useNestedRenderGroups;
};
#endif
