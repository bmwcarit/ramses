//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cassert>

#include "PlatformAbstraction/PlatformTypes.h"

#include "TestScenes/Triangle.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/DataObject.h"

namespace ramses
{
    Triangle::Triangle(Scene& scene, const Effect& effect, enum TriangleAppearance::EColor color, float alpha, TriangleGeometry::EVerticesOrder vertOrder)
        : m_appearance(scene, effect, color, alpha)
        , m_geometry(scene, effect, vertOrder)
    {
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
