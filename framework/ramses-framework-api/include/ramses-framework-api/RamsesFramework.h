//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORK_H
#define RAMSES_RAMSESFRAMEWORK_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/APIExport.h"

#include <memory>
#include <string>
#include <cstdint>

namespace ramses
{
    class DcsmProvider;
    class DcsmConsumer;
    class RamsesRenderer;
    class RamsesClient;
    class RendererConfig;
    class IRamshCommand;

    /**
    * @brief Class representing ramses framework components that are needed
    *        to initialize an instance of ramses client and renderer.
    */
    class RAMSES_API RamsesFramework : public StatusObject
    {
    public:
        /**
        * @brief Default constructor
        */
        RamsesFramework();

        /**
        * @brief Constructor of RamsesFramework using command line parameters
        *
        * @param[in] argc Number of strings in argv array
        * @param[in] argv Command line parameters as array of string
        */
        RamsesFramework(int32_t argc, char const* const* argv);

        /**
        * @brief Constructor of RamsesFramework using RamsesFrameworkConfig object
        *
        * @param[in] config Configuration object
        */
        explicit RamsesFramework(const RamsesFrameworkConfig& config);

        /**
        * @brief Tries to establish a connection to the RAMSES system.
        *
        * If only local rendering is desired this does not need to be called - but all
        * scenes must be published in #ramses::EScenePublicationMode_LocalOnly.
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
        * @brief Create a new RamsesClient linked to this framework. Creation of multiple clients is
        *        supported. It might be disallowed to create any client depending on internal policy.
        *        Ownership of the client will remain with the framework.
        *
        *        Must create all client instances before calling #connect().
        *
        *        The created instance is valid until #destroyClient() is called or it will be
        *        automatically deleted in the RamsesFramework destructor.
        *
        * @param applicationName a name for the ramses client application
        *
        * @return The new RamsesClient object or nullptr if the creation failed or was denied.
        */
        RamsesClient* createClient(const char* applicationName);

        /**
        * @brief Destroy a RamsesClient created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        *        May not be called when connected.
        *
        * @param client the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyClient(RamsesClient& client);

        /**
        * @brief Create a new RamsesRenderer linked to this framework.
        *        Only one RamsesRenderer can exist per RamsesFramework instance. It might be disallowed
        *        to create any renderer depending on internal policy. Ownership of the renderer will
        *        remain with the framework.
        *
        *        Must create the renderer object before calling #connect().
        *
        *        The created instance is valid until #destroyRenderer() is called or it will be
        *        automatically deleted in the RamsesFramework destructor.
        *
        * @param config  Set of configuration flags and attributes for the ramses renderer
        *
        * @return The new RamsesRenderer object or nullptr if the creation failed or was denied.
        */
        RamsesRenderer* createRenderer(const RendererConfig& config);

        /**
        * @brief Destroy a RamsesRenderer created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        *        May not be called when connected.
        *
        * @param renderer the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyRenderer(RamsesRenderer& renderer);

        /**
        * @brief Create a new DcsmProvider linked to this framework.
        *        Only one DcsmProvider can exist per RamsesFramework instance. Depending on user
        *        the creation is not allowed and will always fail.
        *
        *        Must call #connect() before communication with remote Dcsm consumers is possible.
        *
        *        The created instance is valid until #destroyDcsmProvider() is called or it will be
        *        automatically deleted in the RamsesFramework destructor.
        *
        * @return The new DcsmProvider object or nullptr if the creation failed or was denied.
        */
        DcsmProvider* createDcsmProvider();

        /**
        * @brief Destroy a DcsmProvider created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *        Ownership of the provider will remain with the framework.
        *
        * @param provider the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDcsmProvider(const DcsmProvider& provider);

        /**
        * @brief Create a new DcsmConsumer linked to this framework.
        *        Only one DcsmConsumer can exist per RamsesFramework instance. It might be disallowed
        *        to create any consumer depending on internal policy.
        *        Ownership of the consumer will remain with the framework.
        *
        *        Must call #connect() before communication with remote Dcsm providers is possible.
        *
        *        The created instance is valid until #destroyDcsmConsumer() is called or it will be
        *        automatically deleted in the RamsesFramework destructor.
        *
        * @return The new DcsmConsumer object or nullptr if the creation failed or was denied.
        */
        DcsmConsumer* createDcsmConsumer();

