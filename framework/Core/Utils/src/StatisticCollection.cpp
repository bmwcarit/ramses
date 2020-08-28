//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/StatisticCollection.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{

    StatisticCollection::StatisticCollection()
        : m_numberTimeIntervalsSinceLastSummaryReset(0)
    {
    }

    void StatisticCollection::reset()
    {
        m_numberTimeIntervalsSinceLastSummaryReset = 0;
    }

    void StatisticCollection::resetSummaries()
    {
        m_numberTimeIntervalsSinceLastSummaryReset = 0;
    }

    void StatisticCollection::nextTimeInterval()
    {
        m_numberTimeIntervalsSinceLastSummaryReset++;
    }

    ramses_internal::UInt32 StatisticCollection::getNumberTimeIntervalsSinceLastSummaryReset() const
    {
        return m_numberTimeIntervalsSinceLastSummaryReset;
    }

    void StatisticCollectionFramework::reset()
    {
        StatisticCollection::reset();

        statMessagesSent.reset();
        statMessagesReceived.reset();
        statSendResourceMessagesReceived.reset();
        statRequestResourceMessagesReceived.reset();
        statResourcesCreated.reset();
        statResourcesDestroyed.reset();
        statResourcesNumber.reset();
        statResourcesSentNumber.reset();
        statResourcesSentSize.reset();
        statResourcesReceivedNumber.reset();
        statResourcesLoadedFromFileNumber.reset();
        statResourcesLoadedFromFileSize.reset();
    }

    void StatisticCollectionFramework::resetSummaries()
    {
        StatisticCollection::resetSummaries();

        statMessagesSent.getSummary().reset();
        statMessagesReceived.getSummary().reset();
        statSendResourceMessagesReceived.getSummary().reset();
        statRequestResourceMessagesReceived.getSummary().reset();
        statResourcesCreated.getSummary().reset();
        statResourcesDestroyed.getSummary().reset();
        statResourcesNumber.getSummary().reset();
        statResourcesSentNumber.getSummary().reset();
        statResourcesSentSize.getSummary().reset();
        statResourcesReceivedNumber.getSummary().reset();
        statResourcesLoadedFromFileNumber.getSummary().reset();
        statResourcesLoadedFromFileSize.getSummary().reset();
    }

    void StatisticCollectionFramework::nextTimeInterval()
    {
        StatisticCollection::nextTimeInterval();

        statMessagesSent.updateSummaryAndResetCounter();
        statMessagesReceived.updateSummaryAndResetCounter();
        statSendResourceMessagesReceived.updateSummaryAndResetCounter();
        statRequestResourceMessagesReceived.updateSummaryAndResetCounter();
        const UInt32 resourcesCreated = statResourcesCreated.updateSummaryAndResetCounter();
        const UInt32 resourcesDestroyed = statResourcesDestroyed.updateSummaryAndResetCounter();
        statResourcesSentNumber.updateSummaryAndResetCounter();
        statResourcesSentSize.updateSummaryAndResetCounter();
        statResourcesReceivedNumber.updateSummaryAndResetCounter();
        statResourcesLoadedFromFileNumber.updateSummaryAndResetCounter();
        statResourcesLoadedFromFileSize.updateSummaryAndResetCounter();

        statResourcesNumber.incCounter(resourcesCreated);
        statResourcesNumber.decCounter(resourcesDestroyed);
        statResourcesNumber.updateSummary();
    }

    void StatisticCollectionScene::reset()
    {
        StatisticCollection::reset();

        statFlushesTriggered.reset();
        statObjectsCreated.reset();
        statObjectsDestroyed.reset();
        statObjectsNumber.reset();
        statResourceObjectsCreated.reset();
        statResourceObjectsDestroyed.reset();
        statResourceObjectsNumber.reset();
        statSceneActionsSent.reset();
        statSceneActionsSentSkipped.reset();
        statSceneActionsGenerated.reset();
        statSceneActionsGeneratedSize.reset();
    }

    void StatisticCollectionScene::resetSummaries()
    {
        StatisticCollection::resetSummaries();

        statFlushesTriggered.getSummary().reset();
        statObjectsCreated.getSummary().reset();
        statObjectsDestroyed.getSummary().reset();
        statObjectsNumber.getSummary().reset();
        statResourceObjectsCreated.getSummary().reset();
        statResourceObjectsDestroyed.getSummary().reset();
        statResourceObjectsNumber.getSummary().reset();
        statSceneActionsSent.getSummary().reset();
        statSceneActionsSentSkipped.getSummary().reset();
        statSceneActionsGenerated.getSummary().reset();
        statSceneActionsGeneratedSize.getSummary().reset();
    }

    void StatisticCollectionScene::nextTimeInterval()
    {
        StatisticCollection::nextTimeInterval();

        statFlushesTriggered.updateSummaryAndResetCounter();
        const UInt32 objectsCreated = statObjectsCreated.updateSummaryAndResetCounter();
        const UInt32 objectsDestroyed = statObjectsDestroyed.updateSummaryAndResetCounter();
        const UInt32 resourcesCreated = statResourceObjectsCreated.updateSummaryAndResetCounter();
        const UInt32 resourcesDestroyed = statResourceObjectsDestroyed.updateSummaryAndResetCounter();

        statSceneActionsSent.updateSummaryAndResetCounter();
        statSceneActionsSentSkipped.updateSummaryAndResetCounter();
        statSceneActionsGenerated.updateSummaryAndResetCounter();
        statSceneActionsGeneratedSize.updateSummaryAndResetCounter();

        statObjectsNumber.incCounter(objectsCreated);
        statObjectsNumber.decCounter(objectsDestroyed);
        statObjectsNumber.updateSummary();
        statResourceObjectsNumber.incCounter(resourcesCreated);
        statResourceObjectsNumber.decCounter(resourcesDestroyed);
        statResourceObjectsNumber.updateSummary();
    }
}
