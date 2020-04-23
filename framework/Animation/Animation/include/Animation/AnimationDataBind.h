//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONDATABIND_H
#define RAMSES_ANIMATIONDATABIND_H

#include "Utils/DataBind.h"
#include "Common/MemoryHandle.h"
#include "SceneAPI/IScene.h"
#include "AnimationProcessDataDispatch.h"

namespace ramses_internal
{
    class AnimationDataBindBase
    {
    public:
        virtual ~AnimationDataBindBase() {}
        virtual EDataTypeID getDataType() const = 0;
        virtual EDataBindContainerType getContainerType() const = 0;
        virtual EDataBindAccessorType getAccessorType() const = 0;
        virtual TDataBindID getBindID() const = 0;
        virtual MemoryHandle getHandle() const = 0;
        virtual MemoryHandle getHandle2() const = 0;
        virtual bool isPropertyValid() const = 0;
        virtual void setInitialValue() = 0;
        virtual void dispatch(const AnimationProcessDataDispatch& dispatcher) const = 0;
    };

    // Forward declaration
    template <typename ContainerType, typename EDataType, typename HandleType = TypeNone, typename HandleType2 = TypeNone>
    class AnimationDataBind;

    // Specialization for no handle
    template < typename ContainerType, typename EDataType >
    class AnimationDataBind < ContainerType, EDataType, TypeNone, TypeNone > : public AnimationDataBindBase
    {
    public:
        AnimationDataBind(ContainerType& instance, TDataBindID bindID);

        virtual EDataTypeID getDataType() const override;
        virtual EDataBindContainerType getContainerType() const override;
        virtual EDataBindAccessorType getAccessorType() const override;
        virtual TDataBindID getBindID() const override;
        virtual MemoryHandle getHandle() const override;
        virtual MemoryHandle getHandle2() const override;
        virtual bool isPropertyValid() const override;
        virtual void setInitialValue() override;
        virtual void dispatch(const AnimationProcessDataDispatch& dispatcher) const override;

        EDataType getValue() const;
        void setValue(const EDataType& value) const;
        const EDataType& getInitialValue() const;

    private:
        DataBind<ContainerType, EDataType> m_dataBind;
        EDataType m_initialValue;
    };

    // Specialization for 1 handle
    template < typename ContainerType, typename EDataType, typename HandleType >
    class AnimationDataBind < ContainerType, EDataType, HandleType, TypeNone > : public AnimationDataBindBase
    {
    public:
        AnimationDataBind(ContainerType& instance, HandleType handle, TDataBindID bindID);

        virtual EDataTypeID getDataType() const override;
        virtual EDataBindContainerType getContainerType() const override;
        virtual EDataBindAccessorType getAccessorType() const override;
        virtual TDataBindID getBindID() const override;
        virtual MemoryHandle getHandle() const override;
        virtual MemoryHandle getHandle2() const override;
        virtual bool isPropertyValid() const override;
        virtual void setInitialValue() override;
        virtual void dispatch(const AnimationProcessDataDispatch& dispatcher) const override;

        EDataType getValue() const;
        void setValue(const EDataType& value) const;
        const EDataType& getInitialValue() const;

    private:
        DataBind<ContainerType, EDataType, HandleType> m_dataBind;
        EDataType m_initialValue;
    };

    // Specialization for 2 handles
    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    class AnimationDataBind : public AnimationDataBindBase
    {
    public:
        AnimationDataBind(ContainerType& instance, HandleType handle, HandleType2 handle2, TDataBindID bindID);

        virtual EDataTypeID getDataType() const override;
        virtual EDataBindContainerType getContainerType() const override;
        virtual EDataBindAccessorType getAccessorType() const override;
        virtual TDataBindID getBindID() const override;
        virtual MemoryHandle getHandle() const override;
        virtual MemoryHandle getHandle2() const override;
        virtual bool isPropertyValid() const override;
        virtual void setInitialValue() override;
        virtual void dispatch(const AnimationProcessDataDispatch& dispatcher) const override;

        EDataType getValue() const;
        void setValue(const EDataType& value) const;
        const EDataType& getInitialValue() const;

    private:
        DataBind<ContainerType, EDataType, HandleType, HandleType2> m_dataBind;
        EDataType m_initialValue;
    };


    // Specialization for no handle
    template < typename ContainerType, typename EDataType >
    inline AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::AnimationDataBind(ContainerType& instance, TDataBindID bindID)
        : m_dataBind(instance, bindID)
        , m_initialValue(0)
    {
    }

    template < typename ContainerType, typename EDataType >
    inline EDataTypeID AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getDataType() const
    {
        return DataTypeToDataIDSelector<EDataType>::DataTypeID;
    }

    template < typename ContainerType, typename EDataType >
    inline EDataBindContainerType AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getContainerType() const
    {
        return DataBindContainerToContainerIDSelector<ContainerType>::ContainerID;
    }

