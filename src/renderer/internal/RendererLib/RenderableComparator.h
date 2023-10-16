//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/ResourceCachedScene.h"

namespace ramses::internal
{
    class RenderableComparator
    {
    public:
        explicit RenderableComparator(const ResourceCachedScene& scene)
            : m_scene(scene)
        {
        }

        bool operator()(const RenderableOrderEntry& renderableOrder1, const RenderableOrderEntry& renderableOrder2) const
        {
            if (renderableOrder1.order == renderableOrder2.order)
            {
                const Renderable& renderable1 = m_scene.getRenderable(renderableOrder1.renderable);
                const Renderable& renderable2 = m_scene.getRenderable(renderableOrder2.renderable);

                ResourceContentHash effectHash1 = ResourceContentHash::Invalid();
                if (renderable1.dataInstances[ERenderableDataSlotType_Geometry].isValid())
                {
                    const DataLayoutHandle geometry1DataLayoutHandle = m_scene.getLayoutOfDataInstance(renderable1.dataInstances[ERenderableDataSlotType_Geometry]);
                    effectHash1 = m_scene.getDataLayout(geometry1DataLayoutHandle).getEffectHash();
                }

                ResourceContentHash effectHash2 = ResourceContentHash::Invalid();
                if (renderable2.dataInstances[ERenderableDataSlotType_Geometry].isValid())
                {
                    const DataLayoutHandle geometry2DataLayoutHandle = m_scene.getLayoutOfDataInstance(renderable2.dataInstances[ERenderableDataSlotType_Geometry]);
                    effectHash2 = m_scene.getDataLayout(geometry2DataLayoutHandle).getEffectHash();
                }

                if (effectHash1 == effectHash2)
                {
                    return renderable1.dataInstances[ERenderableDataSlotType_Geometry] < renderable2.dataInstances[ERenderableDataSlotType_Geometry];
                }

                return effectHash1 < effectHash2;
            }

            return renderableOrder1.order < renderableOrder2.order;
        }

    private:
        const ResourceCachedScene& m_scene;
    };
}
