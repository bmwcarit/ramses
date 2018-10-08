//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEACTIONCOLLECTION_H
#define RAMSES_SCENEACTIONCOLLECTION_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "TransportCommon/ESceneActionId.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/Guid.h"
#include "Collections/String.h"
#include "Collections/Vector.h"
#include <algorithm>
#include <iterator>

namespace ramses_internal
{
    class SceneActionCollection
    {
    public:
        SceneActionCollection();
        SceneActionCollection(UInt initialDataCapacity, UInt initialNumberOfSceneActionsInformationCapacity);

        SceneActionCollection(const SceneActionCollection&) = delete;
        SceneActionCollection& operator=(const SceneActionCollection&) = delete;

        SceneActionCollection(SceneActionCollection&&) = default;
        SceneActionCollection& operator=(SceneActionCollection&&) = default;

        SceneActionCollection copy() const;

        void clear();
        bool empty() const;
        void reserveAdditionalCapacity(UInt additionalDataCapacity, UInt additionalSceneActionsInformationCapacity);

        void append(const SceneActionCollection& other);

        Bool operator==(const SceneActionCollection& rhs) const;
        Bool operator!=(const SceneActionCollection& rhs) const;

        void swap(SceneActionCollection& second);
        friend void swap(SceneActionCollection& first, SceneActionCollection& second);

        // writing
        void beginWriteSceneAction(ESceneActionId type);

        void write(const String& str);
        void write(const Byte* data, UInt32 size);
        void write(const generic_uuid_t& guid);
        void write(const Guid& guid);

        template<typename T>
        void write(const T& value);

        static const UInt32 MaxStringLength = 255;

        // blob write access
        void appendRawData(const Byte* data, UInt dataSize);
        Vector<Byte>& getRawDataForDirectWriting();
        void addRawSceneActionInformation(ESceneActionId type, UInt32 offset);

        // blob read access
        const Vector<Byte>& collectionData() const;

        // reading
        class Iterator;
        class SceneActionReader;

        UInt32 numberOfActions() const;

        Iterator begin() const;
        Iterator end() const;

        SceneActionReader front() const;
        SceneActionReader back() const;
        SceneActionReader operator[](UInt actionIndex) const;

        // read classes
        class SceneActionReader
        {
        public:
            ESceneActionId type() const;
            UInt32 size() const;
            const Byte* data() const;
            UInt32 offsetInCollection() const;

            void read(String& str);
            void read(const Byte*& data, UInt32& size);
            void read(generic_uuid_t& guid);
            void read(Guid& guid);

            template<typename T>
            void read(T& value);

            bool isFullyRead() const;

        private:
            friend SceneActionCollection;
            friend Iterator;

            SceneActionReader(const SceneActionCollection* collection, UInt actionIndex);

            template<typename T>
            void readPritimiteValue(T& value);
            UInt32 offsetForIndex(UInt idx) const;

            const SceneActionCollection* m_collection;
            UInt m_actionIndex;
            UInt m_readPosition;
        };

        class Iterator : public std::iterator<std::forward_iterator_tag, SceneActionReader, UInt, SceneActionReader*, SceneActionReader&>
        {
        public:
            Iterator();

            bool operator!=(const Iterator& other) const;
            bool operator==(const Iterator& other) const;

            Iterator& operator++();
            Iterator operator+(Int32 add);

            SceneActionReader& operator*();
            const SceneActionReader& operator*() const;

            SceneActionReader* operator->();
            const SceneActionReader* operator->() const;
        private:
            friend class SceneActionCollection;
            Iterator(const SceneActionReader& reader);

            SceneActionReader m_reader;
        };

    private:
        friend class SceneActionReader;

        struct ActionInfo
        {
            ESceneActionId type;
            UInt32 offset;

            bool operator!=(const ActionInfo&) const;
            bool operator==(const ActionInfo&) const;
        };

        template<typename T>
        void writePritimiteValue(const T& value);
        void reserveAdditionalDataCapacity(UInt additionalCapacity);

        Vector<Byte> m_data;
        Vector<ActionInfo> m_actionInfo;
    };

