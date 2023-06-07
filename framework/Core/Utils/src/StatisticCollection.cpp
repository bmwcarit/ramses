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

    uint32_t StatisticCollection::getNumberTimeIntervalsSinceLastSummaryReset() const
    {
        return m_numberTimeIntervalsSinceLastSummaryReset;
    }

    void StatisticCollectionFramework::reset()
    {
        StatisticCollection::reset();

        statMessagesSent.reset();
        statMessagesReceived.reset();
        statResourcesCreated.reset();
        statResourcesDestroyed.reset();
        statResourcesNumber.reset();
        statResourcesLoadedFromFileNumber.reset();
        statResourcesLoadedFromFileSize.reset();
    }

    void StatisticCollectionFramework::resetSummaries()
    {
        StatisticCollection::resetSummaries();

        statMessagesSent.getSummary().reset();
        statMessagesReceived.getSummary().reset();
        statResourcesCreated.getSummary().reset();
        statResourcesDestroyed.getSummary().reset();
        statResourcesNumber.getSummary().reset();
        statResourcesLoadedFromFileNumber.getSummary().reset();
        statResourcesLoadedFromFileSize.getSummary().reset();
    }

    void StatisticCollectionFramework::nextTimeInterval()
    {
        StatisticCollection::nextTimeInterval();

        statMessagesSent.updateSummaryAndResetCounter();
        statMessagesReceived.updateSummaryAndResetCounter();
        const uint32_t resourcesCreated = statResourcesCreated.updateSummaryAndResetCounter();
        const uint32_t resourcesDestroyed = statResourcesDestroyed.updateSummaryAndResetCounter();
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
        statObjectsCount.reset();
        statSceneActionsSent.reset();
        statSceneActionsSentSkipped.reset();
        statSceneActionsGenerated.reset();
        statSceneActionsGeneratedSize.reset();
        statSceneUpdatesGeneratedPackets.reset();
        statSceneUpdatesGeneratedSize.reset();
        statMaximumSizeSingleSceneUpdate.reset();

        for (size_t type = 0; type < EResourceStatisticIndex_NumIndices; type++)
        {
            statResourceCount[type].reset();
            statResourceAvgSize[type].reset();
            statResourceMaxSize[type].reset();
        }
    }

    void StatisticCollectionScene::resetSummaries()
    {
        StatisticCollection::resetSummaries();

        statFlushesTriggered.getSummary().reset();
        statObjectsCreated.getSummary().reset();
        statObjectsDestroyed.getSummary().reset();
        statObjectsCount.getSummary().reset();
        statSceneActionsSent.getSummary().reset();
        statSceneActionsSentSkipped.getSummary().reset();
        statSceneActionsGenerated.getSummary().reset();
        statSceneActionsGeneratedSize.getSummary().reset();
        statSceneUpdatesGeneratedPackets.getSummary().reset();
        statSceneUpdatesGeneratedSize.getSummary().reset();
        statMaximumSizeSingleSceneUpdate.getSummary().reset();
        for (size_t type = 0; type < EResourceStatisticIndex_NumIndices; type++)
        {
            statResourceCount[type].getSummary().reset();
            statResourceAvgSize[type].getSummary().reset();
            statResourceMaxSize[type].getSummary().reset();

        }
    }

    void StatisticCollectionScene::nextTimeInterval()
    {
        StatisticCollection::nextTimeInterval();

        statFlushesTriggered.updateSummaryAndResetCounter();
        const uint32_t objectsCreated = statObjectsCreated.updateSummaryAndResetCounter();
        const uint32_t objectsDestroyed = statObjectsDestroyed.updateSummaryAndResetCounter();

        statSceneActionsSent.updateSummaryAndResetCounter();
        statSceneActionsSentSkipped.updateSummaryAndResetCounter();
        statSceneActionsGenerated.updateSummaryAndResetCounter();
        statSceneActionsGeneratedSize.updateSummaryAndResetCounter();
        statSceneUpdatesGeneratedPackets.updateSummaryAndResetCounter();
        statSceneUpdatesGeneratedSize.updateSummaryAndResetCounter();

        statMaximumSizeSingleSceneUpdate.updateSummaryAndResetCounter();

        statObjectsCount.incCounter(objectsCreated);
        statObjectsCount.decCounter(objectsDestroyed);
        statObjectsCount.updateSummary();

        for (size_t type = 0; type < EResourceStatisticIndex_NumIndices; type++)
        {
            statResourceCount[type].updateSummaryAndResetCounter();
            statResourceAvgSize[type].updateSummaryAndResetCounter();
            statResourceMaxSize[type].updateSummaryAndResetCounter();
        }
    }
}
