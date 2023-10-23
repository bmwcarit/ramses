//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses/framework/Issue.h"
#include "ramses/framework/APIExport.h"

#include <memory>
#include <string>
#include <cstdint>
#include <string_view>
#include <optional>

namespace ramses
{
    namespace internal
    {
        class RamsesFrameworkImpl;
    }

    class RamsesRenderer;
    class RamsesClient;
    class RendererConfig;
    class IRamshCommand;

    /**
    * @brief Class representing ramses framework components that are needed
    *        to initialize an instance of ramses client and renderer.
    */
    class RAMSES_API RamsesFramework
    {
    public:
        /**
        * @brief Constructor of RamsesFramework
        *
        * @param[in] config Configuration object
        */
        explicit RamsesFramework(const RamsesFrameworkConfig& config);

        /**
        * @brief Tries to establish a connection to the RAMSES system.
        *
        * If only local rendering is desired this does not need to be called - but all
        * scenes must be published in #ramses::EScenePublicationMode::LocalOnly.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool connect();

        /**
        * @brief Check if the RamsesClient is connected or not
        *
        * @return true if connected, false otherwise.
        */
        [[nodiscard]] bool isConnected() const;

        /**
        * @brief Disconnects the RamsesClient from the system
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool disconnect();

        /**
        * Returns the feature level this #RamsesFramework instance was configured to use when created.
        * See #ramses::RamsesFrameworkConfig::setFeatureLevel and #ramses::EFeatureLevel for more details.
        *
        * @return feature level this #RamsesFramework instance was configured to use
        */
        [[nodiscard]] EFeatureLevel getFeatureLevel() const;

        /**
        * @brief Create a new #ramses::RamsesClient linked to this framework. Creation of multiple clients is
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
        * @return The new #ramses::RamsesClient object or nullptr if the creation failed or was denied.
        */
        RamsesClient* createClient(std::string_view applicationName);

        /**
        * @brief Destroy a #ramses::RamsesClient created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        *        May not be called when connected.
        *
        * @param client the object to destroy
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool destroyClient(RamsesClient& client);

        /**
        * @brief Create a new #ramses::RamsesRenderer linked to this framework.
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
        * @return The new #ramses::RamsesRenderer object or nullptr if the creation failed or was denied.
        */
        RamsesRenderer* createRenderer(const RendererConfig& config);

        /**
        * @brief Destroy a #ramses::RamsesRenderer created with this framework.
        *        This method will fail when handed an object created with another RamsesFramework.
        *
        *        May not be called when connected.
        *
        * @param renderer the object to destroy
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool destroyRenderer(RamsesRenderer& renderer);

        /**
        * Returns the last error which occurred during an API call within the Ramses SDK and clears it.
        * Subsequent calls to this method will return std:nullopt until some API call produces a new error.
        *
        * It is highly recommended to always check return value from all Ramses API (typically bool or pointer to object)
        * and in case there is problem (returned false or nullptr) use this method to get more details about what happened.
        * Note that all errors are also logged regardless if using this method or not.
        *
        * @return last error if any occurred and not already cleared by calling this method, std::nullopt otherwise
        */
        [[nodiscard]] std::optional<Issue> getLastError();

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
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool addRamshCommand(const std::shared_ptr<IRamshCommand>& command);

        /**
        * @brief Execute a ramsh command programmatically
        *
        * Instead of typing the command in ramsh console or using DLT injection this function enables the user to
        * execute a ramsh command in code. The input is a string that contains the command and args.
        * For setting the consoleLogLevel the input string with command and argument would be: "setLogLevelConsole trace".
        *
        * @param[in] input a a string containing the ramsh command and args
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool executeRamshCommand(const std::string& input);

        /**
        * @brief Destructor of RamsesFramework
        *
        * This will destroy all objects created with this RamsesFramework instance (#ramses::RamsesRenderer, #ramses::RamsesClient),
        * so there is no general need to explicitly destroy these objects individually, if not specifically intended to do so.
        *
        */
        ~RamsesFramework();

        /**
         * @brief Deleted copy constructor
         */
        RamsesFramework(const RamsesFramework&) = delete;

        /**
         * @brief Deleted move constructor
         */
        RamsesFramework(RamsesFramework&&) = delete;

        /**
         * @brief Deleted copy assignment
         * @return unused
         */
        RamsesFramework& operator=(const RamsesFramework&) = delete;

        /**
         * @brief Deleted move assignment
         * @return unused
         */
        RamsesFramework& operator=(RamsesFramework&&) = delete;

        /**
         * Get the internal data for implementation specifics of RamsesFramework.
         */
        [[nodiscard]] internal::RamsesFrameworkImpl& impl();

        /**
         * Get the internal data for implementation specifics of RamsesFramework.
         */
        [[nodiscard]] const internal::RamsesFrameworkImpl& impl() const;

    protected:
        /**
        * Stores internal data for implementation specifics of RamsesFramework
        */
        std::unique_ptr<internal::RamsesFrameworkImpl> m_impl;
    };
}
