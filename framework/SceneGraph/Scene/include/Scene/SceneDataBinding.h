//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEDATABINDING_H
#define RAMSES_SCENEDATABINDING_H

#include "SceneAPI/IScene.h"
#include "Utils/DataBindCommon.h"

namespace ramses_internal
{
    DATA_BIND_DECLARE_BEGIN(IScene, 12)
        DATA_BIND_DECLARE( 0, TransformNode_Translation)
        DATA_BIND_DECLARE( 1, TransformNode_Rotation)
        DATA_BIND_DECLARE( 2, TransformNode_Scaling)
        DATA_BIND_DECLARE( 3, DataField_Float)
        DATA_BIND_DECLARE( 4, DataField_Vector2f)
        DATA_BIND_DECLARE( 5, DataField_Vector3f)
        DATA_BIND_DECLARE( 6, DataField_Vector4f)
        DATA_BIND_DECLARE( 7, DataField_Integer)
        DATA_BIND_DECLARE( 8, DataField_Vector2i)
        DATA_BIND_DECLARE( 9, DataField_Vector3i)
        DATA_BIND_DECLARE(10, DataField_Vector4i)
        DATA_BIND_DECLARE(11, DataField_Matrix44f)
    DATA_BIND_DECLARE_END(IScene, EDataBindContainerType_Scene)
}

#endif

