//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommandArgumentsConverter.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/Ramsh/RamshTypeInfo.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    class RamshArgumentBase;
    using ArgumentVector = std::vector<RamshArgumentBase *>;

    template<typename = void> class RamshArgument;

    struct RamshArgumentDataProvider;

    using ArgumentDataVector = std::vector<const RamshArgumentData *>;

    template<typename T> struct ArgumentConverterProxy;

    class RamshArgumentBase
    {
    public:
        // returns the description of the argument
        [[nodiscard]] std::string description() const;

        // returns all the concatenated keywords of the argument
        [[nodiscard]] std::string keywords() const;

    protected:
        explicit RamshArgumentBase(const RamshTypeInfo& typeinfo, void* defaultValue = nullptr);

        virtual ~RamshArgumentBase() = default;

        // amount of data consumed by the argument - usually 2 for data arguments (parameter name + data)
        [[nodiscard]] virtual uint32_t amountConsumed() const;

        // try to set the argument's data with a given keyword (doesn't modify the argument itself, just returns a data object or nullptr if keyword doesn't match)
        [[nodiscard]] const RamshArgumentData* set(const std::string& keyword, const std::string& data) const;

        // tries to get the value of the argument (converts the data) with a data object previously retrieved by calling set/forceSet
        template<typename T> bool getValue(const RamshArgumentData* data, T& value) const;

        [[nodiscard]] virtual std::string typeString() const = 0;

        [[nodiscard]] virtual std::string defaultValueString() const = 0;

        template<typename T> [[nodiscard]] std::string defaultValueStringInternal() const;

        void registerKeywordInternal(const std::string& keyword);

        void setDescriptionInternal(const std::string& description);

        template<typename T> void setDefaultValueInternal(const T& defaultValue);

        template<typename T> inline void cleanup()
        {
            delete static_cast<T*>(m_defaultValue);
        }

    private:
        HashSet<std::string> m_keywords;
        std::string m_description;
        void* m_defaultValue;
        const RamshTypeInfo m_typeInfo;

        friend struct RamshArgumentDataProvider;
        friend class RamshArgumentProvider;

    };

    // typeless accessor to the argument definition
    template<>
    class RamshArgument<void> : public RamshArgumentBase
    {
    public:
        template<typename T> RamshArgument<void>& setDefaultValue(const T& defaultValue);

        RamshArgument<void>& registerKeyword(const std::string& keyword);

        RamshArgument<void>& setDescription(const std::string& description);
    };

    // typed argument definition
    template<typename T>
    class RamshArgument : public RamshArgumentBase
    {
    public:
        // construct an argument definition with given default value
        explicit RamshArgument(const T& defaultValue);

        // construct an argument definition - argument will be mandatory if no default value is set later
        RamshArgument();

        RamshArgument<T>& setDefaultValue(const T& defaultValue);

        RamshArgument<T>& registerKeyword(const std::string& keyword);

        RamshArgument<T>& setDescription(const std::string& description);

        inline ~RamshArgument() override
        {
            RamshArgumentBase::cleanup<T>();
        }

    protected:
        [[nodiscard]] std::string defaultValueString() const override;

        [[nodiscard]] std::string typeString() const override;
    };

    template<typename T>
    class TypedRamshArgument : public RamshArgument<T>
    {
    public:
        inline explicit TypedRamshArgument(const T& defaultValue)
            : RamshArgument<T>(defaultValue)
        {}

        inline TypedRamshArgument()
            : RamshArgument<T>()
        {}
    };

    template<>
    class TypedRamshArgument<bool> : public RamshArgument<bool>
    {
    public:
        inline explicit TypedRamshArgument(const bool& defaultValue)
            : RamshArgument<bool>(defaultValue)
        {}

        TypedRamshArgument() = default;

        [[nodiscard]] uint32_t amountConsumed() const override;
    };

    struct RamshArgumentDataProvider
    {
        // parses the command line input according to a given argument definition vector
        RamshArgumentDataProvider(const ArgumentVector& args, const std::vector<std::string>& input);

        // try to get the converted data of the argument with index
        template<typename T> bool getValue(uint32_t index, T& value) const;

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
        [[nodiscard]] RamshArgument<void>& getArgument(uint32_t index) const;

        // returns the argument definitions in a readable format
        [[nodiscard]] std::string argumentString() const;

        // parses the given input according to the argument definitions
        [[nodiscard]] RamshArgumentDataProvider parse(const std::vector<std::string>& in) const;

        // delete created argmuents
        ~RamshArgumentProvider();

    protected:
        ArgumentVector m_arguments;
    };

    // -----------------
    // RamshArgumentBase
    // -----------------

    inline RamshArgumentBase::RamshArgumentBase(const RamshTypeInfo& typeinfo, void* defaultValue /*= 0*/)
        : m_defaultValue(defaultValue)
        , m_typeInfo(typeinfo)
    {
    }

    inline uint32_t RamshArgumentBase::amountConsumed() const
    {
        return 2; // default data amount consumed by an argument (flag + data)
    }

    inline std::string RamshArgumentBase::description() const
    {
        return fmt::format("{} ({}, {})", m_description, typeString(), defaultValueString());
    }

    inline std::string RamshArgumentBase::keywords() const
    {
        std::string result;
        for (const auto& kw : m_keywords)
            result += "-" + kw;
        return result;
    }

    inline const RamshArgumentData* RamshArgumentBase::set(const std::string& keyword, const std::string& data) const
    {
        if(m_keywords.contains(keyword))
        {
            return &data;
        }
        return nullptr;
    }

    template<typename T>
    inline void RamshArgumentBase::setDefaultValueInternal(const T& defaultValue)
    {
        // type safety check because this class is typeless, should be able to resolve at compile-time
        if(m_typeInfo == RamshTypeId::id<T>())
        {
            // either allocates new memory for default value or copy-assigns new default value
            if (m_defaultValue)
            {
                *static_cast<T*>(m_defaultValue) = defaultValue;
            }
            else
            {
                m_defaultValue = new T(defaultValue);
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_RAMSH, "Trying to set default value of argument of type {} with value of wrong type {}", typeString(), static_cast<std::string>(TypeName<T>()));
        }
    }

    template<typename T>
    inline bool RamshArgumentBase::getValue(const RamshArgumentData* data, T& value) const
    {
        // type safety check + check if data matches argument definition
        if(RamshTypeId::id<T>() != m_typeInfo)
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
    inline std::string RamshArgumentBase::defaultValueStringInternal() const
    {
        // if a default value is set, return a string representation, else the argument is mandatory
        if(m_defaultValue)
        {
            return fmt::format("{}", *static_cast<T*>(m_defaultValue));
        }
        return "required";
    }

    inline void RamshArgumentBase::registerKeywordInternal(const std::string& keyword)
    {
        m_keywords.put(keyword);
    }

    inline void RamshArgumentBase::setDescriptionInternal(const std::string& description)
    {
        m_description = description;
    }

    // -------------------
    // RamshArgument<void>
    // -------------------

    inline RamshArgument<void>& RamshArgument<void>::registerKeyword(const std::string& keyword)
    {
        RamshArgumentBase::registerKeywordInternal(keyword);
        return *this;
    }

    inline RamshArgument<void>& RamshArgument<void>::setDescription(const std::string& description)
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
        : RamshArgumentBase(RamshTypeId::id<T>(),new T(defaultValue))
    {
    }

    template<typename T>
    inline RamshArgument<T>::RamshArgument()
        : RamshArgumentBase(RamshTypeId::id<T>())
    {
    }

    template<typename T>
    inline std::string RamshArgument<T>::defaultValueString() const
    {
        return RamshArgumentBase::defaultValueStringInternal<T>();
    }

    template<typename T>
    inline std::string RamshArgument<T>::typeString() const
    {
        return static_cast<std::string>(TypeName<T>());
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgument<T>::registerKeyword(const std::string& keyword)
    {
        RamshArgumentBase::registerKeywordInternal(keyword);
        return *this;
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgument<T>::setDescription(const std::string& description)
    {
        RamshArgumentBase::setDescriptionInternal(description);
        return *this;
    }

    // -------------------
    // TypedRamshArgument<bool>
    // -------------------

    inline uint32_t TypedRamshArgument<bool>::amountConsumed() const
    {
        return 1; // data amount consumed by a bool-argument (just flag)
    }

    // -------------------
    // RamshArgumentDataProvider
    // -------------------

    inline RamshArgumentDataProvider::RamshArgumentDataProvider(const ArgumentVector& args, const std::vector<std::string>& input)
        : m_args(args)
    {
        // holds the unconsumed raw data
        std::vector<const std::string*> in;
        m_data.resize(m_args.size());

        // initialize the list with references to input
        for(size_t i = 1; i < input.size(); i++)
        {
            in.push_back(&input[i]);
        }

        for(size_t j = 0; j < m_args.size(); j++)
        {
            // initialize current argument's data with 0
            m_data[j] = nullptr;
            for (size_t pos = 0; pos < in.size(); ++pos)
            {
                // determine if current raw data is a flag
                if(in[pos]->find("-") == 0)
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
                            const std::string keyword = in[pos]->substr(1,in[pos]->size()-1);
                            ++pos;
                            m_data[j] = m_args[j]->set(keyword, *in[pos]);
                        }
                        --pos;
                    }
                    else if (m_args[j]->amountConsumed() == 1)
                    {
                        // only the flag is consumed by the argument
                        m_data[j] = m_args[j]->set(in[pos]->substr(1, in[pos]->size() - 1), "");
                    }

                    if(m_data[j])
                    {
                        // if an argument value was found, remove the consumed data
                        in.erase(in.begin() + static_cast<std::ptrdiff_t>(pos));
                        if(m_args[j]->amountConsumed() > 1)
                            in.erase(in.begin() + static_cast<std::ptrdiff_t>(pos));

                        break;
                    }
                }
            }
        }

        // try to set positional values for each unset argument (data without a flag)
        for(size_t j = 0; j < m_args.size(); j++)
        {
            // only set the data if an argument has no previously found value, if any data is left and if the argument actually consumes any data
            if(!m_data[j] && !in.empty() && m_args[j]->amountConsumed() > 1)
            {
                m_data[j] = *in.begin();
                in.erase(in.begin());
            }
        }
    }

    template<typename T>
    inline bool RamshArgumentDataProvider::getValue(uint32_t index, T& value) const
    {
        // get the argument data of this index
        const RamshArgumentData* argData = m_data[index];

        // validate if a valid argument value is returned
        if(!m_args[index]->getValue(argData,value))
        {
            LOG_ERROR(CONTEXT_RAMSH, "Missing/Invalid argument {}!", m_args[index]->keywords());
            LOG_ERROR(CONTEXT_RAMSH, "Usage: {}", m_args[index]->description());
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

    inline std::string RamshArgumentProvider::argumentString() const
    {
        std::string result;
        for (const auto& arg : m_arguments)
            result += fmt::format(" [{} - {}]", arg->keywords(), arg->description());
        return result;
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgumentProvider::addArgument(const T& defaultValue)
    {
        auto* arg = new TypedRamshArgument<T>(defaultValue);
        m_arguments.push_back(arg);
        return *arg;
    }

    template<typename T>
    inline RamshArgument<T>& RamshArgumentProvider::addArgument()
    {
        auto* arg = new TypedRamshArgument<T>();
        m_arguments.push_back(arg);
        return *arg;
    }

    inline RamshArgumentDataProvider RamshArgumentProvider::parse(const std::vector<std::string>& in) const
    {
        return RamshArgumentDataProvider(m_arguments,in);
    }

    inline RamshArgument<void>& RamshArgumentProvider::getArgument(uint32_t index) const
    {
        return *static_cast<RamshArgument<void>*>(m_arguments[index]);
    }

    // -------------------
    // ArgumentConverterProxy
    // -------------------

    // proxy needed for bool-arguments because they aren't converted like other argument types
    template<>
    struct ArgumentConverterProxy<bool>
    {
        static inline bool tryConvert(const void* defaultValue, const RamshArgumentData& /*unused*/, bool& value)
        {
            // either flip default value or return true if flag was found
            value = defaultValue ? !(*static_cast<const bool*>(defaultValue)) : true;
            return true;
        }
    };

    template<typename T>
    struct ArgumentConverterProxy
    {
        static inline bool tryConvert(const void* /*unused*/, const RamshArgumentData& data, T& value)
        {
            return ArgumentConverter<T>::tryConvert(data,value);
        }
    };
}

