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

namespace ramses_internal
{
    class IScene;
    class Variant;

    class DataInstanceHelper
    {
    public:
        static void CopyInstanceFieldData(const IScene& srcScene, DataInstanceHandle srcDataInstance, DataFieldHandle srcDataField,
            IScene& dstScene, DataInstanceHandle dstDataInstance, DataFieldHandle dstDataField);

        static void GetInstanceFieldData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, Variant& value);
        static void SetInstanceFieldData(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, const Variant& value);
    };
}

#endif