    template < typename ContainerType, typename EDataType >
    inline EDataBindAccessorType AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getAccessorType() const
    {
        return EDataBindAccessorType_Handles_None;
    }

    template < typename ContainerType, typename EDataType >
    inline TDataBindID AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getBindID() const
    {
        return m_dataBind.getBindID();
    }

    template < typename ContainerType, typename EDataType >
    inline MemoryHandle AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getHandle() const
    {
        return InvalidMemoryHandle;
    }

    template < typename ContainerType, typename EDataType >
    inline MemoryHandle AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getHandle2() const
    {
        return InvalidMemoryHandle;
    }

    template < typename ContainerType, typename EDataType >
    inline bool AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::isPropertyValid() const
    {
        return m_dataBind.isPropertyValid();
    }

    template < typename ContainerType, typename EDataType >
    inline void AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::setInitialValue()
    {
        m_initialValue = getValue();
    }

    template < typename ContainerType, typename EDataType >
    inline void AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::dispatch(const AnimationProcessDataDispatch& dispatcher) const
    {
        dispatcher.dispatchDataBind(*this);
    }

    template < typename ContainerType, typename EDataType >
    inline EDataType AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getValue() const
    {
        return m_dataBind.getValue();
    }

    template < typename ContainerType, typename EDataType >
    inline void AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::setValue(const EDataType& value) const
    {
        m_dataBind.setValue(value);
    }

    template < typename ContainerType, typename EDataType >
    inline const EDataType& AnimationDataBind<ContainerType, EDataType, TypeNone, TypeNone>::getInitialValue() const
    {
        return m_initialValue;
    }


    // Specialization for 1 handle
    template < typename ContainerType, typename EDataType, typename HandleType >
    inline AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::AnimationDataBind(ContainerType& instance, HandleType handle, TDataBindID bindID)
        : m_dataBind(instance, handle, bindID)
        , m_initialValue(0)
    {
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline EDataTypeID AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getDataType() const
    {
        return DataTypeToDataIDSelector<EDataType>::DataTypeID;
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline EDataBindContainerType AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getContainerType() const
    {
        return DataBindContainerToContainerIDSelector<ContainerType>::ContainerID;
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline EDataBindAccessorType AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getAccessorType() const
    {
        return EDataBindAccessorType_Handles_1;
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline TDataBindID AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getBindID() const
    {
        return m_dataBind.getBindID();
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline MemoryHandle AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getHandle() const
    {
        return AsMemoryHandle(m_dataBind.getHandle());
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline MemoryHandle AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getHandle2() const
    {
        return InvalidMemoryHandle;
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline bool AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::isPropertyValid() const
    {
        return m_dataBind.isPropertyValid();
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline void AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::setInitialValue()
    {
        m_initialValue = getValue();
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline void AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::dispatch(const AnimationProcessDataDispatch& dispatcher) const
    {
        dispatcher.dispatchDataBind(*this);
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline EDataType AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getValue() const
    {
        return m_dataBind.getValue();
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline void AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::setValue(const EDataType& value) const
    {
        m_dataBind.setValue(value);
    }

    template < typename ContainerType, typename EDataType, typename HandleType >
    inline const EDataType& AnimationDataBind<ContainerType, EDataType, HandleType, TypeNone>::getInitialValue() const
    {
        return m_initialValue;
    }


    // Specialization for 2 handles
    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::AnimationDataBind(ContainerType& instance, HandleType handle, HandleType2 handle2, TDataBindID bindID)
        : m_dataBind(instance, handle, handle2, bindID)
        , m_initialValue(0)
    {
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline EDataTypeID AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getDataType() const
    {
        return DataTypeToDataIDSelector<EDataType>::DataTypeID;
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline EDataBindContainerType AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getContainerType() const
    {
        return DataBindContainerToContainerIDSelector<ContainerType>::ContainerID;
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline EDataBindAccessorType AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getAccessorType() const
    {
        return EDataBindAccessorType_Handles_2;
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline TDataBindID AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getBindID() const
    {
        return m_dataBind.getBindID();
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline MemoryHandle AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getHandle() const
    {
        return m_dataBind.getHandle();
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline MemoryHandle AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getHandle2() const
    {
        return m_dataBind.getHandle2();
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline bool AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::isPropertyValid() const
    {
        return m_dataBind.isPropertyValid();
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline void AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::setInitialValue()
    {
        m_initialValue = getValue();
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline void AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::dispatch(const AnimationProcessDataDispatch& dispatcher) const
    {
        dispatcher.dispatchDataBind(*this);
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline EDataType AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getValue() const
    {
        return m_dataBind.getValue();
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline void AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::setValue(const EDataType& value) const
    {
        m_dataBind.setValue(value);
    }

    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2 >
    inline const EDataType& AnimationDataBind<ContainerType, EDataType, HandleType, HandleType2>::getInitialValue() const
    {
        return m_initialValue;
    }
}

#endif
