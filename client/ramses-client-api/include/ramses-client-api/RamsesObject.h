//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECT_H
#define RAMSES_RAMSESOBJECT_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-client-api/RamsesObjectTypes.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief The RamsesObject is a base class for all client API objects owned by the framework.
    */
    class RamsesObject : public StatusObject
    {
    public:
        /**
        * @brief Returns the name of the object.
        *
        * @return Name of the object
        */
        [[nodiscard]] RAMSES_API const char* getName() const;

        /**
        * @brief Changes the name of the object.
        *
        * @param name New name of the object
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setName(const char* name);

        /**
        * @brief Gets type of the object.
        *
        * @return Type of the object, see ERamsesObjectType enum for possible values.
        */
        [[nodiscard]] RAMSES_API ERamsesObjectType getType() const;

        /**
        * @brief Checks if the object is of given type.
        *
        * @param[in] type Type to check against.
        * @return True if object is of given type, ie. it can be converted to given type.
        */
        [[nodiscard]] RAMSES_API bool isOfType(ERamsesObjectType type) const;

        /**
        * Stores internal data for implementation specifics of RamsesObject.
        */
        class RamsesObjectImpl& m_impl;

    protected:
        /**
        * @brief Constructor for RamsesObject.
        *
        * @param[in] impl Internal data for implementation specifics of RamsesObject (sink - instance becomes owner)
        */
        explicit RamsesObject(std::unique_ptr<RamsesObjectImpl> impl);

        friend class RamsesObjectRegistry;
    };
}

#endif
