//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "renderer_common_gmock_header.h"
#include "RendererLib/PendingClientResourcesUtils.h"

using namespace testing;
using namespace ramses_internal;

class APendingClientResourcesUtils : public ::testing::Test
{
protected:
    static void ExpectListContains(const ResourceContentHashVector& list, const ResourceContentHashVector& resources)
    {
        for (const auto& res : resources)
        {
            EXPECT_TRUE(contains_c(list, res));
        }
    }

    static void ExpectListContainsExactly(const ResourceContentHashVector& list, const ResourceContentHashVector& resources)
    {
        EXPECT_EQ(list.size(), resources.size());
        ExpectListContains(list, resources);
    }

    const ResourceContentHash hash1 = { 1u, 2u };
    const ResourceContentHash hash2 = { 3u, 4u };
    const ResourceContentHash hash3 = { 5u, 6u };
    const ResourceContentHash hash4 = { 7u, 8u };

    ResourceContentHashVector needed;
    ResourceContentHashVector unneeded;
    ResourceContentHashVector added;
    ResourceContentHashVector removed;
    ResourceContentHashVector newlyNeeded;
    ResourceContentHashVector newlyUnneededPreviouslyNeeded;
    ResourceContentHashVector pendingUnneeded;
    ResourceContentHashVector inUse;
};

TEST_F(APendingClientResourcesUtils, marksNewResourcesAsNewlyAdded)
{
    added = { hash1, hash2 };
    PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
    ExpectListContainsExactly(needed, added);
    ExpectListContainsExactly(unneeded, {});
    ExpectListContainsExactly(newlyNeeded, added);
}

TEST_F(APendingClientResourcesUtils, ignoresNewResourcesIfPreviouslyUnneeded)
{
    unneeded = { hash1, hash2 };
    added = { hash1, hash2 };
    PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
    ExpectListContainsExactly(needed, {});
    ExpectListContainsExactly(unneeded, {});
    ExpectListContainsExactly(newlyNeeded, {});
}

TEST_F(APendingClientResourcesUtils, consolidatesNewResourcesWithPreviouslyNeededAndUnneeded)
{
    needed = { hash1 };
    unneeded = { hash2 };
    added = { hash2, hash3 };
    PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
    ExpectListContainsExactly(needed, { hash1, hash3 });
    ExpectListContainsExactly(unneeded, {});
    ExpectListContainsExactly(newlyNeeded, { hash3 });
}

TEST_F(APendingClientResourcesUtils, marksRemovedResourcesAsUnneeded)
{
    removed = { hash1, hash2 };
    PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
    ExpectListContainsExactly(needed, {});
    ExpectListContainsExactly(unneeded, removed);
    ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, {});
}

TEST_F(APendingClientResourcesUtils, marksRemovedResourcesPreviouslyNeededAsNewlyUnneeded)
{
    needed = { hash1, hash2 };
    removed = { hash1, hash2 };
    PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
    ExpectListContainsExactly(needed, {});
    ExpectListContainsExactly(unneeded, {});
    ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, removed);
}

TEST_F(APendingClientResourcesUtils, consolidatesRemovedResourcesWithPreviouslyNeededAndUnneeded)
{
    needed = { hash1 };
    unneeded = { hash2 };
    removed = { hash1, hash3 };
    PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
    ExpectListContainsExactly(needed, {});
    ExpectListContainsExactly(unneeded, { hash2, hash3 });
    ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, { hash1 });
}

TEST_F(APendingClientResourcesUtils, keepsNewlyNeededResourcesIfNotPendingUnneeded)
{
    newlyNeeded = { hash1, hash2 };
    PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
    ExpectListContainsExactly(newlyNeeded, { hash1, hash2 });
    ExpectListContainsExactly(pendingUnneeded, {});
}

TEST_F(APendingClientResourcesUtils, consolidatesNewlyNeededResourcesIfPendingUnneeded)
{
    newlyNeeded = { hash2, hash3 };
    pendingUnneeded = { hash1, hash3 };
    PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
    ExpectListContainsExactly(newlyNeeded, { hash2 });
    ExpectListContainsExactly(pendingUnneeded, { hash1 });
}

TEST_F(APendingClientResourcesUtils, addsNewlyUnneededResourcesAsPendingUnneeded)
{
    pendingUnneeded = { hash1 };
    newlyUnneededPreviouslyNeeded = { hash2, hash3 };
    PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
    ExpectListContainsExactly(pendingUnneeded, { hash1, hash2, hash3 });
}

