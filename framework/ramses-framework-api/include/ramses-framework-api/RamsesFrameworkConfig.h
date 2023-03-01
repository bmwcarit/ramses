//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKCONFIG_H
#define RAMSES_RAMSESFRAMEWORKCONFIG_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/IThreadWatchdogNotification.h"
#include "ramses-framework-api/APIExport.h"
#include "ramses-framework-api/EFeatureLevel.h"

namespace CLI
{
    class App;
}

namespace ramses
{
    /**
    * @brief The RamsesFrameworkConfig holds a set of parameters to be used
    * to initialize ramses.
    */
    class RAMSES_API RamsesFrameworkConfig : public StatusObject
    {
    public:
        /**
        * @brief Default constructor of RamsesFrameworkConfig
        */
        RamsesFrameworkConfig();

        /**
        * @brief Destructor of RamsesFrameworkConfig
        */
        ~RamsesFrameworkConfig() override;

        /**
        * @brief Register command line options for the CLI11 command line parser
        *
        * Creates an option group "Framework Options" and registers command line options
        * After parsing the command line with CLI::App::parser() this config object is assigned with the values provided by command line
        *
        * @param[in] cli CLI11 command line parser
        */
        void registerOptions(CLI::App& cli);

        /**
        * @brief Set feature level
        *
        * Sets feature level that will be used when creating #ramses::RamsesFramework.
        * #ramses::RamsesClient and #ramses::RamsesRenderer created from this framework will only be able to connect
        * to client/renderer using a compatible feature level.
        * Only files exported using the exact same feature level can be loaded into #ramses::RamsesClient created
        * from this framework.
        * See #ramses::EFeatureLevel for more details.
        *
        * @param[in] featureLevel feature level to use (default is #ramses::EFeatureLevel_01)
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setFeatureLevel(EFeatureLevel featureLevel);

        /**
        * @brief Get feature level
        *
        * Gets feature level that was set using #setFeatureLevel.
        *
        * @return currently set feature level.
        */
        [[nodiscard]] EFeatureLevel getFeatureLevel() const;

        /**
         * @brief Request a certain type of ramses shell
         * @param[in] requestedShellType type of ramses shell
         * @return StatusOK on success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setRequestedRamsesShellType(ERamsesShellType requestedShellType);

        /**
        * @brief Set watchdog notification interval of ramses threads
        *
        * @param[in] thread which thread identifier to set the interval for
        * @param[in] interval interval in ms which is used to call given callback
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval);

        /**
        * @brief Set watchdog callback
        *
        * notifyThread Method will be called in the interval specified
        * registerThread and unregisterThread are called to signal which threads will be calling the callback,
        * and should be 'monitored' by user code in an appropriate way.
        *
        * @param[in] callback callback class to use for watchdog reporting
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback);

        /**
         * @brief Disable DLT application registration
         *
         * When set and DLT is enabled ramses expects DLT_REGISTER_APP being called before
         * RamsesFramework construction and DLT_UNREGISTER_APP after RamsesFramework destruction.
         * Ramses will add its context to the existing application.
         *
         * When not disabled, ramses will manage DLT application registration itself.
         *
         * @return StatusOK on success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t disableDLTApplicationRegistration();

        /**
        * @brief Set the application ID name for DLT (4 chars)
        *
        * @param[in] id to use as DLT application id
        */
        void setDLTApplicationID(const char* id);

        /**
        * @brief Return the DLT application id value set in configuration object
        *
        * @return DLT application id value set in this configuration object
        */
        [[nodiscard]] const char* getDLTApplicationID() const;

        /**
        * @brief Set the application description for DLT
        *
        * @param[in] description to use as DLT application description
        */
        void setDLTApplicationDescription(const char* description);

        /**
        * @brief Return the DLT application description set in configuration object
        *
        * @return DLT application description set in this configuration object
        */
        [[nodiscard]] const char* getDLTApplicationDescription() const;

        /**
        * @brief Enables or disables the periodic log messages provided by the Ramses framework
        *
        * If enabled the Ramses framework periodically logs information about the Ramses version, connected participants, scene states etc.
        * Please leave enabled in a production environment as the provided information is important for error analysis.
        *
        * The default value is enabled.
        *
        * @param[in] enabled If true the periodic logs are enabled
        */
        void setPeriodicLogsEnabled(bool enabled);

        /**
        * @brief Sets the IP address that is used to select the local network interface
        * The value is only evaluated if SOME/IP is not used. This communication type is intended for prototype use-cases only.
        *
        * @param[in] ip IP to use
        */
        void setInterfaceSelectionIPForTCPCommunication(const char* ip);

        /**
        * @brief Sets the IP address of the communication daemon
        * The value is only evaluated if SOME/IP is not used. This communication type is intended for prototype use-cases only.
        *
        * @param[in] ip IP to use
        */
        void setDaemonIPForTCPCommunication(const char* ip);

        /**
        * @brief Sets the port of the communication daemon
        * The value is only evaluated if SOME/IP is not used. This communication type is intended for prototype use-cases only.
        *
        * @param[in] port Port to use
        */
        void setDaemonPortForTCPCommunication(uint16_t port);

        /**
        * Stores internal data for implementation specifics of RamsesFrameworkConfig
        */
        class RamsesFrameworkConfigImpl& impl;

        /**
         * @brief Deleted copy constructor
         */
        RamsesFrameworkConfig(const RamsesFrameworkConfig&) = delete;

        /**
         * @brief Deleted copy assignment
         * @return unused
         */
        RamsesFrameworkConfig& operator=(const RamsesFrameworkConfig&) = delete;
    };

}

#endif
