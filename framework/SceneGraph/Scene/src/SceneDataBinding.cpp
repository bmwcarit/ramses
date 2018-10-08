//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneDataBinding.h"

namespace ramses_internal
{
    DATA_BIND_DEFINE_BEGIN(IScene)
        DATA_BIND_DEFINE( 0,  IScene,  IScene::getTranslation,           IScene::setTranslation,           IScene::isTransformAllocated,     EDataTypeID_Vector3f,  EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE( 1,  IScene,  IScene::getRotation,              IScene::setRotation,              IScene::isTransformAllocated,     EDataTypeID_Vector3f,  EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE( 2,  IScene,  IScene::getScaling,               IScene::setScaling,               IScene::isTransformAllocated,     EDataTypeID_Vector3f,  EDataBindAccessorType_Handles_1)
        DATA_BIND_DEFINE( 3,  IScene,  IScene::getDataSingleFloat,       IScene::setDataSingleFloat,       IScene::isDataInstanceAllocated,  EDataTypeID_Float,     EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE( 4,  IScene,  IScene::getDataSingleVector2f,    IScene::setDataSingleVector2f,    IScene::isDataInstanceAllocated,  EDataTypeID_Vector2f,  EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE( 5,  IScene,  IScene::getDataSingleVector3f,    IScene::setDataSingleVector3f,    IScene::isDataInstanceAllocated,  EDataTypeID_Vector3f,  EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE( 6,  IScene,  IScene::getDataSingleVector4f,    IScene::setDataSingleVector4f,    IScene::isDataInstanceAllocated,  EDataTypeID_Vector4f,  EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE( 7,  IScene,  IScene::getDataSingleInteger,     IScene::setDataSingleInteger,     IScene::isDataInstanceAllocated,  EDataTypeID_Int32,     EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE( 8,  IScene,  IScene::getDataSingleVector2i,    IScene::setDataSingleVector2i,    IScene::isDataInstanceAllocated,  EDataTypeID_Vector2i,  EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE( 9,  IScene,  IScene::getDataSingleVector3i,    IScene::setDataSingleVector3i,    IScene::isDataInstanceAllocated,  EDataTypeID_Vector3i,  EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE(10,  IScene,  IScene::getDataSingleVector4i,    IScene::setDataSingleVector4i,    IScene::isDataInstanceAllocated,  EDataTypeID_Vector4i,  EDataBindAccessorType_Handles_2)
        DATA_BIND_DEFINE(11,  IScene,  IScene::getDataSingleMatrix44f,   IScene::setDataSingleMatrix44f,   IScene::isDataInstanceAllocated,  EDataTypeID_Matrix44f, EDataBindAccessorType_Handles_2)
        // Add test for added entries to SceneTest_DataBinding
    DATA_BIND_DEFINE_END()
}
