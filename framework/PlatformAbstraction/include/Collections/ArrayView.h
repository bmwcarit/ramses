//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ARRAYVIEW_H
#define RAMSES_ARRAYVIEW_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <iterator>

namespace ramses_internal
{
    template<typename T>
    class ArrayView
    {
    public:
        ArrayView(T* data, UInt32 size)
            : m_data(data)
            , m_size(size)
        {
            assert(data != nullptr || size == 0);
        }

        template<typename I>
        class InternalIterator : public std::iterator<std::random_access_iterator_tag, I>
        {
        public:
            InternalIterator()
                : m_ptr(nullptr)
            {}

            InternalIterator(I* ptr)
                : m_ptr(ptr)
            {}

            inline I* get()
            {
                return m_ptr;
            }

            inline const I* get() const
            {
                return m_ptr;
            }

            inline I& operator*()
            {
                return *m_ptr;
            }

            inline const I& operator*() const
            {
                return *m_ptr;
            }

            inline InternalIterator& operator++()
            {
                m_ptr++;
                return *this;
            }

            inline const InternalIterator operator++(int)
            {
                InternalIterator ret(m_ptr);
                ++(*this);
                return ret;
            }

            inline InternalIterator& operator--()
            {
                m_ptr--;
                return *this;
            }

            inline bool operator==(const InternalIterator& rhs) const
            {
                return m_ptr == rhs.m_ptr;
            }

            inline bool operator!=(const InternalIterator& rhs) const
            {
                return !(*this == rhs);
            }

            inline bool operator<(const InternalIterator& rhs) const
            {
                return m_ptr < rhs.m_ptr;
            }

            inline bool operator<=(const InternalIterator& rhs) const
            {
                return m_ptr <= rhs.m_ptr;
            }

            inline bool operator>=(const InternalIterator& rhs) const
            {
                return m_ptr >= rhs.m_ptr;
            }

            inline bool operator>(const InternalIterator& rhs) const
            {
                return m_ptr > rhs.m_ptr;
            }

            inline std::ptrdiff_t operator-(const InternalIterator& rhs) const
            {
                return m_ptr - rhs.m_ptr;
            }

            inline InternalIterator operator-(const std::ptrdiff_t& offset) const
            {
                return InternalIterator(m_ptr - offset);
            }

            inline InternalIterator operator+(const std::ptrdiff_t& offset) const
            {
                return InternalIterator(m_ptr + offset);
            }

            inline InternalIterator& operator+=(const std::ptrdiff_t& offset)
            {
                m_ptr += offset;
                return *this;
            }

        private:
            I* m_ptr;
        };

        using Iterator = InternalIterator<T>;
        using ConstIterator = InternalIterator<const T>;

        inline Iterator begin()
        {
            return Iterator(m_data);
        }

        inline Iterator end()
        {
            return Iterator(m_data + m_size);
        }

        inline ConstIterator begin() const
        {
            return ConstIterator(m_data);
        }

        inline ConstIterator end() const
        {
            return ConstIterator(m_data + m_size);
        }

        inline UInt32 size() const
        {
            return m_size;
        }

        inline const T* data() const
        {
            return m_data;
        }

        inline T* data()
        {
            return m_data;
        }

    private:

        T*      m_data;
        UInt32  m_size;
    };

    using ByteArrayView = ArrayView<const Byte>;
}
#endif
