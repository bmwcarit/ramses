//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAINSTANCEHELPER_H
#define RAMSES_DATAINSTANCEHELPER_H

#include "SceneAPI/Handles.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"

#include "PlatformAbstraction/VariantWrapper.h"

namespace ramses_internal
{
    class IScene;

    using DataInstanceValueVariant = absl::variant<
        absl::monostate,
        float,
        int32_t,
        Vector2,
        Vector3,
        Vector4,
        Vector2i,
        Vector3i,
        Vector4i,
        Matrix22f,
        Matrix33f,
        Matrix44f>;

    class DataInstanceHelper
    {
    public:
        static void CopyInstanceFieldData(const IScene& srcScene, DataInstanceHandle srcDataInstance, DataFieldHandle srcDataField,
            IScene& dstScene, DataInstanceHandle dstDataInstance, DataFieldHandle dstDataField);

        static void GetInstanceFieldData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, DataInstanceValueVariant& value);
        static void SetInstanceFieldData(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, const DataInstanceValueVariant& value);
    };
}

#endif