    static_assert(std::is_nothrow_move_constructible<SceneActionCollection>::value &&
                  std::is_nothrow_move_assignable<SceneActionCollection>::value, "SceneActionCollection must be movable");

    inline SceneActionCollection::SceneActionCollection()
    {
    }

    inline SceneActionCollection::SceneActionCollection(UInt initialCapacity, UInt initialNumberOfSceneActionsInformationCapacity)
    {
        m_data.reserve(initialCapacity);
        m_actionInfo.reserve(initialNumberOfSceneActionsInformationCapacity);
    }

    inline SceneActionCollection SceneActionCollection::copy() const
    {
        SceneActionCollection ret;
        ret.m_data = m_data;
        ret.m_actionInfo = m_actionInfo;
        return ret;
    }

    inline void SceneActionCollection::reserveAdditionalDataCapacity(UInt additionalCapacity)
    {
        m_data.reserve(m_data.size() + additionalCapacity);
    }

    inline void SceneActionCollection::clear()
    {
        m_actionInfo.clear();
        m_data.clear();
    }

    inline bool SceneActionCollection::empty() const
    {
        return m_actionInfo.empty();
    }

    inline void SceneActionCollection::reserveAdditionalCapacity(UInt additionalDataCapacity, UInt additionalSceneActionsInformationCapacity)
    {
        reserveAdditionalDataCapacity(additionalDataCapacity);
        m_actionInfo.reserve(m_actionInfo.size() + additionalSceneActionsInformationCapacity);
    }

    inline void SceneActionCollection::append(const SceneActionCollection& other)
    {
        const bool hasIncomplete = !m_actionInfo.empty() &&
            !other.m_actionInfo.empty() &&
            m_actionInfo.back().type == ESceneActionId_Incomplete;

        if (hasIncomplete)
        {
            // append to incomplete
            // - merge incomplete last info with first other (offset from this, type from other)
            // - offsets start at last info offset
            m_actionInfo.reserve(m_actionInfo.size() + other.m_actionInfo.size() - 1);
            m_actionInfo.back().type = other.m_actionInfo.front().type;

            // skip first because was already merged
            const UInt32 offsetBase = m_actionInfo.back().offset;
            for (auto it = other.m_actionInfo.begin() + 1; it != other.m_actionInfo.end(); ++it)
            {
                m_actionInfo.push_back({ it->type, it->offset + offsetBase});
            }
        }
        else
        {
            // normal append
            const UInt32 offsetBase = static_cast<UInt32>(m_data.size());
            m_actionInfo.reserve(m_actionInfo.size() + other.m_actionInfo.size());
            for (const auto info : other.m_actionInfo)
            {
                m_actionInfo.push_back({ info.type, info.offset + offsetBase });
            }
        }
        m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
    }

    inline Bool SceneActionCollection::operator==(const SceneActionCollection& rhs) const
    {
        return m_data == rhs.m_data && m_actionInfo == rhs.m_actionInfo;
    }

    inline Bool SceneActionCollection::operator!=(const SceneActionCollection& rhs) const
    {
        return m_data != rhs.m_data || m_actionInfo != rhs.m_actionInfo;
    }

    inline void SceneActionCollection::swap(SceneActionCollection& second)
    {
        using ramses_capu::swap;
        swap(m_data, second.m_data);
        swap(m_actionInfo, second.m_actionInfo);
    }

    inline void swap(SceneActionCollection& first, SceneActionCollection& second)
    {
        first.swap(second);
    }

    // writing
    inline void SceneActionCollection::beginWriteSceneAction(ESceneActionId type)
    {
        m_actionInfo.push_back(ActionInfo{ type, static_cast<UInt32>(m_data.size()) });
    }

    inline void SceneActionCollection::write(const String& str)
    {
        // check for MaxStringLength
        const UInt32 truncatedLength = std::min(MaxStringLength, static_cast<UInt32>(str.getLength()));

        // resize if necessary
        reserveAdditionalDataCapacity(sizeof(UInt32) + truncatedLength);

        writePritimiteValue(static_cast<UInt32>(truncatedLength));

        const Byte* strPtr = reinterpret_cast<const Byte*>(str.c_str());
        m_data.insert(m_data.end(), strPtr, strPtr + truncatedLength);
    }

