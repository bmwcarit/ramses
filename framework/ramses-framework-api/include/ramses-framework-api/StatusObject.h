//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STATUSOBJECT_H
#define RAMSES_STATUSOBJECT_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/APIExport.h"
#include "EValidationSeverity.h"

namespace ramses
{
    /**
    * @brief The StatusObject provides status message handling
    */
    class RAMSES_API StatusObject
    {
    public:
        /**
        * @brief Generates verbose validation of the object.
        *        Checks validity of internal values, references, states and performance warnings.
        *        Result can be obtained by calling \c getValidationReport().
        *
        * @return StatusOK if no issues found, otherwise an index to a status message.
        *         The status message does not contain validation results, those can be obtained
        *         by calling \c getValidationReport().
        */
        status_t validate() const;

        /**
        * @brief Provides verbose report in human readable form generated by \c validate().
        *
        * If ValidationSeverity is set to EValidationSeverity_Error, only objects with errors and their descriptions
        * are returned, if filtering is set to EValidationSeverity_Warning additionally all objects with warnings and
        * their descriptions are returned. If filtering is set to EValidationSeverity_Info all errors and warnings are printed
        * and additionally some objects will print more information like number of instances or similar.
        *
        * @param[in] severity Optional level of severity the report should be filtered by
        * @return Validation string containing human readable status of the object,
        *         if \c validate() was not called a pointer to an empty string is returned.
        */
        const char* getValidationReport(EValidationSeverity severity = EValidationSeverity_Info) const;

        /**
        * @brief Get the string description for a status provided by a RAMSES API function
        *
        * @param status Status returned by a RAMSES API function call
        * @return If value refers to an existing status message, the string with text description for status is returned.
        *         If no status message for value is available, unknown status message is returned.
        */
        const char* getStatusMessage(status_t status) const;

        /**
        * Stores internal data for implementation specifics of StatusObject.
        */
        class StatusObjectImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        StatusObject(const StatusObject& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        StatusObject& operator=(const StatusObject& other) = delete;

    protected:
        /**
        * @brief Constructor for StatusObject.
        *
        * @param[in] pimpl Internal data for implementation specifics of StatusObject (sink - instance becomes owner)
        */
        explicit StatusObject(StatusObjectImpl& pimpl);

        /**
        * @brief Destructor of the StatusObject
        */
        virtual ~StatusObject();
    };
}

#endif // RAMSES_STATUSOBJECT_H