//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/ESceneActionId.h"
#include "internal/Core/Common/TypedMemoryHandle.h"
#include "internal/Core/Common/StronglyTypedValue.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/PlatformAbstraction/Collections/Guid.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "internal/PlatformAbstraction/Macros.h"
#include "internal/Core/Utils/AssertMovable.h"
#include "glm/gtx/range.hpp"

#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <string>
#include <string_view>

namespace ramses::internal
{
    class SceneActionCollection
    {
    public:
        SceneActionCollection();
        SceneActionCollection(size_t initialDataCapacity, size_t initialNumberOfSceneActionsInformationCapacity);

        SceneActionCollection(const SceneActionCollection&) = delete;
        SceneActionCollection& operator=(const SceneActionCollection&) = delete;

        SceneActionCollection(SceneActionCollection&&) noexcept = default;
        SceneActionCollection& operator=(SceneActionCollection&&) noexcept = default;

        [[nodiscard]] SceneActionCollection copy() const;

        void clear();
        [[nodiscard]] bool empty() const;
        void reserveAdditionalCapacity(size_t additionalDataCapacity, size_t additionalSceneActionsInformationCapacity);

        void append(const SceneActionCollection& other);

        bool operator==(const SceneActionCollection& rhs) const;
        bool operator!=(const SceneActionCollection& rhs) const;

        void swap(SceneActionCollection& second);
        friend void swap(SceneActionCollection& first, SceneActionCollection& second);

        // writing
        void beginWriteSceneAction(ESceneActionId type);

        // concrete types
        void write(std::string_view str);
        void write(const Guid& guid);
        void write(const ResourceContentHash& hash);
        template <typename T>
        void write(TypedMemoryHandle<T> handle);
        template <typename T, T D, typename U>
        void write(StronglyTypedValue<T, D, U> value);
        template <int L, typename T, glm::qualifier Q>
        void write(const glm::vec<L, T, Q>& value);
        template <int C, int R, typename T, glm::qualifier Q>
        void write(const glm::mat<C, R, T, Q>& value);
        template <typename E, typename = typename std::enable_if<std::is_enum<E>::value>::type>
        void write(E enumValue);
        // fixed size array
        template<typename T, int N, typename = typename std::enable_if_t<std::is_arithmetic_v<T>||std::is_same_v<T, std::byte>>>
        void write(const T(&data)[N]); // NOLINT(modernize-avoid-c-arrays)
        // generic
        template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>||std::is_same_v<T, std::byte>>>
        void write(const T& value);
        // generic array
        template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>||std::is_same_v<T, std::byte>>>
        void write(const T* data, uint32_t numElements);

        static const uint8_t MaxStringLength = std::numeric_limits<uint8_t>::max();

        // blob write access
        void appendRawData(const std::byte* data, size_t dataSize);
        std::vector<std::byte>& getRawDataForDirectWriting();
        void addRawSceneActionInformation(ESceneActionId type, uint32_t offset);

        // blob read access
        [[nodiscard]] const std::vector<std::byte>& collectionData() const;

        // reading
        class Iterator;
        class SceneActionReader;

        [[nodiscard]] uint32_t numberOfActions() const;

        [[nodiscard]] Iterator begin() const;
        [[nodiscard]] Iterator end() const;

        [[nodiscard]] SceneActionReader front() const;
        [[nodiscard]] SceneActionReader back() const;
        SceneActionReader operator[](size_t actionIndex) const;

        // read classes
        class SceneActionReader
        {
        public:
            [[nodiscard]] ESceneActionId type() const;
            [[nodiscard]] uint32_t size() const;
            [[nodiscard]] const std::byte* data() const;
            [[nodiscard]] uint32_t offsetInCollection() const;

            // concrete types
            void read(std::string& str);
            void read(Guid& guid);
            void read(ResourceContentHash& hash);
            template <typename T>
            void read(TypedMemoryHandle<T>& handle);
            template <typename T, T D, typename U>
            void read(StronglyTypedValue<T, D, U>& value);
            template<int L, typename T, glm::qualifier Q>
            void read(glm::vec<L, T, Q>& value);
            template<int C, int R, typename T, glm::qualifier Q>
            void read(glm::mat<C, R, T, Q>& value);
            template<typename E, typename std::enable_if<std::is_enum<E>::value, int>::type = 0>
            void read(E& value);
            // fixed size array
            template<typename T, int N, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
            void read(T(&data)[N]); // NOLINT(modernize-avoid-c-arrays)
            // generic
            template<typename T, typename std::enable_if<std::is_arithmetic<T>::value && !std::is_enum<T>::value, int>::type = 0>
            void read(T& value);
            // generic array
            template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
            void read(T* data, uint32_t& numElements);
            // get pointer to written array of bytes and increment reader position
            void readWithoutCopy(const std::byte*& data, uint32_t& size);

