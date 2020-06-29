//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYOUTPUTSTREAM_H
#define RAMSES_BINARYOUTPUTSTREAM_H

#include "Collections/IOutputStream.h"
#include <vector>

namespace ramses_internal
{
    template <typename T>
    class BinaryOutputStreamT: public IOutputStream
    {
    public:
        static_assert(sizeof(T) == 1, "only works with byte sized types");

        explicit BinaryOutputStreamT(size_t startSize = 16);

        IOutputStream& write(const void* data, const uint32_t size) override;

        const T* getData() const;
        size_t getSize() const;
        size_t getCapacity() const;

        std::vector<T> release();

    private:
        std::vector<T> m_buffer;
    };

    using BinaryOutputStream = BinaryOutputStreamT<char>;

    template <typename T>
    inline BinaryOutputStreamT<T>::BinaryOutputStreamT(size_t startSize)
        : m_buffer()
    {
        m_buffer.reserve(startSize);
    }

    template <typename T>
    inline IOutputStream& BinaryOutputStreamT<T>::write(const void* data, const uint32_t size)
    {
        const T* dataCharptr = static_cast<const T*>(data);
        m_buffer.insert(m_buffer.end(), dataCharptr, dataCharptr+size);
        return *this;
    }

    template <typename T>
    inline const T* BinaryOutputStreamT<T>::getData() const
    {
        return m_buffer.data();
    }

    template <typename T>
    inline size_t BinaryOutputStreamT<T>::getSize() const
    {
        return static_cast<uint32_t>(m_buffer.size());
    }

    template <typename T>
    inline size_t BinaryOutputStreamT<T>::getCapacity() const
    {
        return static_cast<uint32_t>(m_buffer.capacity());
    }

    template <typename T>
    inline std::vector<T> BinaryOutputStreamT<T>::release()
    {
        std::vector<T> result;
        result.swap(m_buffer);
        return result;
    }
}

#endif
