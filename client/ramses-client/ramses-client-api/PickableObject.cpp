//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

//API
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/LocalCamera.h"

//internal
#include "PickableObjectImpl.h"
#include "RamsesFrameworkTypesImpl.h"

namespace ramses
{
    PickableObject::PickableObject(PickableObjectImpl& pimpl)
        : Node(pimpl)
        , impl(pimpl)
    {
    }

    PickableObject::~PickableObject() = default;

    const ArrayBuffer& PickableObject::getGeometryBuffer() const
    {
        return impl.getGeometryBuffer();
    }

    const LocalCamera* PickableObject::getCamera() const
    {
        return impl.getCamera();
    }

    status_t PickableObject::setCamera(const LocalCamera& camera)
    {
        const status_t status = impl.setCamera(camera.impl);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(camera));
        return status;
    }

    pickableObjectId_t PickableObject::getPickableObjectId() const
    {
        return impl.getPickableObjectId();
    }

    status_t PickableObject::setPickableObjectId(pickableObjectId_t id)
    {
        const status_t status = impl.setPickableObjectId(id);
        LOG_HL_CLIENT_API1(status, id);
        return status;
    }

    status_t PickableObject::setEnabled(bool enable)
    {
        const status_t status = impl.setEnabled(enable);
        LOG_HL_CLIENT_API1(status, enable);
        return status;
    }

    bool PickableObject::isEnabled() const
    {
        return impl.isEnabled();
    }
}
