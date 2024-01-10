//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesObject.h"

namespace ramses
{
    namespace internal
    {
        class ClientObjectImpl;
    }

    /**
    * @brief The ClientObject is a base class for all client API objects owned by a RamsesClient.
    * @ingroup CoreAPI
    */
    class RAMSES_API ClientObject : public RamsesObject
    {
    public:
        /**
         * Get the internal data for implementation specifics of ClientObject.
         */
        [[nodiscard]] internal::ClientObjectImpl& impl();

        /**
         * Get the internal data for implementation specifics of ClientObject.
         */
        [[nodiscard]] const internal::ClientObjectImpl& impl() const;

    protected:
        /**
        * @brief Constructor for ClientObject.
        *
        * @param[in] impl Internal data for implementation specifics of ClientObject (sink - instance becomes owner)
        */
        explicit ClientObject(std::unique_ptr<internal::ClientObjectImpl> impl);

        /**
        * Stores internal data for implementation specifics of ClientObject.
        */
        internal::ClientObjectImpl& m_impl;
    };
}
