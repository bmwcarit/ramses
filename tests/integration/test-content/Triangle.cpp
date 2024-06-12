//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/Triangle.h"

#include "ramses/client/Scene.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"
#include "ramses/client/DataObject.h"

#include <cstdint>
#include <cassert>

namespace ramses::internal
{
    Triangle::Triangle(ramses::Scene& scene, const Effect& effect, enum TriangleAppearance::EColor color, float alpha, TriangleGeometry::EVerticesOrder vertOrder)
        : m_appearance(scene, effect, color, alpha)
        , m_geometry(scene, effect, vertOrder)
    {
    }

    void Triangle::setColor(TriangleAppearance::EColor color, float alpha)
    {
        m_appearance.setColor(color, alpha);
    }

    void Triangle::bindColor(const DataObject& colorDataObject)
    {
        m_appearance.bindColor(colorDataObject);
    }

    void Triangle::unbindColor()
    {
        m_appearance.unbindColor();
    }
}
