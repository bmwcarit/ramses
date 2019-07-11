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
        * @brief Constructor of RamsesFrameworkConfig using command line parameters
        *
        * @param[in] argc Number of strings in argv array
        * @param[in] argv Command line parameters as array of string
        */
        explicit RamsesFrameworkConfig(int32_t argc = 0, char const* const* argv = 0);

        /**
        * @brief Constructor of RamsesFrameworkConfig using command line parameters
        *
        * @param[in] argc Number of strings in argv array
        * @param[in] argv Command line parameters as array of string
        */
        RamsesFrameworkConfig(int32_t argc, char * argv[]);

        /**
        * @brief Destructor of RamsesFrameworkConfig
        */
        ~RamsesFrameworkConfig() override;

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
        * @brief Set the application ID name for DLT (4 chars)
        *
        * @param[in] id to use as DLT application id
        */
        void setDLTApplicationID(const char* id);

        /**
        * @brief Return the DLT application id value set in configuration object
        *
        * @return dlt application id value set in this configuration object
        */
        const char* getDLTApplicationID() const;

        /**
        * @brief Set the application description for DLT
        *
        * @param[in] description to use as DLT application description
        */
        void setDLTApplicationDescription(const char* description);

        /**
        * @brief Return the DLT application description set in configuration object
        *
        * @return dlt application description set in this configuration object
        */
        const char* getDLTApplicationDescription() const;

        /**
        * @brief Sets the maximum bytes (heap) that will be used during asynchronous resource loading
        *
        * During loading of resources from assets, resources are loaded from file to cpu memory, then
        * uploaded to GPU and then deleted from main memory again. To speed up this process resource loading to CPU
        * memory and uploading to GPU happens at the same time (with different resources of course). The value given
        * to this function determines the maximum number of bytes that will be used by this system before waiting for GPU
        * uploads and therefore memory becoming available again. Basically higher value means higher peak heap usage, but
        * highest maximum parallelism for loading resources from files into GPU.
        *
        * The default value is 20 * 1024 * 1024 (20MB). If your resource files are larger you can benefit from a higher value.
        *
        * @param[in] maximumTotalBytesForAsynResourceLoading Maximum resource size in bytes
        */
        void setMaximumTotalBytesAllowedForAsyncResourceLoading(uint32_t maximumTotalBytesForAsynResourceLoading);

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
         * @param other unused
         */
        RamsesFrameworkConfig(const RamsesFrameworkConfig& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RamsesFrameworkConfig& operator=(const RamsesFrameworkConfig& other) = delete;
    };

}

#endif
