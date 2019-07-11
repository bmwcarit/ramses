//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ITHREADWATCHDOGNOTIFICATION_H
#define RAMSES_ITHREADWATCHDOGNOTIFICATION_H

#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
     * Specifies ramses threads
    */
    enum ERamsesThreadIdentifier
    {
        ERamsesThreadIdentifier_Unknown = 0,
        ERamsesThreadIdentifier_Workers,
        ERamsesThreadIdentifier_Renderer
    };

    /**
    * Users can implement this callback to receive alive notifications about ramses threads.
    *
    * In order to receive alive notification of ramses threads, one must provide a pointer to
    * an implementation of this interface to RamsesFrameworkConfig::setWatchdogNotificationCallBack
    * Before calling notifyThread with a certain threadID a registerThread call for that id will
    * be invoked. Before stopping notifications of a certain threadID the unregisterThread method will
    * be invoked -  therefore do not expect alive messages for a certain threadID after an unregisterThread
    * call for that threadID.
    */
    class RAMSES_API IThreadWatchdogNotification
    {
    public:
        /**
        * Virtual destructor of IThreadWatchdogNotification
        */
        virtual ~IThreadWatchdogNotification() = default;

        /**
        * This method is invoked cyclic with threadID of alive threads.
        *
        * The method is invoked from the actual thread context in question, therefore
        * users must take care not to call long operations in this callback, or
        * ramses processing will be blocked.
        *
        * @param[in] threadID ID of thread reporting it being alive.
        */
        virtual void notifyThread(ERamsesThreadIdentifier threadID) = 0;

        /**
        * Before reporting alive messages of a certain threadID, this method
        * will be called with that specific threadID. Do not expect notifyThread calls for
        * threadID before this method was invoked.
        *
        * @param[in] threadID ID of thread that will be reported.
        */
        virtual void registerThread(ERamsesThreadIdentifier threadID) = 0;

        /**
        * Before stopping to report alive messages of a certain threadID, this method
        * will be called with that specific threadID. Do not expect notifyThread calls for
        * threadID after this method was invoked.
        *
        * @param[in] threadID ID of thread that will not report anymore.
        */
        virtual void unregisterThread(ERamsesThreadIdentifier threadID) = 0;
    };
}

#endif
