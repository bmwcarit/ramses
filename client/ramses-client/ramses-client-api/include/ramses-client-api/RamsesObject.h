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
    * @brief The RamsesObject is a base class for all client API objects owned by the framework.
    */
    class RAMSES_API RamsesObject : public StatusObject
    {
    public:
        /**
        * @brief Returns the name of the object.
        *
        * @return Name of the object
        */
        [[nodiscard]] const char* getName() const;

        /**
        * @brief Changes the name of the object.
        *
        * @param name New name of the object
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setName(const char* name);

        /**
        * @brief Gets type of the object.
        *
        * @return Type of the object, see ERamsesObjectType enum for possible values.
        */
        [[nodiscard]] ERamsesObjectType getType() const;

        /**
        * @brief Checks if the object is of given type.
        *
        * @param[in] type Type to check against.
        * @return True if object is of given type, ie. it can be converted to given type.
        */
        [[nodiscard]] bool isOfType(ERamsesObjectType type) const;

        /**
        * Stores internal data for implementation specifics of RamsesObject.
        */
        class RamsesObjectImpl& impl;

    protected:
        /**
        * @brief Constructor for RamsesObject.
        *
        * @param[in] pimpl Internal data for implementation specifics of RamsesObject (sink - instance becomes owner)
        */
        explicit RamsesObject(RamsesObjectImpl& pimpl);

        /**
        * @brief Destructor of the RamsesObject
        */
        virtual ~RamsesObject();

    private:
        /**
        * @brief Copy constructor of RamsesObject
        */
        RamsesObject(const RamsesObject& other);

        /**
        * @brief Assignment operator of RamsesObject.
        *
        * @param[in] other Instance to assign from
        * @return This instance after assignment
        */
        RamsesObject& operator=(const RamsesObject& other);
    };
}

#endif
