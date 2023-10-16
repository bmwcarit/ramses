//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/SceneLinks.h"
#include "internal/Core/Utils/ThreadLocalLog.h"

using namespace testing;

namespace ramses::internal
{
class ASceneLinks : public ::testing::Test
{
public:
    ASceneLinks()
        : providerScene1(1u)
        , providerScene2(2u)
        , consumerScene1(3u)
        , consumerScene2(4u)
        , providerSlot1(1u)
        , providerSlot2(2u)
        , consumerSlot1(3u)
        , consumerSlot2(4u)
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);
    }

protected:
    static bool LinksEqual(const SceneLink& link1, const SceneLink& link2)
    {
        return link1.providerSceneId == link2.providerSceneId
            && link1.providerSlot == link2.providerSlot
            && link1.consumerSceneId == link2.consumerSceneId
            && link1.consumerSlot == link2.consumerSlot;
    }

    static bool ContainsLink(const SceneLinkVector& links, const SceneLink& link)
    {
        for(auto linkIt : links)
        {
            if (LinksEqual(linkIt, link))
            {
                return true;
            }
        }

        return false;
    }

    void expectNoLinks()
    {
        EXPECT_FALSE(sceneLinks.hasAnyLinksToProvider(consumerScene1));
        EXPECT_FALSE(sceneLinks.hasAnyLinksToProvider(consumerScene2));
        EXPECT_FALSE(sceneLinks.hasAnyLinksToConsumer(providerScene1));
        EXPECT_FALSE(sceneLinks.hasAnyLinksToConsumer(providerScene2));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene1, consumerSlot1));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene1, consumerSlot2));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene2, consumerSlot1));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene2, consumerSlot2));
        EXPECT_FALSE(sceneLinks.hasLinkedConsumers(providerScene1, providerSlot1));
        EXPECT_FALSE(sceneLinks.hasLinkedConsumers(providerScene1, providerSlot2));
        EXPECT_FALSE(sceneLinks.hasLinkedConsumers(providerScene2, providerSlot1));
        EXPECT_FALSE(sceneLinks.hasLinkedConsumers(providerScene2, providerSlot2));
        expectLinkCount(providerScene1, 0u, 0u);
        expectLinkCount(providerScene2, 0u, 0u);
        expectLinkCount(consumerScene1, 0u, 0u);
        expectLinkCount(consumerScene2, 0u, 0u);
    }

    void expectLinkCount(SceneId sceneId, uint32_t providerScenesLinked, uint32_t consumerScenesLinked)
    {
        SceneLinkVector links;
        sceneLinks.getLinkedProviders(sceneId, links);
        EXPECT_EQ(providerScenesLinked, links.size());

        links.clear();
        sceneLinks.getLinkedConsumers(sceneId, links);
        EXPECT_EQ(consumerScenesLinked, links.size());
    }

    void expectLink(
        SceneId providerSceneId,
        DataSlotHandle providerSlotHandle,
        SceneId consumerSceneId,
        DataSlotHandle consumerSlotHandle)
    {
        EXPECT_TRUE(sceneLinks.hasAnyLinksToProvider(consumerSceneId));
        EXPECT_TRUE(sceneLinks.hasAnyLinksToConsumer(providerSceneId));
        EXPECT_TRUE(sceneLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(sceneLinks.hasLinkedConsumers(providerSceneId, providerSlotHandle));

        SceneLink expectedLink;
        expectedLink.providerSceneId = providerSceneId;
        expectedLink.providerSlot = providerSlotHandle;
        expectedLink.consumerSceneId = consumerSceneId;
        expectedLink.consumerSlot = consumerSlotHandle;

        SceneLinkVector links;
        sceneLinks.getLinkedProviders(consumerSceneId, links);
        EXPECT_TRUE(ContainsLink(links, expectedLink));
        links.clear();

        EXPECT_TRUE(LinksEqual(expectedLink, sceneLinks.getLinkedProvider(consumerSceneId, consumerSlotHandle)));

        sceneLinks.getLinkedConsumers(providerSceneId, links);
        EXPECT_TRUE(ContainsLink(links, expectedLink));
        links.clear();

        sceneLinks.getLinkedConsumers(providerSceneId, providerSlotHandle, links);
        EXPECT_TRUE(ContainsLink(links, expectedLink));
        links.clear();
    }

    SceneLinks sceneLinks;

    const SceneId providerScene1;
    const SceneId providerScene2;
    const SceneId consumerScene1;
    const SceneId consumerScene2;

    const DataSlotHandle providerSlot1;
    const DataSlotHandle providerSlot2;
    const DataSlotHandle consumerSlot1;
    const DataSlotHandle consumerSlot2;
};

TEST_F(ASceneLinks, reportsNoLinksForUnlinkedScenes)
{
    expectNoLinks();
}

TEST_F(ASceneLinks, canAddLink)
{
    sceneLinks.addLink(providerScene1, providerSlot1, consumerScene1, consumerSlot1);
    expectLink(providerScene1, providerSlot1, consumerScene1, consumerSlot1);
    expectLinkCount(providerScene1, 0u, 1u);
    expectLinkCount(consumerScene1, 1u, 0u);
}

TEST_F(ASceneLinks, canAddAndRemoveLink)
{
    sceneLinks.addLink(providerScene1, providerSlot1, consumerScene1, consumerSlot1);
    sceneLinks.removeLink(consumerScene1, consumerSlot1);
    expectNoLinks();
}

TEST_F(ASceneLinks, canAddAndRemoveMultipleLinks)
{
    sceneLinks.addLink(providerScene1, providerSlot1, consumerScene1, consumerSlot1);
    sceneLinks.addLink(providerScene1, providerSlot1, consumerScene1, consumerSlot2);
    sceneLinks.addLink(providerScene1, providerSlot1, consumerScene2, consumerSlot1);
    sceneLinks.addLink(providerScene2, providerSlot2, consumerScene2, consumerSlot2);

    expectLink(providerScene1, providerSlot1, consumerScene1, consumerSlot1);
    expectLink(providerScene1, providerSlot1, consumerScene1, consumerSlot2);
    expectLink(providerScene1, providerSlot1, consumerScene2, consumerSlot1);
    expectLink(providerScene2, providerSlot2, consumerScene2, consumerSlot2);

    expectLinkCount(providerScene1, 0u, 3u);
    expectLinkCount(consumerScene1, 2u, 0u);
    expectLinkCount(consumerScene2, 2u, 0u);

    sceneLinks.removeLink(consumerScene1, consumerSlot1);
    sceneLinks.removeLink(consumerScene1, consumerSlot2);
    sceneLinks.removeLink(consumerScene2, consumerSlot1);
    sceneLinks.removeLink(consumerScene2, consumerSlot2);
    expectNoLinks();
}

TEST_F(ASceneLinks, removingLinkDoesNotAffectOtherLinks)
{
    sceneLinks.addLink(providerScene1, providerSlot1, consumerScene1, consumerSlot1);
    sceneLinks.addLink(providerScene1, providerSlot1, consumerScene1, consumerSlot2);
    sceneLinks.removeLink(consumerScene1, consumerSlot1);

    expectLink(providerScene1, providerSlot1, consumerScene1, consumerSlot2);
    expectLinkCount(providerScene1, 0u, 1u);
    expectLinkCount(consumerScene1, 1u, 0u);

    sceneLinks.removeLink(consumerScene1, consumerSlot2);
    expectNoLinks();
}
}