            [[nodiscard]] bool isFullyRead() const;

        private:
            friend SceneActionCollection;
            friend Iterator;

            SceneActionReader(const SceneActionCollection* collection, size_t actionIndex);

            template<typename T, typename = typename std::enable_if<std::is_trivial<T>::value>::type>
            void readFromByteBlob(T& value);
            void readFromByteBlob(void* data, size_t size);

            [[nodiscard]] uint32_t offsetForIndex(size_t idx) const;

            const SceneActionCollection* m_collection;
            size_t m_actionIndex;
            size_t m_readPosition;
        };

        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = SceneActionReader;
            using difference_type = std::ptrdiff_t;
            using pointer = SceneActionReader*;
            using reference = SceneActionReader&;

            Iterator();

            bool operator!=(const Iterator& other) const;
            bool operator==(const Iterator& other) const;

            Iterator& operator++();
            Iterator operator+(int32_t add);

            SceneActionReader& operator*();
            const SceneActionReader& operator*() const;

            SceneActionReader* operator->();
            const SceneActionReader* operator->() const;
        private:
            friend class SceneActionCollection;
            explicit Iterator(const SceneActionReader& reader);

            SceneActionReader m_reader;
        };

    private:
        friend class SceneActionReader;

        struct ActionInfo
        {
            ESceneActionId type;
            uint32_t offset;

            bool operator!=(const ActionInfo& rhs) const;
            bool operator==(const ActionInfo& rhs) const;
        };

        template<typename T, typename = typename std::enable_if<std::is_trivial<T>::value>::type>
        void writeAsByteBlob(const T& value);
        void writeAsByteBlob(const void* value, size_t size);
        void reserveAdditionalDataCapacity(size_t additionalCapacity);

