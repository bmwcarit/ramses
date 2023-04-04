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
#include <vector>
#include <atomic>
#include <cstddef>

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

    template<typename DataType, std::size_t N>
    struct FirstNElementsSummaryEntry final
    {
        std::vector<DataType> array;

        void reset()
        {
            array.clear();
        }

        void update(DataType value)
        {
            if (array.size() < N)
            {
                array.push_back(value);
            }
        }

        [[nodiscard]] constexpr size_t maxSize() const noexcept
        {
            return N;
        }
    };

    template<typename DataType, template<typename> class SummaryTypeTemplate>
    struct StatisticEntry final
    {
        using ValueType   = DataType;
        using SummaryType = SummaryTypeTemplate<DataType>;

        StatisticEntry()
            : m_counter(0)
        {
        }

        void incCounter(DataType increment)
        {
            m_counter += increment;
        }

        void decCounter(DataType decrement)
        {
            m_counter -= decrement;
        }

        void setCounterValue(DataType newValue)
        {
            m_counter = newValue;
        }

        template<typename Cmp>
        void setCounterValueIfCurrent(DataType newValue)
        {
            if (Cmp{}(m_counter, newValue))
                m_counter = newValue;
        }

        [[nodiscard]] DataType getCounterValue() const
        {
            return m_counter.load();
        }

        void reset()
        {
            m_counter = 0;
            m_summary.reset();
        }

        void updateSummary()
        {
            m_summary.update(m_counter.load());
        }

        DataType updateSummaryAndResetCounter()
        {
            const DataType val = m_counter.exchange(0);
            m_summary.update(val);
            return val;
        }

        SummaryType& getSummary()
        {
            return m_summary;
        }

    private:
        std::atomic<DataType> m_counter;
        SummaryType           m_summary;
    };

    class StatisticCollection
    {
    public:
        StatisticCollection();

        virtual ~StatisticCollection()
        {}

        virtual void reset();
        virtual void resetSummaries();
        virtual void nextTimeInterval();

        [[nodiscard]] UInt32 getNumberTimeIntervalsSinceLastSummaryReset() const;

    protected:
        UInt32 m_numberTimeIntervalsSinceLastSummaryReset;
    };

    class StatisticCollectionFramework : public StatisticCollection
    {
    public:
        void reset() override;
        void resetSummaries() override;
        void nextTimeInterval() override;

        StatisticEntry<UInt32, SummaryEntry> statMessagesSent;
        StatisticEntry<UInt32, SummaryEntry> statMessagesReceived;
        StatisticEntry<UInt32, SummaryEntry> statResourcesCreated;
        StatisticEntry<UInt32, SummaryEntry> statResourcesDestroyed;
        StatisticEntry<UInt32, SummaryEntry> statResourcesNumber; //updated by values of statResourcesCreated and statResourcesDestroyed
        StatisticEntry<UInt32, SummaryEntry> statResourcesLoadedFromFileNumber;
        StatisticEntry<UInt32, SummaryEntry> statResourcesLoadedFromFileSize;
    };

    enum EResourceStatisticIndex : std::size_t // deliberately not enum class, supposed to be implicitly convertible
    {
        EResourceStatisticIndex_ArrayResource,
        EResourceStatisticIndex_Effect,
        EResourceStatisticIndex_Texture,

        EResourceStatisticIndex_NumIndices
    };

    template <typename DataType>
    using FirstFiveElements = FirstNElementsSummaryEntry<DataType, 5>;

    class StatisticCollectionScene : public StatisticCollection
    {
    public:
        void reset() override;
        void resetSummaries() override;
        void nextTimeInterval() override;

        StatisticEntry<UInt32, SummaryEntry> statFlushesTriggered;
        StatisticEntry<UInt32, SummaryEntry> statObjectsCreated;
        StatisticEntry<UInt32, SummaryEntry> statObjectsDestroyed;
        StatisticEntry<UInt32, SummaryEntry> statObjectsCount; //updated by values of statObjectsCreated and statObjectsDestroyed
        StatisticEntry<UInt32, SummaryEntry> statSceneActionsSent;
        StatisticEntry<UInt32, SummaryEntry> statSceneActionsSentSkipped;
        StatisticEntry<UInt32, SummaryEntry> statSceneActionsGenerated;
        StatisticEntry<UInt32, SummaryEntry> statSceneActionsGeneratedSize;
        StatisticEntry<UInt32, SummaryEntry> statSceneUpdatesGeneratedPackets;
        StatisticEntry<UInt32, SummaryEntry> statSceneUpdatesGeneratedSize;
        StatisticEntry<UInt64, FirstFiveElements> statMaximumSizeSingleSceneUpdate;

        std::array<StatisticEntry<UInt64, SummaryEntry>, EResourceStatisticIndex_NumIndices> statResourceCount;
        std::array<StatisticEntry<UInt64, SummaryEntry>, EResourceStatisticIndex_NumIndices> statResourceAvgSize;
        std::array<StatisticEntry<UInt64, SummaryEntry>, EResourceStatisticIndex_NumIndices> statResourceMaxSize;
    };
}

#endif

