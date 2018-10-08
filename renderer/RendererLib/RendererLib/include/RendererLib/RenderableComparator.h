//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERABLECOMPARATOR_H
#define RAMSES_RENDERABLECOMPARATOR_H

#include "RendererLib/ResourceCachedScene.h"

namespace ramses_internal
{
    class RenderableComparator
    {
    public:
        RenderableComparator(const ResourceCachedScene& scene)
            : m_scene(scene)
        {
        }

        Bool operator()(const RenderableOrderEntry& renderableOrder1, const RenderableOrderEntry& renderableOrder2) const
        {
            if (renderableOrder1.order == renderableOrder2.order)
            {
                const Renderable& renderable1 = m_scene.getRenderable(renderableOrder1.renderable);
                const Renderable& renderable2 = m_scene.getRenderable(renderableOrder2.renderable);
                if (renderable1.effectResource == renderable2.effectResource)
                {
                    return renderable1.dataInstances[ERenderableDataSlotType_Geometry] < renderable2.dataInstances[ERenderableDataSlotType_Geometry];
                }

                return renderable1.effectResource < renderable2.effectResource;
            }

            return renderableOrder1.order < renderableOrder2.order;
        }

    private:
        const ResourceCachedScene& m_scene;
    };
}

#endif
