//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/BufferLinks.h"

using namespace testing;
using namespace ramses_internal;

using BufferTypes = ::testing::Types <
    OffscreenBufferHandle,
    StreamBufferHandle > ;

template <typename BUFFERHANDLE>
class ABufferLinks : public ::testing::Test
{
protected:
    static bool LinksEqual(const BufferLink<BUFFERHANDLE>& link1, const BufferLink<BUFFERHANDLE>& link2)
    {
        return link1.providerBuffer == link2.providerBuffer
            && link1.consumerSceneId == link2.consumerSceneId
            && link1.consumerSlot == link2.consumerSlot;
    }

    static bool ContainsLink(const BufferLinkVector<BUFFERHANDLE>& links, const BufferLink<BUFFERHANDLE>& link)
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
        EXPECT_FALSE(this->sceneLinks.hasAnyLinksToProvider(this->consumerScene1));
        EXPECT_FALSE(this->sceneLinks.hasAnyLinksToProvider(this->consumerScene2));
        EXPECT_FALSE(this->sceneLinks.hasAnyLinksToConsumer(this->providerBuffer1));
        EXPECT_FALSE(this->sceneLinks.hasAnyLinksToConsumer(this->providerBuffer2));
        EXPECT_FALSE(this->sceneLinks.hasLinkedProvider(this->consumerScene1, this->consumerSlot1));
        EXPECT_FALSE(this->sceneLinks.hasLinkedProvider(this->consumerScene1, this->consumerSlot2));
        EXPECT_FALSE(this->sceneLinks.hasLinkedProvider(this->consumerScene2, this->consumerSlot1));
        EXPECT_FALSE(this->sceneLinks.hasLinkedProvider(this->consumerScene2, this->consumerSlot2));
        expectLinkCount(this->providerBuffer1, 0u);
        expectLinkCount(this->providerBuffer2, 0u);
        expectLinkCount(this->consumerScene1, 0u);
        expectLinkCount(this->consumerScene2, 0u);
    }

    void expectLinkCount(SceneId sceneId, uint32_t providerScenesLinked)
    {
        BufferLinkVector<BUFFERHANDLE> links;
        this->sceneLinks.getLinkedProviders(sceneId, links);
        EXPECT_EQ(providerScenesLinked, links.size());
    }

    void expectLinkCount(BUFFERHANDLE providerBuffer, uint32_t consumerScenesLinked)
    {
        BufferLinkVector<BUFFERHANDLE> links;
        this->sceneLinks.getLinkedConsumers(providerBuffer, links);
        EXPECT_EQ(consumerScenesLinked, links.size());
    }

    void expectLink(
        BUFFERHANDLE providerBuffer,
        SceneId consumerSceneId,
        DataSlotHandle consumerSlotHandle)
    {
        EXPECT_TRUE(this->sceneLinks.hasAnyLinksToProvider(consumerSceneId));
        EXPECT_TRUE(this->sceneLinks.hasAnyLinksToConsumer(providerBuffer));
        EXPECT_TRUE(this->sceneLinks.hasLinkedProvider(consumerSceneId, consumerSlotHandle));

        BufferLink<BUFFERHANDLE> expectedLink;
        expectedLink.providerBuffer = providerBuffer;
        expectedLink.consumerSceneId = consumerSceneId;
        expectedLink.consumerSlot = consumerSlotHandle;

        BufferLinkVector<BUFFERHANDLE> links;
        this->sceneLinks.getLinkedProviders(consumerSceneId, links);
        EXPECT_TRUE(ContainsLink(links, expectedLink));
        links.clear();

        EXPECT_TRUE(LinksEqual(expectedLink, this->sceneLinks.getLinkedProvider(consumerSceneId, consumerSlotHandle)));

        this->sceneLinks.getLinkedConsumers(providerBuffer, links);
        EXPECT_TRUE(ContainsLink(links, expectedLink));
    }

    BufferLinks<BUFFERHANDLE> sceneLinks;

    const BUFFERHANDLE providerBuffer1{ 1u };
    const BUFFERHANDLE providerBuffer2{ 2u };
    const SceneId consumerScene1{ 3u };
    const SceneId consumerScene2{ 4u };

    const DataSlotHandle consumerSlot1{ 5u };
    const DataSlotHandle consumerSlot2{ 6u };
};

TYPED_TEST_SUITE(ABufferLinks, BufferTypes);

TYPED_TEST(ABufferLinks, reportsNoLinksInitially)
{
    this->expectNoLinks();
}

TYPED_TEST(ABufferLinks, canAddLink)
{
    this->sceneLinks.addLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot1);
    this->expectLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot1);
    this->expectLinkCount(this->providerBuffer1, 1u);
    this->expectLinkCount(this->consumerScene1, 1u);
}

TYPED_TEST(ABufferLinks, canAddAndRemoveLink)
{
    this->sceneLinks.addLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot1);
    this->sceneLinks.removeLink(this->consumerScene1, this->consumerSlot1);
    this->expectNoLinks();
}

TYPED_TEST(ABufferLinks, canAddAndRemoveMultipleLinks)
{
    this->sceneLinks.addLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot1);
    this->sceneLinks.addLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot2);
    this->sceneLinks.addLink(this->providerBuffer1, this->consumerScene2, this->consumerSlot1);
    this->sceneLinks.addLink(this->providerBuffer2, this->consumerScene2, this->consumerSlot2);

    this->expectLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot1);
    this->expectLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot2);
    this->expectLink(this->providerBuffer1, this->consumerScene2, this->consumerSlot1);
    this->expectLink(this->providerBuffer2, this->consumerScene2, this->consumerSlot2);

    this->expectLinkCount(this->providerBuffer1, 3u);
    this->expectLinkCount(this->consumerScene1, 2u);
    this->expectLinkCount(this->consumerScene2, 2u);

    this->sceneLinks.removeLink(this->consumerScene1, this->consumerSlot1);
    this->sceneLinks.removeLink(this->consumerScene1, this->consumerSlot2);
    this->sceneLinks.removeLink(this->consumerScene2, this->consumerSlot1);
    this->sceneLinks.removeLink(this->consumerScene2, this->consumerSlot2);
    this->expectNoLinks();
}

TYPED_TEST(ABufferLinks, removingLinkDoesNotAffectOtherLinks)
{
    this->sceneLinks.addLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot1);
    this->sceneLinks.addLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot2);
    this->sceneLinks.removeLink(this->consumerScene1, this->consumerSlot1);

    this->expectLink(this->providerBuffer1, this->consumerScene1, this->consumerSlot2);
    this->expectLinkCount(this->providerBuffer1, 1u);
    this->expectLinkCount(this->consumerScene1, 1u);

    this->sceneLinks.removeLink(this->consumerScene1, this->consumerSlot2);
    this->expectNoLinks();
}
