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
    * @brief The ClientObject is a base class for all client API objects owned by a RamsesClient.
    */
    class RAMSES_API ClientObject : public RamsesObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of ClientObject.
        */
        class ClientObjectImpl& impl;

    protected:
        /**
        * @brief Constructor for ClientObject.
        *
        * @param[in] pimpl Internal data for implementation specifics of ClientObject (sink - instance becomes owner)
        */
        explicit ClientObject(ClientObjectImpl& pimpl);

        /**
        * @brief Destructor of the ClientObject
        */
        ~ClientObject() override;

        /**
        * @brief RamsesClientImpl is the factory for creating client objects.
        */
        friend class RamsesClientImpl;

    private:
        /**
        * @brief Copy constructor of ClientObject
        */
        ClientObject(const ClientObject& other);

        /**
        * @brief Assignment operator of ClientObject.
        *
        * @param[in] other Instance to assign from
        * @return This instance after assignment
        */
        ClientObject& operator=(const ClientObject& other);
    };
}

#endif
