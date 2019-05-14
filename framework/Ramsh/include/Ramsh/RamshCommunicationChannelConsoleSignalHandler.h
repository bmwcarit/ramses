//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLESIGNALHANDLER_H
#define RAMSES_RAMSHCOMMUNICATIONCHANNELCONSOLESIGNALHANDLER_H

#include "Collections/Vector.h"

namespace ramses_internal
{

    class RamshCommunicationChannelConsole;

    /// Class, that handles signals to restore the settings of RamshCommunicationChannelConsole instances before exitting.
    /** Stopping the threads of the consoles is done, to ensure that the original console settings are restored
     *  before the application terminates. Necessary on some platforms, otherwise input on the
     *  console is not displayed anymore after interrupting a RAMSES application by STRG-C, e.g. */
    class RamshCommunicationChannelConsoleSignalHandler
    {
    public:
        /// Inserts a RamshCommunicationChannelConsole instance to the handler.
        /** @param console The console instance. */
        void insert(RamshCommunicationChannelConsole* console);

        /// Removes a RamshCommunicationChannelConsole instance from the handler.
        /** @param console The console instance. */
        void remove(RamshCommunicationChannelConsole* console);

        /// Handler for signal. Stops the RamshCommunicationChannelConsole threads.
        /** @param sig The signal. */
        void handleSignal(int sig);

        /// Returns the singleton object.
        /** @return The singleton object. */
        static RamshCommunicationChannelConsoleSignalHandler& getInstance();

    protected:
        /// Constructor.
        RamshCommunicationChannelConsoleSignalHandler();

        /// List of registered RamshCommunicationChannelConsole instances.
        std::vector<RamshCommunicationChannelConsole*> m_consoles;
    };
}

#endif
