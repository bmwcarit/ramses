//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Scene/Scene.h"
#include "Scene/SceneDataBinding.h"
#include "Utils/DataBind.h"

using namespace testing;

namespace ramses_internal
{
    TEST(SceneDataBinding, canCreateDataBindToScene)
    {
        Scene scene;
        typedef DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType BindTraits;

        DataBind<IScene, Vector3,   NodeHandle> dataBind0 (scene, NodeHandle::Invalid(), BindTraits::TransformNode_Translation);
        DataBind<IScene, Vector3,   NodeHandle> dataBind1 (scene, NodeHandle::Invalid(), BindTraits::TransformNode_Rotation);
        DataBind<IScene, Vector3,   NodeHandle> dataBind2 (scene, NodeHandle::Invalid(), BindTraits::TransformNode_Scaling);

        DataBind<IScene, Float,     NodeHandle, NodeHandle> dataBind5 (scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Float);
        DataBind<IScene, Vector2,   NodeHandle, NodeHandle> dataBind6 (scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Vector2f);
        DataBind<IScene, Vector3,   NodeHandle, NodeHandle> dataBind7 (scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Vector3f);
        DataBind<IScene, Vector4,   NodeHandle, NodeHandle> dataBind8 (scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Vector4f);
        DataBind<IScene, Int32,     NodeHandle, NodeHandle> dataBind9 (scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Integer);
        DataBind<IScene, Vector2i,  NodeHandle, NodeHandle> dataBind10(scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Vector2i);
        DataBind<IScene, Vector3i,  NodeHandle, NodeHandle> dataBind11(scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Vector3i);
        DataBind<IScene, Vector4i,  NodeHandle, NodeHandle> dataBind12(scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Vector4i);
        DataBind<IScene, Matrix44f, NodeHandle, NodeHandle> dataBind13(scene, NodeHandle::Invalid(), NodeHandle::Invalid(), BindTraits::DataField_Matrix44f);
    }
}
