/*
 * Copyright (C) 2013 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_VECTOR_H
#define RAMSES_CAPU_VECTOR_H

#include "ramses-capu/Error.h"
#include "ramses-capu/util/Traits.h"
#include "ramses-capu/os/Memory.h"
#include "ramses-capu/util/AlgorithmRaw.h"
#include "ramses-capu/util/Algorithm.h"
#include <iterator>
#include <new>
#include <initializer_list>
#include <assert.h>
#include <algorithm>
#include <type_traits>

namespace ramses_capu
{
    /**
     * Basic Vector implementation
     * The internal memory is doubled if the vector is full
     */
    template<typename T>
    class vector
    {
    private:

        /**
         * Iterator for Vector
         */
        template<typename TYPE>
        class InternalIterator : public std::iterator<std::random_access_iterator_tag, typename std::remove_const<TYPE>::type>
        {
        public:
            friend class vector<T>;

            typedef typename std::remove_const<TYPE>::type value_type;

            /**
             * Default constructor for the iterator
            */
            InternalIterator()
                : m_current(nullptr)
            {

            }

            /**
            * Copy constructor for the iterator
            */
            InternalIterator(const InternalIterator& other)
                : m_current(other.m_current)
            {

            }

            /**
             * Compares two Iterators if their internal position is not the same
             * @param other Iterator to compare with
             * @return true if Iterators don't point to the same data, false otherwise
             */
            bool operator!=(const InternalIterator<T>& other) const
            {
                return m_current != other.m_current;
            }

            /**
             * @copydoc operator!=(const InternalIterator<T>& other) const
             *
             **/
            bool operator!=(const InternalIterator<const T>& other) const
            {
                return m_current != other.m_current;
            }

            /**
            * Compares two Iterators if their internal position is the same
            * @param other Iterator to compare with
            * @return true if Iterators point to the same data, false otherwise
            */
            bool operator==(const InternalIterator<T>& other) const
            {
                return m_current == other.m_current;
            }

            /**
            * @copydoc operator==(const InternalIterator<T>& other) const
            *
            **/
            bool operator==(const InternalIterator<const T>& other) const
            {
                return m_current == other.m_current;
            }

            /**
             * Returns a reference to the element at the current position plus the offset
             * @param offset for the wanted position
             * @return reference to the element
             */
            TYPE& operator[](uint_t offset)
            {
                return *(m_current + offset);
            }

            /**
             * Dereferences the iterator to access the internal data
             * return a reference to the current data
             */
            TYPE& operator*()
            {
                return *m_current;
            }

            const TYPE& operator*() const
            {
                return *m_current;
            }

            /**
             * Allows access to methods and member of the internal data
             * @return the pointer to the current data
             */
            TYPE* operator->()
            {
                return &** this;
            }

            /**
            * @copydoc operator->
            */
            const TYPE* operator->() const
            {
                return &** this;
            }

            /**
             * Checks if the pointer of the other iterator is bigger than my own pointer
             * @param other InternalIterator to check with
             * @return true if other pointer is bigger than my one one, false otherwise
             */
            bool operator <(const InternalIterator<TYPE>& other)
            {
                return m_current < other.m_current;
            }

            /**
             * Checks if the pointer of the other iterator is smaller than my own pointer
             * @param other InternalIterator to check with
             * @return true if other pointer is smaller than my one one, false otherwise
             */
            bool operator >(const InternalIterator<TYPE>& other)
            {
                return m_current > other.m_current;
            }

            /**
            * Checks if the pointer of the other iterator is smaller or equal than my own pointer
            * @param other InternalIterator to check with
            * @return true if other pointer is smaller or equal than my one one, false otherwise
            */
            bool operator <=(const InternalIterator<TYPE>& other)
            {
                return m_current <= other.m_current;
            }

            /**
            * Checks if the pointer of the other iterator is bigger or equal than my own pointer
            * @param other InternalIterator to check with
            * @return true if other pointer is bigger or equal than my one one, false otherwise
            */
            bool operator >=(const InternalIterator<TYPE>& other)
            {
                return m_current >= other.m_current;
            }

            /**
             * Adds the given value to the internal pointer and returns a new iterator
             * @param value to add to the internal pointer
             * @return InternalIterator with the new pointer
             */
            InternalIterator operator+(const int_t value) const
            {
                return InternalIterator(m_current + value);
            }

            /**
            * Substracts the given value to the internal pointer and returns a new iterator
            * @param value to substract from the internal pointer
            * @return InternalIterator with the new pointer
            */
            InternalIterator operator-(const int_t value) const
            {
                return InternalIterator(m_current - value);
            }

            /**
            * Difference of the positions the iterators point
            * @param value to substract from this iterator
            * @return number of elements between iterators. may be negative
            */
            int_t operator-(const InternalIterator<TYPE>& other) const
            {
                return m_current - other.m_current;
            }

            /**
            * Sets the position to the current element plus the offset
            * @return Iterator to the new position
            */
            friend InternalIterator<TYPE> operator+(int_t offset, InternalIterator<TYPE>& other)
            {
                return other + offset;
            }

            /**
             * Sets the position to the current position plus the given offset
             * @param offset to add to the current position
             */
            InternalIterator<TYPE>& operator+=(const int_t offset)
            {
                m_current += offset;
                return *this;
            }

            /**
            * Sets the position to the current position minus the given offset
            * @param offset to substract from the current position
            */
            InternalIterator<TYPE>& operator-=(const int_t offset)
            {
                m_current -= offset;
                return *this;
            }


            /**
            * Sets the current position to the next element
            * @return InternalIterator pointing to the current position
            */
            InternalIterator<TYPE>& operator++()
            {
                ++m_current;
                return *this;
            }

            /**
            * Sets the current position to the next element and returns an iterator to the current element
            * @return InternalIterator to the element before the incrementation
            */
            InternalIterator<TYPE> operator++(int32_t)
            {
                InternalIterator<TYPE> result(*this);
                ++(*this);
                return result;
            }

            /**
            *  Sets the current position to the previous element
            */
            InternalIterator<TYPE>& operator--()
            {
                --m_current;
                return *this;
            }

            /**
            * Sets the current position to the previous element and returns an iterator to the current element
            * @return InternalIterator to the element before the decrement
            */
            InternalIterator<TYPE> operator--(int32_t)
            {
                InternalIterator<TYPE> result(*this);
                --(*this);
                return result;
            }

            /**
             * Assigns the position of the other iterator to my own position
             * @param other iterator to take position from
             * @return reference to myself
             */
            InternalIterator& operator=(const InternalIterator& other)
            {
                m_current = other.m_current;
                return *this;
            }

            /**
             * Swaps the internal pointer to the underlying data
             * @param iterator to swap with
             */
            void swap(InternalIterator& other)
            {
                using std::swap;
                swap(m_current, other.m_current);
            }

            /**
             * Swaps the internal pointers of the two given iterators
             * @param first iterator to swap with
             * @param second iterator to swap with
            */
            friend void swap(InternalIterator& first, InternalIterator& second)
            {
                first.swap(second);
            }

        protected:
        private:
            /**
             * Creates a Iterator for the Vector
             * @param start pointer for the iterator
             */
            explicit InternalIterator(TYPE* start)
                : m_current(start)
            {

            }

            /**
             * Pointer to the current data
             */
            TYPE* m_current;
        };

    public:

        /// Iterator
        typedef InternalIterator<T> iterator;
        /// constant iterator
        typedef InternalIterator<const T> const_iterator;

        /**
         * Iterator
         * @deprecated this form is deprecated
         */
        typedef InternalIterator<T> Iterator;
        /**
        * constant iterator
        * @deprecated this form is deprecated
        */
        typedef InternalIterator<const T> ConstIterator;

        /**
         * Creates a new vector. The initial capacity is 16
         */
        vector();

        /**
         * Creates a new vector with a given initial size
         * and initializes elements with default values
         * @param initialSize for the Vector
         */
        vector(const uint_t initialSize);

        /**
         * Initializes the vector with the given size and sets all elements
         * to the given value
         * @param initialSize for the Vector
         * @param value to set for all elements
         */
        vector(const uint_t initialSize, const T& value);

        /**
         * Initializes the vector from given initializer_list
         * @param init the initializer_list
         */
        vector(std::initializer_list<T> init);

        /**
         * Initializes the vector from another vector
         */
        vector(const vector& other);

        /**
         * Move initializes the vector from another vector
         */
        vector(vector&& other) noexcept;

        /**
         * Assignment operator for vector
         * @param Vector to copy from
         * @return reference to Vector with copied data
         */
        vector& operator=(const vector& other);

        /**
         * Move assign operator for vector
         * @param Vector to move from
         * @return reference to Vector with copied data
         */
        vector& operator=(vector&& other) noexcept;

        /**
         * Destructor
         */
        ~vector();

        /**
         * Adds an Element to the end of the vector
         * @param reference to the value to add
         */
        void push_back(const T& value);

        void push_back(T&& value);

        template <class... Args>
        void emplace_back(Args&&... args);

        /**
         * Removes the last element of the vector.
         * Returns CAPU_ERANGE if vector was empty.
         */
        void pop_back();

        /**
         * Returns the current size of the Vector
         * @return size of the current Vector
         */
        uint_t size() const;

        /**
         * Returns if the vector is empty.
         */
        bool empty() const;

        /**
         * Returns the first element.
         */
        T& front();

        /**
         * Returns the first element.
         */
        const T& front() const;

        /**
        * Returns the last element.
        */
        T& back();

        /**
        * Returns the last element.
        */
        const T& back() const;

        /**
        * Returns a pointer to the underlaying element storage.
        */
        T* data();
        const T* data() const;

        /**
         * Returns the current capacity of the vector.
         * The capacity is the total number of elements the vector can hold
         * without causing reallocation because the memory is already reserved.
         * The capacity can be influenced by use of the reserve method.
         */
        uint_t capacity() const;

        /**
         * Resizes the vector to the given size
         * If the new size is smaller than the old size
         * only the elements which fit into the new size are
         * available after resizing.
         * if new size is larger than the old size
         * the new elements are initialized with default values.
         * @param new size of the Vector
         */
        void resize(const uint_t size);

        /**
         * Reserves given capacity for the vector
         * Only has effect if new capacity is larger than the
         * old capacity.
         * @param new capacity of the Vector
         */
        void reserve(const uint_t capacity);

        /**
         * Reduces the capacity to size
         */
        void shrink_to_fit();

        /**
         * Removes all elements from the Vector
         */
        void clear();

        /**
         * Operator to access internal data with index
         * @param index of the element to access
         * @return reference to the element
         */
        T& operator[](const uint_t index);

        /**
        * Operator to access internal data with index
        * @param index of the element to access
        * @return constant reference to the element
        */
        const T& operator[](const uint_t index) const;

        /**
         * Compares the content of two Vectors
         * @param other Vector to compare with
         * @return true if both vectors are identical
         */
        bool operator==(const vector<T>& other) const;

        /**
         * Returns a new Iterator to the start of the Vector
         * @return a new Iterator to the start of the Vector
         * @{
         */
        Iterator begin();
        ConstIterator begin() const;
        /**
         * @}
         */

        /**
         * Returns a new ConstIterator to the start of the Vector
         * @return a new ConstIterator to the start of the Vector
         * @{
         */
        ConstIterator cbegin() const;

        /**
         * Returns a new Iterator to the end of the Vector
         * @return a new Iterator to the end of the Vector
         * @{
         */
        Iterator end();
        ConstIterator end() const;
        /**
        * @}
        */

        /**
         * Returns a new ConstIterator to the end of the Vector
         * @return a new ConstIterator to the end of the Vector
         * @{
         */
        ConstIterator cend() const;

         /**
         * remove the element in the specified index and if the element_old
         * parameter is not nullptr, the removed element will be put to element_old
         *
         * NOTE: Not STL compatible
         *
         * @param index index of element that will be removed
         * @param elementOld the buffer which will keep the copy of the removed element
         * @return CAPU_EINVAL invalid index
         *         CAPU_OK if the element is successfully removed
         */
        status_t erase(const uint_t index, T* elementOld = 0);

        /**
         * Removes the element in the specified iterator position and if the element_old
         * parameter is not nullptr, the removed element will be put to element_old
         * @param iterator iterator of element that will be removed
         * @param elementOld the buffer which will keep the copy of the removed element
         * @return CAPU_EINVAL invalid iterator
         *         CAPU_OK if the element is successfully removed
         *
         */
        status_t erase(const Iterator& iterator, T* elementOld = 0);

        /**
         * Inserts the given value at the specified iterator position
         * @param iterator iterator where value is to be inserted
         * @param value value to insert
         **/
        status_t insert(const Iterator& it, const T& value);

        /**
        * Inserts the given range of value at the specified iterator position
        * @param iterator iterator where value is to be inserted
        * @param first Iterator pointing to beginning of range to insert from
        * @param last Iterator pointing after last value to insert
        **/
        template <typename InputIt>
        status_t insert(const Iterator& it, InputIt first, InputIt last);

        /**
         * Exchange the content of this vector with other
         * @param other the other vector
         */
        void swap(vector<T>& other);

    protected:
    private:
        /**
         * Internal data to store the elements
         */
        T* m_data;

        /**
         * Iterator which points one after the end of the data
         */
        T* m_dataEnd;

        /**
         * Iterator which points to one after the end of the capacity
         */
        T* m_capacityEnd;

        /**
         * Internal method to double the current memory
         */
        void grow(uint_t requiredCapacity);
    };


    /**
     * swap overload for Vector<T>
     * @param first first vector
     * @param second vector to swap with first
     */
    template <typename T>
    inline
    void swap(vector<T>& first, vector<T>& second)
    {
        first.swap(second);
    }

    template<typename T>
    T& ramses_capu::vector<T>::front()
    {
        return *m_data;
    }

    template<typename T>
    const T& ramses_capu::vector<T>::front() const
    {
        return *m_data;
    }

    template<typename T>
    T& ramses_capu::vector<T>::back()
    {
        return *(m_dataEnd -1);
    }

    template<typename T>
    const T& ramses_capu::vector<T>::back() const
    {
        return *(m_dataEnd - 1);
    }

    template<typename T>
    T*  ramses_capu::vector<T>::data()
    {
        return m_data;
    }

    template<typename T>
    const T*  ramses_capu::vector<T>::data() const
    {
        return m_data;
    }

    template<typename T>
    status_t ramses_capu::vector<T>::insert(const Iterator& it, const T& value)
    {
        const uint_t numberFromBeginning = (it.m_current - m_data);
        if (m_dataEnd == m_capacityEnd)
        {
            /*status = */
            // todo: should check status of grow here
            grow(size() + 1);
        }

        // move back
        new(m_dataEnd)T();
        copy_backward(m_data + numberFromBeginning, m_dataEnd, m_dataEnd + 1u);
        ++m_dataEnd;

        m_data[numberFromBeginning] = value;
        return CAPU_OK;
    }

    template <typename T>
    template <typename InputIt>
    status_t ramses_capu::vector<T>::insert(const Iterator& it, InputIt first, InputIt last)
    {
        typedef typename std::iterator_traits<InputIt>::difference_type distanceType;
        const distanceType numberOfNewElements = std::distance(first, last);
        const uint_t insertOffset = (it.m_current - m_data);
        grow(size() + numberOfNewElements);

        const distanceType numberOfExistingElementsThatNeedToBeMoved = size() - insertOffset;
        if (numberOfExistingElementsThatNeedToBeMoved > numberOfNewElements)
        {
            // existing elements move must be split
            copy_to_raw(m_dataEnd - numberOfNewElements, m_dataEnd, m_dataEnd);
            const uint_t numberOfExistingElementsToCopyDirectly = numberOfExistingElementsThatNeedToBeMoved - numberOfNewElements;
            ramses_capu::copy_backward(m_data + insertOffset, m_data + insertOffset + numberOfExistingElementsToCopyDirectly, m_dataEnd);
            ramses_capu::copy(first, last, m_data + insertOffset);
        }
        else
        {
            // insert new elements must be split
            copy_to_raw(m_data + insertOffset, m_dataEnd, m_dataEnd + numberOfNewElements - numberOfExistingElementsThatNeedToBeMoved);

            const distanceType numberOfElementsToInsertDirectly = numberOfExistingElementsThatNeedToBeMoved;
            InputIt insertDirectlyEnd = first;
            std::advance(insertDirectlyEnd, numberOfElementsToInsertDirectly);

            ramses_capu::copy(first, insertDirectlyEnd, m_data + insertOffset);
            ramses_capu::copy_to_raw(insertDirectlyEnd, last, m_dataEnd);
        }
        m_dataEnd += numberOfNewElements;

        return CAPU_OK;
    }

    template<typename T>
    inline
    bool ramses_capu::vector<T>::empty() const
    {
        return m_data == m_dataEnd;
    }

    template<typename T>
    inline
    ramses_capu::vector<T>::vector(const vector& other)
        : m_data(other.capacity() == 0u ? nullptr : reinterpret_cast<T*>(new Byte[sizeof(T) * other.capacity()]))
        , m_dataEnd( m_data + other.size())
        , m_capacityEnd( m_data + other.capacity())
    {
        copy_to_raw(other.m_data, other.m_dataEnd, m_data);
    }

    template<typename T>
    inline
    vector<T>::vector(vector&& other) noexcept
        : m_data(other.m_data)
        , m_dataEnd(other.m_dataEnd)
        , m_capacityEnd(other.m_capacityEnd)
    {
        other.m_data = nullptr;
        other.m_dataEnd = nullptr;
        other.m_capacityEnd = nullptr;
    }

    template<typename T>
    inline
    vector<T>::vector()
        : m_data(0)
        , m_dataEnd(0)
        , m_capacityEnd(0)
    {
    }

    template<typename T>
    inline
    vector<T>::vector(const uint_t initialSize, const T& value)
        : m_data(initialSize == 0u ? nullptr : reinterpret_cast<T*>(new Byte[sizeof(T) * initialSize]))
        , m_dataEnd(m_data + initialSize)
        , m_capacityEnd(m_data + initialSize)
    {
        fill_n_raw(m_data, initialSize, value);
    }


    template<typename T>
    inline
    vector<T>::vector(std::initializer_list<T> init)
        : vector()
    {
        insert(end(), std::begin(init), std::end(init));
    }

    template<typename T>
    inline
    vector<T>::vector(const uint_t initialSize)
        : m_data(initialSize == 0u ? nullptr : reinterpret_cast<T*>(new Byte[sizeof(T) * initialSize]))
        , m_dataEnd(m_data + initialSize)
        , m_capacityEnd(m_data + initialSize)
    {
        fill_n_raw(m_data, initialSize);
    }

    template<typename T>
    inline
    vector<T>&
    vector<T>::operator=(const vector<T>& other)
    {
        if (this != &other)
        {
            clear();
            const uint_t numberOfElementsInOther = other.size();
            reserve(numberOfElementsInOther);

            copy_to_raw(other.m_data, other.m_dataEnd, m_data);
            m_dataEnd = m_data + numberOfElementsInOther;
        }
        return *this;
    }

    template<typename T>
    inline
    vector<T>&
    vector<T>::operator=(vector&& other) noexcept
    {
        vector<T> tmp(std::move(other));
        this->swap(tmp);
        return *this;
    }

    template<typename T>
    inline vector<T>::~vector()
    {
        clear();
        const Byte* untypedMemory = reinterpret_cast<Byte*>(m_data);
        delete[] untypedMemory;
    }

    template<typename T>
    inline void vector<T>::push_back(const T& value)
    {
        if (m_dataEnd == m_capacityEnd)
        {
            grow(size() + 1);
        }

        T* thing = reinterpret_cast<T*>(m_dataEnd);
        new(thing)T(value);

        ++m_dataEnd;
    }

    template <typename T>
    inline void vector<T>::push_back(T&& value)
    {
        if (m_dataEnd == m_capacityEnd)
        {
            grow(size() + 1);
        }

        T* thing = reinterpret_cast<T*>(m_dataEnd);
        new(thing)T(std::move(value));

        ++m_dataEnd;
    }

    template <typename T>
    template <class... Args>
    inline void vector<T>::emplace_back(Args&&... args)
    {
        if (m_dataEnd == m_capacityEnd)
        {
            grow(size() + 1);
        }

        T* thing = reinterpret_cast<T*>(m_dataEnd);
        new(thing)T(std::forward<Args>(args)...);

        ++m_dataEnd;
    }

    template<typename T>
    inline void vector<T>::pop_back()
    {
        assert(m_dataEnd > m_data);
        (m_dataEnd - 1)->~T();
        --m_dataEnd;
    }

    template<typename T>
    inline
    void
    vector<T>::clear()
    {
        destruct_raw(m_data, m_dataEnd);
        m_dataEnd = m_data;
    }

    template<typename T>
    inline
    void
    vector<T>::grow(uint_t requiredCapacity)
    {
        const uint_t currentCapacity = capacity();
        if (requiredCapacity > currentCapacity)
        {
            // try double current capacity: exponential growth
            uint_t newCapacity = currentCapacity * 2;

            // if not enough, use requiredCapacity directly
            if (newCapacity < requiredCapacity)
            {
                newCapacity = requiredCapacity;
            }

            reserve(newCapacity);
        }
    }

    template<typename T>
    inline
    void
    vector<T>::resize(const uint_t newSize)
    {
        const uint_t previousNumberOfElements = size();
        if (newSize < previousNumberOfElements)
        {
            // must delete excess elements
            destruct_raw(m_data + newSize, m_data + previousNumberOfElements);
        }
        else
        {
            if (newSize > capacity())
            {
                // new size does not fit, must grow first
                grow(newSize);
            }
            // initialize new objects
            const uint_t numberOfNewObjects = newSize - previousNumberOfElements;
            fill_n_raw(m_data + previousNumberOfElements, numberOfNewObjects);

        }
        m_dataEnd = m_data + newSize;
    }

    template<typename T>
    inline
    void
    vector<T>::reserve(const uint_t newSize)
    {
        if (newSize > capacity())
        {
            if (newSize > std::numeric_limits<uint_t>::max() / 2)
            {
                abort();
            }
            else
            {
                const uint_t currentNumberOfElements = size();
                void* newMemory = new Byte[sizeof(T) * newSize];
                T* newTypedMemory = reinterpret_cast<T*>(newMemory);

                move_to_raw(m_data, m_dataEnd, newTypedMemory);
                destruct_raw(m_data, m_dataEnd);

                // delete previous memory
                Byte* untypedMemory = reinterpret_cast<Byte*>(m_data);
                delete[] untypedMemory;
                m_data = newTypedMemory;
                m_dataEnd = m_data + currentNumberOfElements;
                m_capacityEnd = m_data + newSize;
            }
        }
    }

    template<typename T>
    inline
    void
    vector<T>::shrink_to_fit()
    {
        vector<T> tmp(0);
        tmp.insert(tmp.begin(), this->begin(), this->end());
        this->swap(tmp);
    }

    template<typename T>
    inline
    T&
    vector<T>::operator[](const uint_t index)
    {
        return *(m_data + index);
    }

    template<typename T>
    inline
    const T&
    vector<T>::operator[](const uint_t index) const
    {
        return *(m_data + index);
    }

    template<typename T>
    inline
    uint_t
    vector<T>::size() const
    {
        const uint_t numberOfElements = (m_dataEnd - m_data);
        return numberOfElements;
    }

    template<typename T>
    inline
    uint_t vector<T>::capacity() const
    {
        return (m_capacityEnd - m_data);
    }

    template <typename T>
    inline
    typename vector<T>::Iterator vector<T>::begin()
    {
        return Iterator(m_data);
    }

    template<typename T>
    inline
    typename vector<T>::ConstIterator
    vector<T>::begin() const
    {
        return ConstIterator(m_data);
    }

    template<typename T>
    inline
    typename vector<T>::ConstIterator
    vector<T>::cbegin() const
    {
        return ConstIterator(m_data);
    }

    template<typename T>
    inline
    typename vector<T>::Iterator
    vector<T>::end()
    {
        return Iterator(m_dataEnd);
    }

    template<typename T>
    inline
    typename vector<T>::ConstIterator
    vector<T>::end() const
    {
        return ConstIterator(m_dataEnd);
    }

    template<typename T>
    inline
    typename vector<T>::ConstIterator
    vector<T>::cend() const
    {
        return ConstIterator(m_dataEnd);
    }

    template<typename T>
    inline
    status_t vector<T>::erase(const uint_t index, T* elementOld)
    {
        const Iterator current = Iterator(m_data + index);
        return erase(current, elementOld);
    }

    template<typename T>
    inline
    status_t vector<T>::erase(const Iterator& it, T* elementOld)
    {
        if(it.m_current >= m_dataEnd)
        {
            return CAPU_EINVAL;
        }

        if(0 != elementOld)
        {
            *elementOld = *it;
        }

        ramses_capu::copy(it + 1u, Iterator(m_dataEnd), it);
        resize(size() - 1);

        return CAPU_OK;
    }

    template<typename T>
    inline
    bool
    vector<T>::operator==(const vector<T>& other) const
    {
        if (size() != other.size())
        {
            return false;
        }
        return equal(m_data, m_dataEnd, other.m_data);
    }

    template<typename T>
    inline
    void vector<T>::swap(vector<T>& other)
    {
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_dataEnd, other.m_dataEnd);
        swap(m_capacityEnd, other.m_capacityEnd);
    }
}

#endif // RAMSES_CAPU_VECTOR_H
