//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/PendingClientResourcesUtils.h"
#include "SceneUtils/ResourceChangeUtils.h"

namespace ramses_internal
{
    void PendingClientResourcesUtils::ConsolidateAddedResources(
        ResourceContentHashVector& needed,
        ResourceContentHashVector& unneeded,
        ResourceContentHashVector& newlyNeeded,
        const ResourceContentHashVector& addedResources)
    {
        assert(newlyNeeded.empty());

        needed.reserve(needed.size() + addedResources.size());
        newlyNeeded.reserve(addedResources.size());

        for (const auto& addedResource : addedResources)
        {
            const auto it = find_c(unneeded, addedResource);
            if (it == unneeded.end())
            {
                // add as needed resource only if not already unneeded
                assert(!contains_c(needed, addedResource));
                needed.push_back(addedResource);
                // resource is newly needed
                newlyNeeded.push_back(addedResource);
            }
            else
            {
                // ignore added resource which was unneeded before
                unneeded.erase(it);
            }
        }
    }

    void PendingClientResourcesUtils::ConsolidateRemovedResources(
        ResourceContentHashVector& needed,
        ResourceContentHashVector& unneeded,
        ResourceContentHashVector& newlyUnneededPreviouslyNeeded,
        const ResourceContentHashVector& removedResources)
    {
        unneeded.reserve(unneeded.size() + removedResources.size());

        for (const auto& removedResource : removedResources)
        {
            const auto it = find_c(needed, removedResource);
            if (it == needed.end())
            {
                // add as unneeded resource only if not already needed
                assert(!contains_c(unneeded, removedResource));
                unneeded.push_back(removedResource);
            }
            else
            {
                // removed resource which was needed before is not needed anymore
                needed.erase(it);
                // resource is newly unneeded but was previously needed
                newlyUnneededPreviouslyNeeded.push_back(removedResource);
            }
        }
    }

    void PendingClientResourcesUtils::ConsolidateNewlyNeededResources(
        ResourceContentHashVector& newlyNeeded,
        ResourceContentHashVector& pendingUnneeded)
    {
        ResourceContentHashVector consolidatedNewlyNeeded;
        consolidatedNewlyNeeded.reserve(newlyNeeded.size());

        for (const auto& res : newlyNeeded)
        {
            ResourceChangeUtils::ConsolidateResource(consolidatedNewlyNeeded, pendingUnneeded, res);
        }

        newlyNeeded = consolidatedNewlyNeeded;
    }

    void PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(
        ResourceContentHashVector& pendingUnneeded,
        const ResourceContentHashVector& newlyUnneededPreviouslyNeeded)
    {
        for (const auto& res : newlyUnneededPreviouslyNeeded)
        {
            if (!contains_c(pendingUnneeded, res))
            {
                pendingUnneeded.push_back(res);
            }
        }
    }

    bool containSameElement(const ResourceContentHashVector& A, const ResourceContentHashVector& B)
    {
        for (const auto& res : A)
        {
            if (contains_c(B, res))
            {
                return true;
            }
        }

        return false;
    }

    void PendingClientResourcesUtils::ConsolidateNeededAndUnneededResources(
        ResourceContentHashVector& inUse,
        const ResourceContentHashVector& needed,
        const ResourceContentHashVector& unneeded)
    {
        assert(!containSameElement(needed, unneeded));
        assert(!containSameElement(needed, inUse));

        for (const auto& res : unneeded)
        {
            assert(!contains_c(needed, res));
            assert(contains_c(inUse, res));
            inUse.erase(find_c(inUse, res));
        }

        inUse.insert(inUse.end(), needed.begin(), needed.end());
    }
}
