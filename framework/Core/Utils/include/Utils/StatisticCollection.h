//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STATISTICCOLLECTION_H
#define RAMSES_STATISTICCOLLECTION_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <limits>
#include <array>
#include <atomic>

namespace ramses_internal
{
    template<typename DataType>
    struct SummaryEntry final
    {
        SummaryEntry();

        DataType minValue;
        DataType maxValue;
        DataType sum;

        void reset();
        void update(DataType value);
    };

    template<typename DataType>
    ramses_internal::SummaryEntry<DataType>::SummaryEntry()
    {
        reset();
    }

    template<typename DataType>
    void SummaryEntry<DataType>::reset()
    {
        minValue = std::numeric_limits<DataType>::max();
        maxValue = std::numeric_limits<DataType>::min();
        sum = 0;
    }

    template<typename DataType>
    void SummaryEntry<DataType>::update(DataType value)
    {
        if (value < minValue)
        {
            minValue = value;
        }
        if (value > maxValue)
        {
            maxValue = value;
        }
        sum += value;
    }

    template<typename DataType>
    struct StatisticEntry final
    {
        StatisticEntry();

        void incCounter(DataType increment);
        void decCounter(DataType decrement);
        void setCounterValue(DataType newValue);
        DataType getCounterValue() const;
        void reset();
        void updateSummary();
        DataType updateSummaryAndResetCounter();
        SummaryEntry<DataType>& getSummary();

    private:
        std::atomic<DataType> m_counter;
        SummaryEntry<DataType> m_summary;
    };

    template<typename DataType>
    ramses_internal::StatisticEntry<DataType>::StatisticEntry()
        : m_counter(0)
    {
    }

    template<typename DataType>
    DataType ramses_internal::StatisticEntry<DataType>::getCounterValue() const
    {
        return m_counter.load();
    }

    template<typename DataType>
    void ramses_internal::StatisticEntry<DataType>::reset()
    {
        m_counter = 0;
        m_summary.reset();
    }


    template<typename DataType>
    void ramses_internal::StatisticEntry<DataType>::updateSummary()
    {
        m_summary.update(m_counter.load());
    }


    template<typename DataType>
    DataType StatisticEntry<DataType>::updateSummaryAndResetCounter()
    {
        const DataType val = m_counter.exchange(0);
        m_summary.update(val);
        return val;
    }

    template<typename DataType>
    void ramses_internal::StatisticEntry<DataType>::incCounter(DataType increment)
    {
        m_counter += increment;
    }

    template<typename DataType>
    void ramses_internal::StatisticEntry<DataType>::decCounter(DataType decrement)
    {
        m_counter -= decrement;
    }

    template<typename DataType>
    void ramses_internal::StatisticEntry<DataType>::setCounterValue(DataType newValue)
    {
        m_counter = newValue;
    }

    template<typename DataType>
    SummaryEntry<DataType>& ramses_internal::StatisticEntry<DataType>::getSummary()
    {
        return m_summary;
    }

    class StatisticCollection
    {
    public:
        StatisticCollection();

        virtual ~StatisticCollection()
        {}

        virtual void reset();
        virtual void resetSummaries();
        virtual void nextTimeInterval();

        UInt32 getNumberTimeIntervalsSinceLastSummaryReset() const;

    protected:
        UInt32 m_numberTimeIntervalsSinceLastSummaryReset;
    };

    class StatisticCollectionFramework : public StatisticCollection
    {
    public:
        virtual void reset() override;
        virtual void resetSummaries() override;
        virtual void nextTimeInterval() override;

        StatisticEntry<UInt32> statMessagesSent;
        StatisticEntry<UInt32> statMessagesReceived;
        StatisticEntry<UInt32> statResourcesCreated;
        StatisticEntry<UInt32> statResourcesDestroyed;
        StatisticEntry<UInt32> statResourcesNumber; //updated by values of statResourcesCreated and statResourcesDestroyed
        StatisticEntry<UInt32> statResourcesLoadedFromFileNumber;
        StatisticEntry<UInt32> statResourcesLoadedFromFileSize;
    };

    enum EResourceStatisticIndex : std::size_t // deliberately not enum class, supposed to be implicitly convertible
    {
        EResourceStatisticIndex_ArrayResource,
        EResourceStatisticIndex_Effect,
        EResourceStatisticIndex_Texture,

        EResourceStatisticIndex_NumIndices
    };

    class StatisticCollectionScene : public StatisticCollection
    {
    public:
        virtual void reset() override;
        virtual void resetSummaries() override;
        virtual void nextTimeInterval() override;

        StatisticEntry<UInt32> statFlushesTriggered;
        StatisticEntry<UInt32> statObjectsCreated;
        StatisticEntry<UInt32> statObjectsDestroyed;
        StatisticEntry<UInt32> statObjectsCount; //updated by values of statObjectsCreated and statObjectsDestroyed
        StatisticEntry<UInt32> statSceneActionsSent;
        StatisticEntry<UInt32> statSceneActionsSentSkipped;
        StatisticEntry<UInt32> statSceneActionsGenerated;
        StatisticEntry<UInt32> statSceneActionsGeneratedSize;
        StatisticEntry<UInt32> statSceneUpdatesGeneratedPackets;
        StatisticEntry<UInt32> statSceneUpdatesGeneratedSize;

        std::array<StatisticEntry<UInt64>, EResourceStatisticIndex_NumIndices> statResourceCount;
        std::array<StatisticEntry<UInt64>, EResourceStatisticIndex_NumIndices> statResourceAvgSize;
        std::array<StatisticEntry<UInt64>, EResourceStatisticIndex_NumIndices> statResourceMaxSize;
    };
}

#endif

