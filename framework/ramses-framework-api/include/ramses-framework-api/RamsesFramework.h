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
#include "ramses-framework-api/APIExport.h"
#include "ramses-framework-api/StatusObject.h"

namespace ramses
{
    class DcsmProvider;
    class DcsmConsumer;

    /**
    * @brief Class representing ramses framework components that are needed
    *        to initialize an instance of ramses client and renderer.
    */
    class RAMSES_API RamsesFramework : public StatusObject
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
        * @brief Create a new DcsmProvider linked to this framework.
        *        Only one DcsmProvider can exist per RamsesFramework instance. Depending on user
        *        the creation is not allowed and will always fail.
        *
        *        Must call connect() before communication with remote Dcsm consumers is possible.
        *
        * @return The new DcsmProvider object or nullptr if the creation failed or was denied.
        */
        DcsmProvider* createDcsmProvider();

        /**
        * @brief Create a new DcsmConsumer linked to this framework.
        *        Only one DcsmConsumer can exist per RamsesFramework instance. It might be disallowed
        *        to create any consumer depending on internal policy.
        *
        *        Must call connect() before communication with remote Dcsm providers is possible.
        *
        * @return The new DcsmConsumer object or nullptr if the creation failed or was denied.
        */
        DcsmConsumer* createDcsmConsumer();

        /**
        * @brief Destroy a DcsmProvider created with this framework.
        *        This method with failed when handed an object created with another RamsesFramework.
        *
        * @param provider the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDcsmProvider(const DcsmProvider& provider);

        /**
        * @brief Destroy a DcsmConsumer created with this framework.
        *        This method with failed when handed an object created with another RamsesFramework.
        *
        * @param consumer the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDcsmConsumer(const DcsmConsumer& consumer);

        /**
        * @brief Destructor of RamsesFramework
        */
        ~RamsesFramework() override;

        /**
        * Stores internal data for implementation specifics of RamsesFramework
        */
        class RamsesFrameworkImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        RamsesFramework(const RamsesFramework& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RamsesFramework& operator=(const RamsesFramework& other) = delete;
    };
}

#endif