TEST_F(APendingClientResourcesUtils, ignoresNewlyUnneededResourcesIfAlreadyPendingUnneeded)
{
    pendingUnneeded = { hash1, hash2 };
    newlyUnneededPreviouslyNeeded = { hash2, hash3 };
    PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
    ExpectListContainsExactly(pendingUnneeded, { hash1, hash2, hash3 });
}

TEST_F(APendingClientResourcesUtils, consolidatesNeededAndUnneededIntoInUse)
{
    inUse = { hash3, hash4 };
    needed = { hash1, hash2 };
    unneeded = { hash3 };
    PendingClientResourcesUtils::ConsolidateNeededAndUnneededResources(inUse, needed, unneeded);
    ExpectListContainsExactly(inUse, { hash1, hash2, hash4 });
}

TEST_F(APendingClientResourcesUtils, confidenceTest_multipleIterationsOfConsolidationsOfAddedAndRemovedResources)
{
    inUse = { hash4 };
    added = { hash1 };
    {
        PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
        PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
        PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
        PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
        ExpectListContainsExactly(needed, { hash1 });
        ExpectListContainsExactly(unneeded, {});
        ExpectListContainsExactly(newlyNeeded, { hash1 });
        ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, {});
        ExpectListContainsExactly(pendingUnneeded, {});
    }

    added = { hash2 };
    removed = { hash1 };
    {
        newlyNeeded.clear();
        newlyUnneededPreviouslyNeeded.clear();
        PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
        PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
        PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
        PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
        ExpectListContainsExactly(needed, { hash2 });
        ExpectListContainsExactly(unneeded, {});
        ExpectListContainsExactly(newlyNeeded, { hash2 });
        ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, { hash1 });
        ExpectListContainsExactly(pendingUnneeded, { hash1 });
    }

    added = { hash3 };
    removed = {};
    {
        newlyNeeded.clear();
        newlyUnneededPreviouslyNeeded.clear();
        PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
        PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
        PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
        PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
        ExpectListContainsExactly(needed, { hash2, hash3 });
        ExpectListContainsExactly(unneeded, {});
        ExpectListContainsExactly(newlyNeeded, { hash3 });
        ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, {});
        ExpectListContainsExactly(pendingUnneeded, { hash1 });
    }

    added = {};
    removed = { hash2, hash3 };
    {
        newlyNeeded.clear();
        newlyUnneededPreviouslyNeeded.clear();
        PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
        PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
        PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
        PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
        ExpectListContainsExactly(needed, {});
        ExpectListContainsExactly(unneeded, {});
        ExpectListContainsExactly(newlyNeeded, {});
        ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, { hash2, hash3 });
        ExpectListContainsExactly(pendingUnneeded, { hash1, hash2, hash3 });
    }

    added = { hash1, hash2, hash3 };
    removed = {};
    {
        newlyNeeded.clear();
        newlyUnneededPreviouslyNeeded.clear();
        PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
        PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
        PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
        PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
        ExpectListContainsExactly(needed, { hash1, hash2, hash3 });
        ExpectListContainsExactly(unneeded, {});
        ExpectListContainsExactly(newlyNeeded, {});
        ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, {});
        ExpectListContainsExactly(pendingUnneeded, {});
    }

    added = {};
    removed = { hash1, hash2, hash3 };
    {
        newlyNeeded.clear();
        newlyUnneededPreviouslyNeeded.clear();
        PendingClientResourcesUtils::ConsolidateAddedResources(needed, unneeded, newlyNeeded, added);
        PendingClientResourcesUtils::ConsolidateRemovedResources(needed, unneeded, newlyUnneededPreviouslyNeeded, removed);
        PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeeded, pendingUnneeded);
        PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingUnneeded, newlyUnneededPreviouslyNeeded);
        ExpectListContainsExactly(needed, {});
        ExpectListContainsExactly(unneeded, {});
        ExpectListContainsExactly(newlyNeeded, {});
        ExpectListContainsExactly(newlyUnneededPreviouslyNeeded, { hash1, hash2, hash3 });
        ExpectListContainsExactly(pendingUnneeded, { hash1, hash2, hash3 });
    }

    PendingClientResourcesUtils::ConsolidateNeededAndUnneededResources(inUse, needed, unneeded);
    ExpectListContainsExactly(inUse, { hash4 });
}
