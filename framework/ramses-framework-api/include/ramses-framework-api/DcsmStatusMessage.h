//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMSTATUSMESSAGE_H
#define RAMSES_DCSMSTATUSMESSAGE_H

#include "ramses-framework-api/APIExport.h"
#include <stdint.h>
#include <memory>

#if defined(_MSC_VER)
__pragma(warning(push))
__pragma(warning(disable: 4251))
#endif

namespace ramses
{
    class DcsmStatusMessageImpl;
    /**
     * @brief The base class of a message to be sent from DCSM consumer to provider.
     */
    class RAMSES_API DcsmStatusMessage
    {
    public:
        /**
         * @brief Convenience function to get the derived class of DcsmStatusMessage easily.
         * @return Pointer to StreamStatusMessage class if type of this message is StreamStatus, nullptr otherwise.
         */
        class StreamStatusMessage const* getAsStreamStatus() const;

        /**
         * @brief Convenience function to get the derived class of DcsmStatusMessage easily.
         * @return Pointer to ActiveLayoutMessage class if type of this message is ActiveLayout, nullptr otherwise.
         */
        class ActiveLayoutMessage const* getAsActiveLayout() const;

        /**
         * @brief Convenience function to get the derived class of DcsmStatusMessage easily.
         * @return Pointer to WidgetFocusStatusMessage class if type of this message is WidgetFocusStatus, nullptr otherwise.
         */
        class WidgetFocusStatusMessage const* getAsWidgetFocusStatus() const;

        /**
         * @brief Default destructor of DcsmStatusMessage
         */
        virtual ~DcsmStatusMessage();

        /**
         * @brief Disable copying
         */
        DcsmStatusMessage(DcsmStatusMessage const&) = delete;

        /**
         * @brief Disable moving
         */
        DcsmStatusMessage(DcsmStatusMessage&&) = delete;

        /**
         * Stores internal data for implementation specifics of DcsmStatusMessage
         */
        std::unique_ptr<DcsmStatusMessageImpl> impl;

    protected:
        /**
         * @brief Constructor from impl
         * @param impl_ impl
         */
        explicit DcsmStatusMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_);
    };


    /**
     * @brief A message containing a stream status.
     */
    class RAMSES_API StreamStatusMessage : public DcsmStatusMessage
    {
    public:
        /**
         * @brief The states of the video decoder on the instrument cluster.
         */
        enum class Status
        {
            Ready = 0,              ///< video subsystem ready (ready to accept start command, this is provided in regard of system startup and error condition management).
            Enabled,                ///< stream enabled (channel is open, no video data available yet).
            Valid,                  ///< stream valid (valid video data has been forwarded to VSPD).
            Invalid,                ///< stream invalid (video stream does not provide valid video data).
            Halted,                 ///< stream halted (stream has been halted, channels closed).
            ChannelError,           ///< channel error (channel experienced error, no video).
            InvalidCommand,         ///< invalid command Error.
            VideoResolutionChanged  ///< video resolution changed.
        };

        /**
        * @brief Create a StreamStatusMessage object containing the provided status.
        * @param status The status to transmit.
        */
        explicit StreamStatusMessage(Status status);

        /**
         * @brief Returns the contained status.
         * @return status the contained status.
         */
        Status getStreamStatus() const;

        /**
         * @brief Constructor from impl, only for internal usage
         * @param impl_ impl
         */
        explicit StreamStatusMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_);
    };


    /**
     * @brief A status message containing the instrument cluster's active layout.
     */
    class RAMSES_API ActiveLayoutMessage : public DcsmStatusMessage
    {
    public:
        /**
         * @brief The possible layout values in the instrument cluster.
         */
        enum class Layout : uint32_t
        {
            Drive = 0,
            Focus,
            Gallery,
            Autonomous,
            Sport_Road,
            Sport_Track,
        };

        /**
        * @brief Create an ActiveLayoutMessage object containing the provided layout.
        * @param layout The layout to transmit.
        */
        explicit ActiveLayoutMessage(Layout layout);

        /**
         * @brief Returns the active layout
         * @return active layout
         */
        Layout getLayout() const;

        /**
         * @brief Constructor from impl, only for internal usage
         * @param impl_ impl
         */
        explicit ActiveLayoutMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_);
    };

    /**
     * @brief A message containing the widget focus state
     */
    class RAMSES_API WidgetFocusStatusMessage : public DcsmStatusMessage
    {
    public:
        /**
         * @brief The focus states of the widget on the instrument cluster.
         */
        enum class Status : uint32_t
        {
            Unknown = 0,
            Focused,               ///< widget is focused
            NotFocused,            ///< widget is not focused
        };

        /**
        * @brief Create a WidgetFocusStatusMessage object containing the provided status.
        * @param status The status to transmit.
        */
        explicit WidgetFocusStatusMessage(Status status);

        /**
         * @brief Returns the contained status.
         * @return status the contained status.
         */
        Status getWidgetFocusStatus() const;

        /**
         * @brief Constructor from impl, only for internal usage
         * @param impl_ impl
         */
        explicit WidgetFocusStatusMessage(std::unique_ptr<DcsmStatusMessageImpl>&& impl_);
    };
}

#if defined(_MSC_VER)
__pragma(warning(pop))
#endif
#endif
