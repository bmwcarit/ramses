//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDARGUMENTSDATAPROVIDER_H
#define RAMSES_RAMSHCOMMANDARGUMENTSDATAPROVIDER_H

#include "Ramsh/RamshInput.h"
#include "Ramsh/RamshCommandArgumentsConverter.h"
#include "Collections/HashSet.h"
#include "PlatformAbstraction/PlatformTypeInfo.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    class RamshArgumentBase;
    typedef std::vector<RamshArgumentBase*> ArgumentVector;

    template<typename = void> class RamshArgument;

    struct RamshArgumentDataProvider;

    typedef std::vector<const RamshArgumentData*> ArgumentDataVector;

    template<typename T> struct ArgumentConverterProxy;

    class RamshArgumentBase : private StringSet
    {
    public:
        // returns the description of the argument
        String description() const;

        // returns all the concatenated keywords of the argument
        String keywords() const;

    protected:
        RamshArgumentBase(const PlatformTypeInfo& typeinfo, void* defaultValue = 0);

        virtual ~RamshArgumentBase()
        {
        }

        // amount of data consumed by the argument - usually 2 for data arguments (parameter name + data)
        virtual UInt32 amountConsumed() const;

        // try to set the argument's data with a given keyword (doesn't modify the argument itself, just returns a data object or NULL if keyword doesn't match)
        const RamshArgumentData* set(const String& keyword, const String& data) const;

        const RamshArgumentData* forceSet(const String& data) const;

        // tries to get the value of the argument (converts the data) with a data object previously retrieved by calling set/forceSet
        template<typename T> Bool getValue(const RamshArgumentData* data, T& value) const;

        virtual String typeString() const = 0;

        virtual String defaultValueString() const = 0;

        template<typename T> String defaultValueStringInternal() const;

        void registerKeywordInternal(const String& keyword);

        void setDescriptionInternal(const String& description);

        template<typename T> void setDefaultValueInternal(const T& defaultValue);

        template<typename T> inline void cleanup()
        {
            if(m_defaultValue) delete static_cast<T*>(m_defaultValue);
        }

    private:
        String m_description;
        void* m_defaultValue;
        const PlatformTypeInfo m_typeInfo;

        friend struct RamshArgumentDataProvider;
        friend class RamshArgumentProvider;

    };

    // typeless accessor to the argument definition
    template<>
    class RamshArgument<void> : public RamshArgumentBase
    {
    public:
        template<typename T> RamshArgument<void>& setDefaultValue(const T& defaultValue);

        RamshArgument<void>& registerKeyword(const String& keyword);

        RamshArgument<void>& setDescription(const String& description);
    };

    // typed argument definition
    template<typename T>
    class RamshArgument : public RamshArgumentBase
    {
    public:
        // construct an argument definition with given default value
        RamshArgument(const T& defaultValue);

        // construct an argument definition - argument will be mandatory if no default value is set later
        RamshArgument();

        RamshArgument<T>& setDefaultValue(const T& defaultValue);

        RamshArgument<T>& registerKeyword(const String& keyword);

        RamshArgument<T>& setDescription(const String& description);

        inline ~RamshArgument()
        {
            RamshArgumentBase::cleanup<T>();
        }

    protected:
        String defaultValueString() const override;

        String typeString() const override;
    };

    template<typename T>
    class TypedRamshArgument : public RamshArgument<T>
    {
    public:
        inline TypedRamshArgument(const T& defaultValue)
            : RamshArgument<T>(defaultValue)
        {}

        inline TypedRamshArgument()
            : RamshArgument<T>()
        {}
    };

    template<>
    class TypedRamshArgument<Bool> : public RamshArgument<Bool>
    {
    public:
        inline TypedRamshArgument(const Bool& defaultValue)
            : RamshArgument<Bool>(defaultValue)
        {}

        inline TypedRamshArgument()
            : RamshArgument<Bool>()
        {}

        UInt32 amountConsumed() const override;
    };

    struct RamshArgumentDataProvider
    {
        // parses the command line input according to a given argument definition vector
        RamshArgumentDataProvider(const ArgumentVector& args, const RamshInput& input);

        // try to get the converted data of the argument with index
        template<typename T> Bool getValue(UInt32 index, T& value) const;

    private:
        const ArgumentVector& m_args;
        ArgumentDataVector m_data;
    };

    // provides argument definitions
    class RamshArgumentProvider
    {
    public:
        // appends an (empty) argument definition with a given default value
        template<typename T> RamshArgument<T>& addArgument(const T& defaultValue);

        // appends an (empty) argument definition
        template<typename T> RamshArgument<T>& addArgument();

        // returns the argument definition with a given index
        RamshArgument<void>& getArgument(UInt32 index) const;

        // returns the argument definitions in a readable format
        String argumentString() const;

        // parses the given input according to the argument definitions
        RamshArgumentDataProvider parse(const RamshInput& in) const;

        // delete created argmuents
        ~RamshArgumentProvider();

    protected:
        ArgumentVector m_arguments;
    };

    // -----------------
    // RamshArgumentBase
    // -----------------

    inline RamshArgumentBase::RamshArgumentBase(const PlatformTypeInfo& typeinfo, void* defaultValue /*= 0*/)
        : m_defaultValue(defaultValue)
        , m_typeInfo(typeinfo)
    {
    }

    inline UInt32 RamshArgumentBase::amountConsumed() const
    {
        return 2; // default data amount consumed by an argument (flag + data)
    }

    inline String RamshArgumentBase::description() const
    {
        return String(m_description).append(" (").append(typeString()).append(", ").append(defaultValueString()).append(")");
    }

    inline String RamshArgumentBase::keywords() const
    {
        StringSet::ConstIterator it = StringSet::begin();
        const StringSet::ConstIterator end = StringSet::end();

        String s;
        for(;it!=end;++it)
        {
            s.append("-").append(*it).append(" ");
        }
        return s;
    }

    inline const RamshArgumentData* RamshArgumentBase::set(const String& keyword, const String& data) const
    {
        if(StringSet::hasElement(keyword))
        {
            return &data;
        }
        return 0;
    }

    inline const RamshArgumentData* RamshArgumentBase::forceSet(const String& data) const
    {
        return &data;
    }


    template<typename T>
    inline void RamshArgumentBase::setDefaultValueInternal(const T& defaultValue)
    {
        // type safety check because this class is typeless, should be able to resolve at compile-time
        if(m_typeInfo == PlatformTypeId::id<T>())
        {
            // either allocates new memory for default value or copy-assigns new default value
            if(m_defaultValue)
                *static_cast<T*>(m_defaultValue) = defaultValue;
            else
                m_defaultValue = new T(defaultValue);
        }
        else
        {
            LOG_ERROR(CONTEXT_RAMSH,"Trying to set default value of argument of type " << typeString() << " with value of wrong type " << TypeName<T>());
        }
    }

    template<typename T>
    inline Bool RamshArgumentBase::getValue(const RamshArgumentData* data, T& value) const
    {
        // type safety check + check if data matches argument definition
        if(PlatformTypeId::id<T>() != m_typeInfo)
        {
            return false;
        }

        // if data is valid, try to convert it
        if(data)
        {
            if(ArgumentConverterProxy<T>::tryConvert(m_defaultValue,*data,value))
                return true;
        }

        // if no data is present, return default value
        if(m_defaultValue)
        {
            value = *static_cast<T*>(m_defaultValue);
            return true;
        }

        return false;
    }

    template<typename T>
    inline String RamshArgumentBase::defaultValueStringInternal() const
    {
        // if a default value is set, return a string representation, else the argument is mandatory
        if(m_defaultValue)
        {
            StringOutputStream s;
            s << *static_cast<T*>(m_defaultValue);
            return s.c_str();
        }
        return "required";
    }

    inline void RamshArgumentBase::registerKeywordInternal(const String& keyword)
    {
        StringSet::put(keyword);
    }

    inline void RamshArgumentBase::setDescriptionInternal(const String& description)
    {
        m_description = description;
    }

    // -------------------
    // RamshArgument<void>
    // -------------------

    inline RamshArgument<void>& RamshArgument<void>::registerKeyword(const String& keyword)
    {
        RamshArgumentBase::registerKeywordInternal(keyword);
        return *this;
    }

    inline RamshArgument<void>& RamshArgument<void>::setDescription(const String& description)
    {
        RamshArgumentBase::setDescriptionInternal(description);
        return *this;
    }

    template<typename T>
    inline RamshArgument<void>& RamshArgument<void>::setDefaultValue(const T& defaultValue)
    {
        RamshArgumentBase::setDefaultValueInternal(defaultValue);
        return *this;
    }

    // -------------------
    // RamshArgument<T>
    // -------------------

    template<typename T>
    inline RamshArgument<T>& RamshArgument<T>::setDefaultValue(const T& defaultValue)
    {
        RamshArgumentBase::setDefaultValueInternal(defaultValue);
        return *this;
    }

    template<typename T>
    inline RamshArgument<T>::RamshArgument(const T& defaultValue)
        : RamshArgumentBase(PlatformTypeId::id<T>(),new T(defaultValue))
    {
    }

    template<typename T>
    inline RamshArgument<T>::RamshArgument()
        : RamshArgumentBase(PlatformTypeId::id<T>())
    {
    }

    template<typename T>
    inline String RamshArgument<T>::defaultValueString() const
    {
        return RamshArgumentBase::defaultValueStringInternal<T>();
    }

    template<typename T>
    inline String RamshArgument<T>::typeString() const
    {
        return TypeName<T>();
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgument<T>::registerKeyword(const String& keyword)
    {
        RamshArgumentBase::registerKeywordInternal(keyword);
        return *this;
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgument<T>::setDescription(const String& description)
    {
        RamshArgumentBase::setDescriptionInternal(description);
        return *this;
    }

    // -------------------
    // TypedRamshArgument<Bool>
    // -------------------

    inline UInt32 TypedRamshArgument<Bool>::amountConsumed() const
    {
        return 1; // data amount consumed by a bool-argument (just flag)
    }

    // -------------------
    // RamshArgumentDataProvider
    // -------------------

    inline RamshArgumentDataProvider::RamshArgumentDataProvider(const ArgumentVector& args, const RamshInput& input)
        : m_args(args)
    {
        // holds the unconsumed raw data
        std::vector<const String*> in;
        m_data.resize(m_args.size());

        // initialize the list with references to input
        for(UInt i = 1; i < input.size();i++)
        {
            in.push_back(&input[i]);
        }

        for(UInt j = 0; j < m_args.size(); j++)
        {
            // initialize current argument's data with 0
            m_data[j] = 0;
            for (UInt pos = 0; pos < in.size(); ++pos)
            {
                // determine if current raw data is a flag
                if(in[pos]->startsWith(String("-")))
                {
                    // TODO account for arguments which consume even more data?
                    if(m_args[j]->amountConsumed() > 1)
                    {
                        // if more than the flag is consumed by the argument try to get that data and consume it accordingly
                        ++pos;
                        if(pos != in.size())
                        {
                            // more data is available, try to set the argument
                            --pos;
                            const String keyword = in[pos]->substr(1,in[pos]->getLength()-1);
                            ++pos;
                            m_data[j] = m_args[j]->set(keyword, *in[pos]);
                        }
                        --pos;
                    }
                    else if(m_args[j]->amountConsumed() == 1)
                        // only the flag is consumed by the argument
                        m_data[j] = m_args[j]->set(in[pos]->substr(1,in[pos]->getLength()-1),"");

                    if(m_data[j])
                    {
                        // if an argument value was found, remove the consumed data
                        in.erase(in.begin() + pos);
                        if(m_args[j]->amountConsumed() > 1) in.erase(in.begin() + pos);

                        break;
                    }
                }
            }
        }

        // try to set positional values for each unset argument (data without a flag)
        for(UInt32 j = 0; j < m_args.size(); j++)
        {
            // only set the data if an argument has no previously found value, if any data is left and if the argument actually consumes any data
            if(!m_data[j] && in.size() > 0 && m_args[j]->amountConsumed() > 1)
            {
                m_data[j] = m_args[j]->forceSet(**in.begin());
                in.erase(in.begin());
            }
        }
    }

    template<typename T>
    inline Bool RamshArgumentDataProvider::getValue(UInt32 index, T& value) const
    {
        // get the argument data of this index
        const RamshArgumentData* argData = m_data[index];

        // validate if a valid argument value is returned
        if(!m_args[index]->getValue(argData,value))
        {
            LOG_ERROR(CONTEXT_RAMSH,"Missing/Invalid argument " << m_args[index]->keywords() << "!");
            LOG_ERROR(CONTEXT_RAMSH,"Usage: " << m_args[index]->description());
            return false;
        }

        return true;
    }

    // -------------------
    // RamshArgumentProvider
    // -------------------

    inline RamshArgumentProvider::~RamshArgumentProvider()
    {
        for (auto arg : m_arguments)
        {
            delete arg;
        }
    }

    inline String RamshArgumentProvider::argumentString() const
    {
        String s;
        for(ArgumentVector::const_iterator it = m_arguments.begin(); it != m_arguments.end(); ++it)
        {
            s.append(" [").append((*it)->keywords()).append(" - ").append((*it)->description()).append("]");
        }
        return s;
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgumentProvider::addArgument(const T& defaultValue)
    {
        TypedRamshArgument<T>* arg = new TypedRamshArgument<T>(defaultValue);
        m_arguments.push_back(arg);
        return *arg;
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgumentProvider::addArgument()
    {
        TypedRamshArgument<T>* arg = new TypedRamshArgument<T>();
        m_arguments.push_back(arg);
        return *arg;
    }

    inline RamshArgumentDataProvider RamshArgumentProvider::parse(const RamshInput& in) const
    {
        return RamshArgumentDataProvider(m_arguments,in);
    }

    inline RamshArgument<void>& RamshArgumentProvider::getArgument(UInt32 index) const
    {
        return *static_cast<RamshArgument<void>*>(m_arguments[index]);
    }

    // -------------------
    // ArgumentConverterProxy
    // -------------------

    // proxy needed for Bool-arguments because they aren't converted like other argument types
    template<>
    struct ArgumentConverterProxy<Bool>
    {
        static inline Bool tryConvert(const void* defaultValue, const RamshArgumentData&, Bool& value)
        {
            // either flip default value or return true if flag was found
            value = defaultValue ? !(*static_cast<const Bool*>(defaultValue)) : true;
            return true;
        }
    };

    template<typename T>
    struct ArgumentConverterProxy
    {
        static inline Bool tryConvert(const void*, const RamshArgumentData& data, T& value)
        {
            return ArgumentConverter<T>::tryConvert(data,value);
        }
    };
}
#endif
