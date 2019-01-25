//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/OffscreenBufferLinks.h"

using namespace testing;
using namespace ramses_internal;

class ABufferLinks : public ::testing::Test
{
public:
    ABufferLinks()
        : providerBuffer1(1u)
        , providerBuffer2(2u)
        , consumerScene1(3u)
        , consumerScene2(4u)
        , consumerSlot1(3u)
        , consumerSlot2(4u)
    {
    }

protected:
    static bool LinksEqual(const OffscreenBufferLink& link1, const OffscreenBufferLink& link2)
    {
        return link1.providerBuffer == link2.providerBuffer
            && link1.consumerSceneId == link2.consumerSceneId
            && link1.consumerSlot == link2.consumerSlot;
    }

    static bool ContainsLink(const OffscreenBufferLinkVector& links, const OffscreenBufferLink& link)
    {
        for(const auto& linkIter : links)
        {
            if (LinksEqual(linkIter, link))
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
        EXPECT_FALSE(sceneLinks.hasAnyLinksToConsumer(providerBuffer1));
        EXPECT_FALSE(sceneLinks.hasAnyLinksToConsumer(providerBuffer2));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene1, consumerSlot1));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene1, consumerSlot2));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene2, consumerSlot1));
        EXPECT_FALSE(sceneLinks.hasLinkedProvider(consumerScene2, consumerSlot2));
        expectLinkCount(providerBuffer1, 0u);
        expectLinkCount(providerBuffer2, 0u);
        expectLinkCount(consumerScene1, 0u);
        expectLinkCount(consumerScene2, 0u);
    }

    void expectLinkCount(SceneId sceneId, UInt32 providerScenesLinked)
    {
        OffscreenBufferLinkVector links;
        sceneLinks.getLinkedProviders(sceneId, links);
        EXPECT_EQ(providerScenesLinked, links.size());
    }

    void expectLinkCount(OffscreenBufferHandle providerBuffer, UInt32 consumerScenesLinked)
    {
        OffscreenBufferLinkVector links;
        sceneLinks.getLinkedConsumers(providerBuffer, links);
        EXPECT_EQ(consumerScenesLinked, links.size());
    }

    void expectLink(
        OffscreenBufferHandle providerBuffer,
        SceneId consumerSceneId,
        DataSlotHandle consumerSlotHandle)
    {
        EXPECT_TRUE(sceneLinks.hasAnyLinksToProvider(consumerSceneId));
        EXPECT_TRUE(sceneLinks.hasAnyLinksToConsumer(providerBuffer));
        EXPECT_TRUE(sceneLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle));

        OffscreenBufferLink expectedLink;
        expectedLink.providerBuffer = providerBuffer;
        expectedLink.consumerSceneId = consumerSceneId;
        expectedLink.consumerSlot = consumerSlotHandle;

        OffscreenBufferLinkVector links;
        sceneLinks.getLinkedProviders(consumerSceneId, links);
        EXPECT_TRUE(ContainsLink(links, expectedLink));
        links.clear();

        EXPECT_TRUE(LinksEqual(expectedLink, sceneLinks.getLinkedProvider(consumerSceneId, consumerSlotHandle)));

        sceneLinks.getLinkedConsumers(providerBuffer, links);
        EXPECT_TRUE(ContainsLink(links, expectedLink));
    }

    OffscreenBufferLinks sceneLinks;

    const OffscreenBufferHandle providerBuffer1;
    const OffscreenBufferHandle providerBuffer2;
    const SceneId consumerScene1;
    const SceneId consumerScene2;

    const DataSlotHandle consumerSlot1;
    const DataSlotHandle consumerSlot2;
};

TEST_F(ABufferLinks, reportsNoLinksInitially)
{
    expectNoLinks();
}

TEST_F(ABufferLinks, canAddLink)
{
    sceneLinks.addLink(providerBuffer1, consumerScene1, consumerSlot1);
    expectLink(providerBuffer1, consumerScene1, consumerSlot1);
    expectLinkCount(providerBuffer1, 1u);
    expectLinkCount(consumerScene1, 1u);
}

TEST_F(ABufferLinks, canAddAndRemoveLink)
{
    sceneLinks.addLink(providerBuffer1, consumerScene1, consumerSlot1);
    sceneLinks.removeLink(consumerScene1, consumerSlot1);
    expectNoLinks();
}

TEST_F(ABufferLinks, canAddAndRemoveMultipleLinks)
{
    sceneLinks.addLink(providerBuffer1, consumerScene1, consumerSlot1);
    sceneLinks.addLink(providerBuffer1, consumerScene1, consumerSlot2);
    sceneLinks.addLink(providerBuffer1, consumerScene2, consumerSlot1);
    sceneLinks.addLink(providerBuffer2, consumerScene2, consumerSlot2);

    expectLink(providerBuffer1, consumerScene1, consumerSlot1);
    expectLink(providerBuffer1, consumerScene1, consumerSlot2);
    expectLink(providerBuffer1, consumerScene2, consumerSlot1);
    expectLink(providerBuffer2, consumerScene2, consumerSlot2);

    expectLinkCount(providerBuffer1, 3u);
    expectLinkCount(consumerScene1, 2u);
    expectLinkCount(consumerScene2, 2u);

    sceneLinks.removeLink(consumerScene1, consumerSlot1);
    sceneLinks.removeLink(consumerScene1, consumerSlot2);
    sceneLinks.removeLink(consumerScene2, consumerSlot1);
    sceneLinks.removeLink(consumerScene2, consumerSlot2);
    expectNoLinks();
}

TEST_F(ABufferLinks, removingLinkDoesNotAffectOtherLinks)
{
    sceneLinks.addLink(providerBuffer1, consumerScene1, consumerSlot1);
    sceneLinks.addLink(providerBuffer1, consumerScene1, consumerSlot2);
    sceneLinks.removeLink(consumerScene1, consumerSlot1);

    expectLink(providerBuffer1, consumerScene1, consumerSlot2);
    expectLinkCount(providerBuffer1, 1u);
    expectLinkCount(consumerScene1, 1u);

    sceneLinks.removeLink(consumerScene1, consumerSlot2);
    expectNoLinks();
}
