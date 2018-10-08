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


#ifndef RAMSES_CAPU_DELEGATE_H
#define RAMSES_CAPU_DELEGATE_H

#include "ramses-capu/Config.h"

namespace ramses_capu
{
    struct EmptyParameter
    {

    };

    template<typename ReturnType = void, typename Param1=EmptyParameter, typename Param2=EmptyParameter>
    class Delegate;

    /**
     * A delegate implementation to call a functions, static member functions and non static member functions
     */
    template<typename ReturnType, typename Param1, typename Param2>
    class Delegate
    {
    public:

        Delegate()
            : m_objPtr(0)
            , m_methodPtr(0)
        {
        }

        /**
         * Creates a delegate to a member function for the given type
         * @tparam T type of the class for the member function
         * @tparam TMethod pointer to the member function of the class T
         * @param instance to call the method on
         * @return the created Delegate
         */
        template<typename T, ReturnType (T::*TMethod)(Param1, Param2)>
        static Delegate Create(T& instance)
        {
            return CreateDelegate(&instance, &MethodCall<T, TMethod>);
        }

        /**
         * Creates a delegate to a function or a static member function
         * @tparam TMethod pointer to function to call
         * @return the created Delegate
         */
        template<ReturnType(*TMethod)(Param1, Param2)>
        static Delegate Create()
        {
            return CreateDelegate(0, &FunctionCall<TMethod>);
        }

        /**
         * Invokes the call on the delegate
         * @param value1 parameter to pass to the function
         * @param value2 parameter to pass to the function
         * @return the return value of the delegated function
         */
        ReturnType operator()(Param1 value1, Param2 value2)
        {
            return (*m_methodPtr)(m_objPtr, value1, value2);
        }

        ReturnType operator()(Param1 value1, Param2 value2) const
        {
            return (*m_methodPtr)(m_objPtr, value1, value2);
        }

        /**
         * Compares two delegates
         * @param other delegate to compare with
         * @return true if both delegates are equal, false otherwise
         */
        bool operator==(const Delegate<ReturnType, Param1, Param2>& other) const
        {
            return m_objPtr == other.m_objPtr && m_methodPtr == other.m_methodPtr;
        }

    private:
        typedef ReturnType(*MethodPtr)(void*, Param1, Param2);

        /**
         * Pointer to the object for member function invocation
         */
        void* m_objPtr;

        /**
         * Pointer to the wrapped fuction
         */
        MethodPtr m_methodPtr;

        /**
         * Initializes a delegate with object pointer and wrapper function
         * @param obj_ptr for member function invocation
         * @param methodPtr pointer to the wrapper function
         * @return the created Delegate
         */
        static Delegate CreateDelegate(void* obj_ptr, MethodPtr methodPtr)
        {
            Delegate delegate;
            delegate.m_objPtr = obj_ptr;
            delegate.m_methodPtr = methodPtr;
            return delegate;
        }

        /**
         * Wrapper function for normal function calls
         * @tparam Method pointer to the delegate function
         * @param pointer to obj instance. This is only to match the MethodPtr typedef
         * @param param1 parameter for the delegated function
         * @param param2 parameter for the delegated function
         */
        template<ReturnType (*Method)(Param1, Param2)>
        static ReturnType FunctionCall(void* obj_ptr, Param1 param1, Param2 param2)
        {
            UNUSED(obj_ptr);
            return (*Method)(param1, param2);
        }

        /**
         * Wrapper function for member function calls
         * @tparam T type of the class with the member function
         * @tparam Method pointer to the delegate function
         * @param pointer to obj instance. This is only to match the MethodPtr typedef
         * @param param1 parameter for the delegated function
         * @param param2 parameter for the delegated function
         */
        template<typename T, ReturnType (T::*Method)(Param1, Param2)>
        static ReturnType MethodCall(void* obj_ptr, Param1 param1, Param2 param2)
        {
            return ((static_cast<T*>(obj_ptr))->*Method)(param1, param2);
        }
    };


    template<typename ReturnType, typename Param1>
    class Delegate<ReturnType, Param1>
    {
    public:

        Delegate()
            : m_objPtr(0)
            , m_methodPtr(0)
        {
        }

        template<typename T, ReturnType (T::*TMethod)(Param1)>
        static Delegate Create(T& instance)
        {
            return CreateDelegate(&instance, &MethodCall<T, TMethod>);
        }

        template<ReturnType(*TMethod)(Param1)>
        static Delegate Create()
        {
            return CreateDelegate(0, &FunctionCall<TMethod>);
        }

        ReturnType operator()(Param1 value1)
        {
            return (*m_methodPtr)(m_objPtr, value1);
        }

        ReturnType operator()(Param1 value1) const
        {
            return (*m_methodPtr)(m_objPtr, value1);
        }

        bool operator==(const Delegate<ReturnType, Param1>& other) const
        {
            return m_objPtr == other.m_objPtr && m_methodPtr == other.m_methodPtr;
        }

    private:
        typedef ReturnType(*MethodPtr)(void*, Param1);

        void* m_objPtr;
        MethodPtr m_methodPtr;

        static Delegate CreateDelegate(void* obj_ptr, MethodPtr methodPtr)
        {
            Delegate delegate;
            delegate.m_objPtr = obj_ptr;
            delegate.m_methodPtr = methodPtr;
            return delegate;
        }

        template<ReturnType (*Method)(Param1)>
        static ReturnType FunctionCall(void* obj_ptr, Param1 param1)
        {
            UNUSED(obj_ptr);
            return (*Method)(param1);
        }

        template<typename T, ReturnType (T::*Method)(Param1)>
        static ReturnType MethodCall(void* obj_ptr, Param1 param1)
        {
            return ((static_cast<T*>(obj_ptr))->*Method)(param1);
        }
    };

    template<typename ReturnType>
    class Delegate<ReturnType>
    {
    public:

        Delegate()
            : m_objPtr(0)
            , m_methodPtr(0)
        {
        }

        template<typename T, ReturnType (T::*TMethod)(void)>
        static Delegate Create(T& instance)
        {
            return CreateDelegate(&instance, &MethodCall<T, TMethod>);
        }

        template<ReturnType(*Method)()>
        static Delegate Create()
        {
            return CreateDelegate(0, &FunctionCall<Method>);
        }

        ReturnType operator()()
        {
            return (*m_methodPtr)(m_objPtr);
        }

        ReturnType operator()() const
        {
            return (*m_methodPtr)(m_objPtr);
        }

        bool operator==(const Delegate<ReturnType>& other) const
        {
            return m_objPtr == other.m_objPtr && m_methodPtr == other.m_methodPtr;
        }

    private:
        typedef ReturnType(*MethodPtr)(void*);

        void* m_objPtr;
        MethodPtr m_methodPtr;

        static Delegate CreateDelegate(void* obj_ptr, MethodPtr methodPtr)
        {
            Delegate delegate;
            delegate.m_objPtr = obj_ptr;
            delegate.m_methodPtr = methodPtr;
            return delegate;
        }

        template<ReturnType (*Method)()>
        static ReturnType FunctionCall(void* obj_ptr)
        {
            UNUSED(obj_ptr);
            return (*Method)();
        }

        template<typename T, ReturnType (T::*Method)(void)>
        static ReturnType MethodCall(void* obj_ptr)
        {
            return ((static_cast<T*>(obj_ptr))->*Method)();
        }
    };
}
#endif // RAMSES_CAPU_DELEGATE_H
