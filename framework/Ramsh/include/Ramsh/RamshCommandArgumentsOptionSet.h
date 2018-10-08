//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDARGUMENTSOPTIONSET_H
#define RAMSES_RAMSHCOMMANDARGUMENTSOPTIONSET_H

#include "Collections/Vector.h"
#include "Collections/String.h"
#include "Ramsh/RamshCommandArgumentsConverter.h"
#include "PlatformAbstraction/PlatformTypeInfo.h"

#define DEF_EXT_LITERAL(str) \
    extern const Char EXT_LITERAL_##str[];
#define DECL_EXT_LITERAL(str) \
    extern const Char EXT_LITERAL_##str[] = #str;
#define EXT_LITERAL(str) \
    EXT_LITERAL_##str

namespace ramses_internal
{
    extern const Char ARGUMENT_OPTION_NONE[];

    DEF_EXT_LITERAL(true)
    DEF_EXT_LITERAL(false)

    // option set: takes values from 0 to n according to the found string literal
    template<const Char* option1 = ARGUMENT_OPTION_NONE,
                const Char* option2 = ARGUMENT_OPTION_NONE,
                const Char* option3 = ARGUMENT_OPTION_NONE,
                const Char* option4 = ARGUMENT_OPTION_NONE,
                const Char* option5 = ARGUMENT_OPTION_NONE>
    struct OptionSet;

    // 0 for false, 1 for true
    typedef OptionSet<EXT_LITERAL(false),EXT_LITERAL(true)> BoolLiteral;

    // implementation
    template<const Char*, const Char*, const Char*, const Char*, const Char*>
    struct OptionSetBase;

    template<>
    struct OptionSetBase
        <ARGUMENT_OPTION_NONE,ARGUMENT_OPTION_NONE,ARGUMENT_OPTION_NONE,ARGUMENT_OPTION_NONE,ARGUMENT_OPTION_NONE>
    {
        virtual ~OptionSetBase()
        {}

    protected:
        virtual inline UInt32 parseInternal(const Char*, UInt32) const
        {
            return static_cast<UInt32>(-1);
        }
    };

    template<const Char* option1,const Char* option2,const Char* option3,const Char* option4,const Char* option5>
    struct OptionSetBase
        : public OptionSetBase<option2,option3,option4,option5,ARGUMENT_OPTION_NONE>
    {
    protected:
        virtual inline UInt32 parseInternal(const Char* data, UInt32 level) const
        {
            return 0 == ramses_capu::StringUtils::Strcmp(option1,data) ?
                level :
                OptionSetBase<option2,option3,option4,option5,ARGUMENT_OPTION_NONE>::parseInternal(data,level+1);
        }
    };

    template<const Char* option1,const Char* option2,const Char* option3,const Char* option4,const Char* option5>
    struct OptionSet
        : public OptionSetBase<option1,option2,option3,option4,option5>
    {
        inline OptionSet(UInt32 value = 0)
            : m_value(value)
        {
        }

        inline operator UInt32() const
        {
            return m_value;
        }

        inline operator Bool() const
        {
            return m_value > 0;
        }

        inline Bool parse(const Char* data)
        {
            m_value = OptionSetBase<option1,option2,option3,option4,option5>::parseInternal(data,0);
            return m_value != static_cast<UInt32>(-1);
        }

    private:
        UInt32 m_value;
    };

    template<const Char* option1,const Char* option2,const Char* option3,const Char* option4,const Char* option5>
    struct ArgumentConverter< OptionSet<option1,option2,option3,option4,option5> >
    {
        static inline Bool tryConvert(const RamshArgumentData& data, OptionSet<option1,option2,option3,option4,option5>& value)
        {
            return value.parse(data.c_str());
        }
    };

    template<const Char* option1,const Char* option2,const Char* option3,const Char* option4,const Char* option5>
    StringOutputStream& operator<<(StringOutputStream& lhs, OptionSet<option1,option2,option3,option4,option5>& rhs)
    {
        const Char* const options[] =
        {
            option1,
            option2,
            option3,
            option4,
            option5
        };

        if(static_cast<UInt32>(rhs) < 5)
            lhs << options[static_cast<UInt32>(rhs)];

        return lhs;
    }

    template<const Char* option1,const Char* option2,const Char* option3,const Char* option4,const Char* option5>
    struct TypeName< OptionSet<option1,option2,option3,option4,option5> >
    {
        inline operator String() const
        {
            String ret;
            const Char* const options[] =
            {
                option1,
                option2,
                option3,
                option4,
                option5
            };

            ret.append("options:");

            for(UInt32 i = 0; i < 5; i++)
            {
                if(ARGUMENT_OPTION_NONE != options[i])
                    ret.append(" ").append(options[i]);
                else
                    break;
            }

            return ret;
        }
    };

}// namespace ramses_internal

#endif