    inline void SceneActionCollection::write(const Byte* data, UInt32 size)
    {
        reserveAdditionalDataCapacity(sizeof(UInt32) + size);
        writePritimiteValue(size);
        m_data.insert(m_data.end(), data, data + size);
    }

    inline void SceneActionCollection::write(const generic_uuid_t& guid)
    {
        writePritimiteValue(guid);
    }

    inline void SceneActionCollection::write(const Guid& guid)
    {
        writePritimiteValue(guid.getGuidData());
    }

    template<typename T>
    inline void SceneActionCollection::write(const T& value)
    {
        writePritimiteValue(value);
    }

    template<typename T>
    inline void SceneActionCollection::writePritimiteValue(const T& value)
    {
        const Byte* valuePtr = reinterpret_cast<const Byte*>(&value);
        m_data.insert(m_data.end(), valuePtr, valuePtr + sizeof(T));
    }

    // blob write access
    inline void SceneActionCollection::appendRawData(const Byte* data, UInt dataSize)
    {
        m_data.insert(m_data.end(), data, data + dataSize);
    }

    inline Vector<Byte>& SceneActionCollection::getRawDataForDirectWriting()
    {
        return m_data;
    }

    inline void SceneActionCollection::addRawSceneActionInformation(ESceneActionId type, UInt32 offset)
    {
        m_actionInfo.push_back({ type, offset });
    }

    // blob read access
    inline const Vector<Byte>& SceneActionCollection::collectionData() const
    {
        return m_data;
    }

    // reading
    inline UInt32 SceneActionCollection::numberOfActions() const
    {
        return static_cast<UInt32>(m_actionInfo.size());
    }

    inline SceneActionCollection::Iterator SceneActionCollection::begin() const
    {
        return Iterator(SceneActionReader(this, 0));
    }

    inline SceneActionCollection::Iterator SceneActionCollection::end() const
    {
        return Iterator(SceneActionReader(this, m_actionInfo.size()));
    }

    inline SceneActionCollection::SceneActionReader SceneActionCollection::front() const
    {
        assert(m_actionInfo.size() > 0);
        return SceneActionCollection::SceneActionReader(this, 0);
    }

    inline SceneActionCollection::SceneActionReader SceneActionCollection::back() const
    {
        assert(m_actionInfo.size() > 0);
        return SceneActionCollection::SceneActionReader(this, m_actionInfo.size() - 1);
    }

    inline SceneActionCollection::SceneActionReader SceneActionCollection::operator[](UInt actionIndex) const
    {
        assert(actionIndex < m_actionInfo.size());
        return SceneActionCollection::SceneActionReader(this, actionIndex);
    }

    // class Reader
    inline SceneActionCollection::SceneActionReader::SceneActionReader(const SceneActionCollection* collection, UInt actionIndex)
        : m_collection(collection)
        , m_actionIndex(actionIndex)
        , m_readPosition(offsetForIndex(actionIndex))
    {
    }

    inline ESceneActionId SceneActionCollection::SceneActionReader::type() const
    {
        assert(m_collection && m_actionIndex < m_collection->m_actionInfo.size());
        return m_collection->m_actionInfo[m_actionIndex].type;
    }

    inline UInt32 SceneActionCollection::SceneActionReader::offsetInCollection() const
    {
        assert(m_collection && m_actionIndex < m_collection->m_actionInfo.size());
        return m_collection->m_actionInfo[m_actionIndex].offset;
    }

    inline UInt32 SceneActionCollection::SceneActionReader::size() const
    {
        assert(m_collection && m_actionIndex < m_collection->m_actionInfo.size());
        return offsetForIndex(m_actionIndex + 1) - m_collection->m_actionInfo[m_actionIndex].offset;
    }

    inline const Byte* SceneActionCollection::SceneActionReader::data() const
    {
        assert(m_collection && m_actionIndex < m_collection->m_actionInfo.size());
        return m_collection->m_data.data() + m_collection->m_actionInfo[m_actionIndex].offset;
    }

