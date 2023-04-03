//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/DataObject.h"

//internal
#include "DataObjectImpl.h"

namespace ramses
{
    DataObject::DataObject(DataObjectImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    DataObject::~DataObject()
    {
    }
}