        std::vector<std::byte> m_data;
        std::vector<ActionInfo> m_actionInfo;
    };

    ASSERT_MOVABLE(SceneActionCollection)

    inline SceneActionCollection::SceneActionCollection() = default;

    inline SceneActionCollection::SceneActionCollection(size_t initialDataCapacity, size_t initialNumberOfSceneActionsInformationCapacity)
    {
        m_data.reserve(initialDataCapacity);
        m_actionInfo.reserve(initialNumberOfSceneActionsInformationCapacity);
    }

    inline SceneActionCollection SceneActionCollection::copy() const
    {
        SceneActionCollection ret;
        ret.m_data = m_data;
        ret.m_actionInfo = m_actionInfo;
        return ret;
    }

    inline void SceneActionCollection::reserveAdditionalDataCapacity(size_t additionalCapacity)
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

    inline void SceneActionCollection::reserveAdditionalCapacity(size_t additionalDataCapacity, size_t additionalSceneActionsInformationCapacity)
    {
        reserveAdditionalDataCapacity(additionalDataCapacity);
        m_actionInfo.reserve(m_actionInfo.size() + additionalSceneActionsInformationCapacity);
    }

    inline void SceneActionCollection::append(const SceneActionCollection& other)
    {
        const bool hasIncomplete = !m_actionInfo.empty() &&
            !other.m_actionInfo.empty() &&
            m_actionInfo.back().type == ESceneActionId::Incomplete;

        if (hasIncomplete)
        {
            // append to incomplete
            // - merge incomplete last info with first other (offset from this, type from other)
            // - offsets start at last info offset
            m_actionInfo.reserve(m_actionInfo.size() + other.m_actionInfo.size() - 1);
            m_actionInfo.back().type = other.m_actionInfo.front().type;

            // skip first because was already merged
            const uint32_t offsetBase = m_actionInfo.back().offset;
            for (auto it = other.m_actionInfo.begin() + 1; it != other.m_actionInfo.end(); ++it)
            {
                m_actionInfo.push_back({ it->type, it->offset + offsetBase});
            }
        }
        else
        {
            // normal append
            const auto offsetBase = static_cast<uint32_t>(m_data.size());
            m_actionInfo.reserve(m_actionInfo.size() + other.m_actionInfo.size());
            for (const auto info : other.m_actionInfo)
            {
                m_actionInfo.push_back({ info.type, info.offset + offsetBase });
            }
        }
        m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
    }

    inline bool SceneActionCollection::operator==(const SceneActionCollection& rhs) const
    {
        return m_data == rhs.m_data && m_actionInfo == rhs.m_actionInfo;
    }

    inline bool SceneActionCollection::operator!=(const SceneActionCollection& rhs) const
    {
        return m_data != rhs.m_data || m_actionInfo != rhs.m_actionInfo;
    }

    inline void SceneActionCollection::swap(SceneActionCollection& second)
    {
        using std::swap;
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
        m_actionInfo.push_back(ActionInfo{ type, static_cast<uint32_t>(m_data.size()) });
    }

    inline void SceneActionCollection::write(std::string_view str)
    {
        // check for MaxStringLength
        const auto truncatedLength = static_cast<uint8_t>(std::min(static_cast<size_t>(MaxStringLength), str.size()));

        reserveAdditionalDataCapacity(sizeof(uint8_t) + truncatedLength);
        writeAsByteBlob(truncatedLength);
        writeAsByteBlob(str.data(), truncatedLength);
    }

    inline void SceneActionCollection::write(const Guid& guid)
    {
        writeAsByteBlob(guid.get());
    }

    inline void SceneActionCollection::write(const ResourceContentHash& hash)
    {
        writeAsByteBlob(&hash, sizeof(hash));
    }

    template <typename T>
    void SceneActionCollection::write(TypedMemoryHandle<T> handle)
    {
        writeAsByteBlob(handle.asMemoryHandle());
    }

    template <typename T, T D, typename U>
    void SceneActionCollection::write(StronglyTypedValue<T, D, U> value)
    {
        writeAsByteBlob(value.getValue());
    }

    template <int L, typename T, glm::qualifier Q>
    void SceneActionCollection::write(const glm::vec<L, T, Q>& value)
    {
        std::for_each(glm::begin(value), glm::end(value), [&](auto& val) { write(val); });
    }

    template <int C, int R, typename T, glm::qualifier Q>
    void SceneActionCollection::write(const glm::mat<C, R, T, Q>& value)
    {
        std::for_each(glm::begin(value), glm::end(value), [&](auto& val) { write(val); });
    }

    template <typename E, typename>
    void SceneActionCollection::write(E enumValue)
    {
        static_assert(!std::is_convertible<E, int>::value, "Use enum class! If need to keep old enum, cast explicitly to integral type.");
        writeAsByteBlob(enumValue);
    }

    template<typename T, int N, typename>
    void SceneActionCollection::write(const T(&data)[N])  // NOLINT(modernize-avoid-c-arrays)
    {
        writeAsByteBlob(data, sizeof(data));
    }

    template<typename T, typename>
    void SceneActionCollection::write(const T* data, uint32_t numElements)
    {
        const uint32_t byteSize = numElements * sizeof(T);
        reserveAdditionalDataCapacity(sizeof(uint32_t) + byteSize);
        writeAsByteBlob(numElements);
        writeAsByteBlob(static_cast<const void*>(data), byteSize);
    }

    template<typename T, typename>
    void SceneActionCollection::write(const T& value)
    {
        writeAsByteBlob(value);
    }

    template<typename T, typename>
    void SceneActionCollection::writeAsByteBlob(const T& value)
    {
        writeAsByteBlob(&value, sizeof(T));
    }

    inline void SceneActionCollection::writeAsByteBlob(const void* value, size_t size)
    {
        const auto* valuePtr = static_cast<const std::byte*>(value);
        m_data.insert(m_data.end(), valuePtr, valuePtr + size);
    }

    // blob write access
    inline void SceneActionCollection::appendRawData(const std::byte* data, size_t dataSize)
    {
        writeAsByteBlob(data, dataSize);
    }

    inline std::vector<std::byte>& SceneActionCollection::getRawDataForDirectWriting()
    {
        return m_data;
    }

    inline void SceneActionCollection::addRawSceneActionInformation(ESceneActionId type, uint32_t offset)
    {
        m_actionInfo.push_back({ type, offset });
    }

    // blob read access
    inline const std::vector<std::byte>& SceneActionCollection::collectionData() const
    {
        return m_data;
    }

    // reading
    inline uint32_t SceneActionCollection::numberOfActions() const
    {
        return static_cast<uint32_t>(m_actionInfo.size());
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
        assert(!m_actionInfo.empty());
        return SceneActionCollection::SceneActionReader(this, 0);
    }

    inline SceneActionCollection::SceneActionReader SceneActionCollection::back() const
    {
        assert(!m_actionInfo.empty());
        return SceneActionCollection::SceneActionReader(this, m_actionInfo.size() - 1);
    }

    inline SceneActionCollection::SceneActionReader SceneActionCollection::operator[](size_t actionIndex) const
    {
        assert(actionIndex < m_actionInfo.size());
        return SceneActionCollection::SceneActionReader(this, actionIndex);
    }

    // class Reader
    inline SceneActionCollection::SceneActionReader::SceneActionReader(const SceneActionCollection* collection, size_t actionIndex)
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

    inline uint32_t SceneActionCollection::SceneActionReader::offsetInCollection() const
    {
        assert(m_collection && m_actionIndex < m_collection->m_actionInfo.size());
        return m_collection->m_actionInfo[m_actionIndex].offset;
    }

    inline uint32_t SceneActionCollection::SceneActionReader::size() const
    {
        assert(m_collection && m_actionIndex < m_collection->m_actionInfo.size());
        return offsetForIndex(m_actionIndex + 1) - m_collection->m_actionInfo[m_actionIndex].offset;
    }

    inline const std::byte* SceneActionCollection::SceneActionReader::data() const
    {
        assert(m_collection && m_actionIndex < m_collection->m_actionInfo.size());
        return m_collection->m_data.data() + m_collection->m_actionInfo[m_actionIndex].offset;
    }

    inline void SceneActionCollection::SceneActionReader::read(std::string& str)
    {
        // read string length
        uint8_t length = 0;
        readFromByteBlob(length);

        if (length > 0)
        {
            // read string
            str.resize(length);
            readFromByteBlob(str.data(), length);
        }
        else
        {
            str.clear();
        }
    }

    inline void SceneActionCollection::SceneActionReader::read(ResourceContentHash& hash)
    {
        readFromByteBlob(&hash, sizeof(hash));
    }

    template<typename T, typename>
    void SceneActionCollection::SceneActionReader::read(T* data, uint32_t& numElements)
    {
        // read array size
        readFromByteBlob(numElements);
        const uint32_t byteSize = numElements * sizeof(T);
        PlatformMemory::Copy(data, m_collection->m_data.data() + m_readPosition, byteSize);
        m_readPosition += byteSize;
    }

    inline void SceneActionCollection::SceneActionReader::readWithoutCopy(const std::byte*& data, uint32_t& size)
    {
        // read array size
        readFromByteBlob(size);
        if (size > 0u)
        {
            data = m_collection->m_data.data() + m_readPosition;
            m_readPosition += size;
        }
        else
            data = nullptr;
    }

    inline void SceneActionCollection::SceneActionReader::read(Guid& guid)
    {
        uint64_t data = 0u;
        readFromByteBlob(data);
        guid = Guid(data);
    }

    template<typename E, typename std::enable_if<std::is_enum<E>::value, int>::type>
    void SceneActionCollection::SceneActionReader::read(E& value)
    {
        static_assert(!std::is_convertible<E, int>::value, "Use enum class! If need to keep old enum, cast explicitly to integral type.");
        readFromByteBlob(value);
    }

    template <typename T>
    void SceneActionCollection::SceneActionReader::read(TypedMemoryHandle<T>& handle)
    {
        readFromByteBlob(handle.asMemoryHandleReference());
    }

    template <typename T, T D, typename U>
    void SceneActionCollection::SceneActionReader::read(StronglyTypedValue<T, D, U>& value)
    {
        readFromByteBlob(value.getReference());
    }

    template<int L, typename T, glm::qualifier Q>
    void SceneActionCollection::SceneActionReader::read(glm::vec<L, T, Q>& value)
    {
        std::for_each(glm::begin(value), glm::end(value), [&](auto& val) { read(val); });
    }

    template<int C, int R, typename T, glm::qualifier Q>
    void SceneActionCollection::SceneActionReader::read(glm::mat<C, R, T, Q>& value)
    {
        std::for_each(glm::begin(value), glm::end(value), [&](auto& val) { read(val); });
    }

    template<typename T, int N, typename>
    void SceneActionCollection::SceneActionReader::read(T(&data)[N])  // NOLINT(modernize-avoid-c-arrays)
    {
        readFromByteBlob(data, sizeof(data));
    }

    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value && !std::is_enum<T>::value, int>::type>
    void SceneActionCollection::SceneActionReader::read(T& value)
    {
        readFromByteBlob(value);
    }

    template<typename T, typename>
    void SceneActionCollection::SceneActionReader::readFromByteBlob(T& value)
    {
        readFromByteBlob(&value, sizeof(value));
    }

    inline void SceneActionCollection::SceneActionReader::readFromByteBlob(void* data, size_t size)
    {
        PlatformMemory::Copy(static_cast<std::byte*>(data), m_collection->m_data.data() + m_readPosition, size);
        m_readPosition += size;
    }

    inline bool SceneActionCollection::SceneActionReader::isFullyRead() const
    {
        return m_readPosition == offsetForIndex(m_actionIndex + 1);
    }

    inline uint32_t SceneActionCollection::SceneActionReader::offsetForIndex(size_t idx) const
    {
        return (idx >= m_collection->m_actionInfo.size()) ? // is > last index?
            static_cast<uint32_t>(m_collection->m_data.size()) : m_collection->m_actionInfo[idx].offset;
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

    inline SceneActionCollection::Iterator SceneActionCollection::Iterator::operator+(int32_t add)
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
