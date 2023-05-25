//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTOBJECT_H
#define RAMSES_CLIENTOBJECT_H

#include "ramses-client-api/RamsesObject.h"

namespace ramses
{
    /**
   * @ingroup CoreAPI
    * @brief The ClientObject is a base class for all client API objects owned by a RamsesClient.
    */
    class ClientObject : public RamsesObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of ClientObject.
        */
        class ClientObjectImpl& m_impl;

    protected:
        /**
        * @brief Constructor for ClientObject.
        *
        * @param[in] impl Internal data for implementation specifics of ClientObject (sink - instance becomes owner)
        */
        explicit ClientObject(std::unique_ptr<ClientObjectImpl> impl);
    };
}

#endif
