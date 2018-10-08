//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_RENDERGROUPUTILS_H
#define RAMSES_INTERNAL_RENDERGROUPUTILS_H

#include "SceneAPI/RenderGroup.h"
#include "SceneAPI/RenderPass.h"

namespace ramses_internal
{
    class RenderGroupUtils
    {
    public:
        static RenderableOrderVector::const_iterator FindRenderableEntry(RenderableHandle handle, const RenderGroup& rg)
        {
            return std::find_if(rg.renderables.cbegin(), rg.renderables.cend(), [handle](const RenderableOrderEntry& r) { return r.renderable == handle; });
        }

        static RenderableOrderVector::iterator FindRenderableEntry(RenderableHandle handle, RenderGroup& rg)
        {
            return std::find_if(rg.renderables.begin(), rg.renderables.end(), [handle](const RenderableOrderEntry& r) { return r.renderable == handle; });
        }

        static Bool ContainsRenderable(RenderableHandle handle, const RenderGroup& rg)
        {
            return std::find_if(rg.renderables.cbegin(), rg.renderables.cend(), [handle](const RenderableOrderEntry& r) { return r.renderable == handle; }) != rg.renderables.cend();
        }

        static RenderGroupOrderVector::const_iterator FindRenderGroupEntry(RenderGroupHandle handle, const RenderGroup& rg)
        {
            return std::find_if(rg.renderGroups.cbegin(), rg.renderGroups.cend(), [handle](const RenderGroupOrderEntry& r) { return r.renderGroup == handle; });
        }

        static RenderGroupOrderVector::iterator FindRenderGroupEntry(RenderGroupHandle handle, RenderGroup& rg)
        {
            return std::find_if(rg.renderGroups.begin(), rg.renderGroups.end(), [handle](const RenderGroupOrderEntry& r) { return r.renderGroup == handle; });
        }

        static Bool ContainsRenderGroup(RenderGroupHandle handle, const RenderGroup& rg)
        {
            return std::find_if(rg.renderGroups.cbegin(), rg.renderGroups.cend(), [handle](const RenderGroupOrderEntry& r) { return r.renderGroup == handle; }) != rg.renderGroups.cend();
        }

        static RenderGroupOrderVector::const_iterator FindRenderGroupEntry(RenderGroupHandle handle, const RenderPass& rp)
        {
            return std::find_if(rp.renderGroups.cbegin(), rp.renderGroups.cend(), [handle](const RenderGroupOrderEntry& r) { return r.renderGroup == handle; });
        }

        static RenderGroupOrderVector::iterator FindRenderGroupEntry(RenderGroupHandle handle, RenderPass& rp)
        {
            return std::find_if(rp.renderGroups.begin(), rp.renderGroups.end(), [handle](const RenderGroupOrderEntry& r) { return r.renderGroup == handle; });
        }

        static Bool ContainsRenderGroup(RenderGroupHandle handle, const RenderPass& rp)
        {
            return std::find_if(rp.renderGroups.cbegin(), rp.renderGroups.cend(), [handle](const RenderGroupOrderEntry& r) { return r.renderGroup == handle; }) != rp.renderGroups.cend();
        }
    };
}

#endif