    inline void SceneActionCollection::SceneActionReader::read(String& str)
    {
        // read string length
        UInt32 length = 0;
        readPritimiteValue(length);

        if (length > 0)
        {
            // read string
            String tmp(length, 0);
            PlatformMemory::Copy(static_cast<void*>(&tmp[0u]), m_collection->m_data.data() + m_readPosition, length);
            str.swap(tmp);
            m_readPosition += length;
        }
        else
        {
            str = String();
        }
    }

    inline void SceneActionCollection::SceneActionReader::read(const Byte*& data, UInt32& size)
    {
        // read array size
        UInt32 arraySize = 0;
        readPritimiteValue(arraySize);

        if (arraySize > 0)
        {
            size = arraySize;
            data = m_collection->m_data.data() + m_readPosition;
            m_readPosition += arraySize;
        }
        else
        {
            size = 0;
            data = NULL;
        }
    }

    inline void SceneActionCollection::SceneActionReader::read(generic_uuid_t& guid)
    {
        readPritimiteValue(guid);
    }

    inline void SceneActionCollection::SceneActionReader::read(Guid& guid)
    {
        generic_uuid_t data;
        readPritimiteValue(data);
        guid = Guid(data);
    }

    template<typename T>
    inline void SceneActionCollection::SceneActionReader::read(T& value)
    {
        readPritimiteValue(value);
    }

    template<typename T>
    inline void SceneActionCollection::SceneActionReader::readPritimiteValue(T& value)
    {
        PlatformMemory::Copy(reinterpret_cast<Byte*>(&value), m_collection->m_data.data() + m_readPosition, sizeof(T));
        m_readPosition += sizeof(T);
    }

    inline bool SceneActionCollection::SceneActionReader::isFullyRead() const
    {
        return m_readPosition == offsetForIndex(m_actionIndex + 1);
    }

    inline UInt32 SceneActionCollection::SceneActionReader::offsetForIndex(UInt idx) const
    {
        return (idx >= m_collection->m_actionInfo.size()) ? // is > last index?
            static_cast<UInt32>(m_collection->m_data.size()) : m_collection->m_actionInfo[idx].offset;
    }

    // class Iterator
    inline SceneActionCollection::Iterator::Iterator()
        : m_reader(nullptr, 0)
    {
    }

    inline SceneActionCollection::Iterator::Iterator(const SceneActionCollection::SceneActionReader& reader)
        : m_reader(reader)
    {
    }

    inline bool SceneActionCollection::Iterator::operator!=(const Iterator& other) const
    {
        return m_reader.m_collection != other.m_reader.m_collection ||
            m_reader.m_actionIndex != other.m_reader.m_actionIndex;
    }

    inline bool SceneActionCollection::Iterator::operator==(const Iterator& other) const
    {
        return !operator!=(other);
    }

    inline SceneActionCollection::Iterator& SceneActionCollection::Iterator::operator++()
    {
        ++m_reader.m_actionIndex;
        m_reader.m_readPosition = m_reader.offsetForIndex(m_reader.m_actionIndex);
        return *this;
    }

    inline SceneActionCollection::Iterator SceneActionCollection::Iterator::operator+(Int32 add)
    {
        return Iterator(SceneActionReader(m_reader.m_collection, m_reader.m_actionIndex + add));
    }

    inline SceneActionCollection::SceneActionReader& SceneActionCollection::Iterator::operator*()
    {
        return m_reader;
    }

    inline const SceneActionCollection::SceneActionReader& SceneActionCollection::Iterator::operator*() const
    {
        return m_reader;
    }

    inline SceneActionCollection::SceneActionReader* SceneActionCollection::Iterator::operator->()
    {
        return &m_reader;
    }

    inline const SceneActionCollection::SceneActionReader* SceneActionCollection::Iterator::operator->() const
    {
        return &m_reader;
    }

    inline bool SceneActionCollection::ActionInfo::operator!=(const ActionInfo& rhs) const
    {
        return type != rhs.type || offset != rhs.offset;
    }

    inline bool SceneActionCollection::ActionInfo::operator==(const ActionInfo& rhs) const
    {
        return !(*this != rhs);
    }
}

#endif
