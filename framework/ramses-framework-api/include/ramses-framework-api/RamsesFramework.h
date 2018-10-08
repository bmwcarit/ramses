//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORK_H
#define RAMSES_RAMSESFRAMEWORK_H

#include "stdint.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "RamsesFrameworkConfig.h"
#include "APIExport.h"

namespace ramses
{
    /**
    * @brief Class representing ramses framework components that are needed
    *        to initialize an instance of ramses client and renderer.
    */
    class RAMSES_API RamsesFramework
    {
    public:
        /**
        * @brief Constructor of RamsesFramework using command line parameters
        *
        * @param[in] argc Number of strings in argv array
        * @param[in] argv Command line parameters as array of string
        */
        explicit RamsesFramework(int32_t argc = 0, const char * argv[] = 0);

        /**
        * @brief Constructor of RamsesFramework using command line parameters
        *
        * @param[in] argc Number of strings in argv array
        * @param[in] argv Command line parameters as array of string
        */
        RamsesFramework(int32_t argc, char * argv[]);

        /**
        * @brief Constructor of RamsesFramework using RamsesFrameworkConfig object
        *
        * @param[in] config Configuration object
        */
        explicit RamsesFramework(const RamsesFrameworkConfig& config);

        /**
        * @brief Tries to establish a connection to the RAMSES system.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t connect();

        /**
        * @brief Check if the RamsesClient is connected or not
        *
        * @return true if connected, false otherwise.
        */
        bool isConnected() const;

        /**
        * @brief Disconnects the RamsesClient from the system
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t disconnect();

        /**
        * @brief Destructor of RamsesFramework
        */
        ~RamsesFramework();

        /**
        * Stores internal data for implementation specifics of RamsesFramework
        */
        class RamsesFrameworkImpl& impl;

    private:
        /**
        * @brief Copy constructor of RamsesFramework
        */
        RamsesFramework(const RamsesFramework&);
    };
}

#endif