        /**
        * @brief Destroy a DcsmConsumer created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        * @param consumer the object to destroy
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDcsmConsumer(const DcsmConsumer& consumer);

        /**
        * @brief Set the log level for all console log messages and apply it immediately.
        *
        * If this function is called before creation of RamsesFramework it will be overwritten
        * by cmdl args ("log-level", "log-level-console"), environment variable ("RAMSES_LOGLEVEL") and
        * by dynamically injected value by ramsh or dlt viewer/logger.
        * If this function is called after creation of RamsesFramework it can only be overwritten
        * dynamically by ramsh or dlt viewer/logger.
        *
        * @param[in] logLevel the log level to be applied
        */
        static void SetConsoleLogLevel(ELogLevel logLevel);

        /**
        * Sets a custom log handler function, which is called each time a log message occurs.
        * All messages will still be logged to the regular sinks (dlt, console)
        *
        * @ param logHandlerFunc function which is called for each log message
        * Set to nullptr to delete the handler
        *
        * Note: the logHandlerFunc will be called from inside ramses internal code, and it must return quickly and
        * not block, otherwise it will break the ramses internal rendering or distribution  logic.
        *
        * Additional restrictions:
        *  -# logHandlerFunc will be called from multiple threads and has to be thread safe
        *  -# logHandlerFunc may never call back into ramses code
        *  -# this method should not be used in target code, it is only for testing and tooling
        */
        static void SetLogHandler(const LogHandlerFunc& logHandlerFunc);

        /**
        * Gets the current value of the synchronized clock in milliseconds
        *
        * The synchronized clock is a common time source shared between ramses client and renderer.
        * Its implementation is system dependent. In the simplest case it refers to the system clock.
        * On systems with different ECUs it may refer to an external time source (ptp time).
        * Both renderer and client ECUs need to be built with the same type of synchronized clock.
        *
        * The synchronized clock is used for the scene expiration mechanism (#ramses::Scene::setExpirationTimestamp)
        *
        * @return current time point of synchronized clock in milliseconds
        */
        static uint64_t GetSynchronizedClockMilliseconds();

        /**
        * @brief Register a ramsh command that can be invoked via console and DLT injection
        *
        * This is for testing and debugging purpose only. Command injection is not guaranteed to work in production.
        *
        * The command has to be provided via shared_ptr to avoid lifetime issues. Internally ramses will only store
        * a std::weak_ptr to the command. Therefore it is valid to let go of the shared_ptr on caller side and expect
        * that no calls will happen in the command anymore. This allows to have user object references in the command
        * implementation with a shorter lifetime than RamsesFramework.
        *
        * It is not possible to delete commands. They are expected to be long-living and are bound to the lifetime
        * of the RamsesFramework object.
        *
        * @param[in] command the ramsh command
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addRamshCommand(const std::shared_ptr<IRamshCommand>& command);

        /**
        * @brief Execute a ramsh command programmatically
        *
        * Instead of typing the command in ramsh console or using DLT injection this function enables the user to
        * execute a ramsh command in code. The input is a string that contains the command and args.
        * For setting the consoleLogLevel the input string with command and argument would be: "setLogLevelConsole trace".
        *
        * @param[in] input a a string containing the ramsh command and args
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t executeRamshCommand(const std::string& input);

        /**
        * @brief Destructor of RamsesFramework
        *
        * This will destroy all objects created with this RamsesFramework instance (RamsesRenderer, RamsesClient,
        * DcsmProvider, DcsmConsumer), so there is no general need to explicitly destroy these objects individually,
        * if not specifically intended to do so.
        *
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
