//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDARGUMENTS_H
#define RAMSES_RAMSHCOMMANDARGUMENTS_H

#include "Ramsh/RamshCommand.h"
#include "Ramsh/RamshCommandArgumentsDataProvider.h"
#include "Ramsh/RamshCommandArgumentsUtils.h"

namespace ramses_internal
{

    // base class for easy-to-use argument definitions
    template<typename T0, typename T1 = void, typename T2 = void, typename T3 = void, typename T4 = void>
    struct RamshCommandArgs;

    // --------------------------

    // empty argument slot
    struct Unused
    {};

    // implementation base class
    template<Int32 N,typename T0 = Unused,typename T1 = Unused,typename T2 = Unused,typename T3 = Unused, typename T4 = Unused>
    class RamshCommandArgsBase;

    template<Int32 N>
    class RamshCommandArgsBase<N,Unused,Unused,Unused,Unused,Unused>
        : protected RamshArgumentProvider, public RamshCommand
    {
    public:
        RamshCommandArgsBase();

        // from RamshCommand
        bool executeInput(const std::vector<std::string>& in) override;
        std::string descriptionString() const override;

        virtual ~RamshCommandArgsBase() override
        {
        }

    protected:
        // gets implemented by a deriving class, which either tries to get the value of the next argument or executes the command implementation
        virtual bool executeInternal(const RamshArgumentDataProvider&,Unused&,Unused&,Unused&,Unused&,Unused&) const = 0;
        // initialize the argument definitions
        virtual void init();
    };

    template<Int32 N, typename T0,typename T1,typename T2,typename T3,typename T4>
    class RamshCommandArgsBase
        : public RamshCommandArgsBase<N+1,T1,T2,T3,T4>
    {
    public:
        RamshCommandArgsBase();
        using RamshCommandArgsBase<N + 1, T1, T2, T3, T4>::executeInput;

    protected:
        using RamshCommandArgsBase<N+1,T1,T2,T3,T4>::executeInternal;

        // gets the value of the current (N-th) argument and calls the deriving class' implementation to either get the next argument or execute the command implementation
        bool executeInternal(const RamshArgumentDataProvider&,T1&,T2&,T3&,T4&,Unused&) const override;

        // gets implemented by a deriving class, which either tries to get the value of the next argument or executes the command implementation
        virtual bool executeInternal(const RamshArgumentDataProvider&,T0&,T1&,T2&,T3&,T4&) const = 0;

        // initializes the current (N-th) argument and call the base class' implementation to initialize the next argument
        virtual void init() override;
    };

    // -------------------------------------------------------
    // Generate classes according to the following definition:
    // -------------------------------------------------------

    // template<typename T0, ... typename TN>
    // struct RamshCommandArgs<T0, ... TN, ... void>
    //     : public RamshCommandArgsBase<0,T0, ... TN>
    // {
    //     using RamshCommandArgsBase<0,T0, ... TN>::RamshArgumentProvider::getArgument;
    //
    //     template<Int32 n>
    //     inline TypedRamshArgument<typename ramsh_utils::SelectType<n,T0, ... TN>::type>& getArgument()
    //     {
    //         return *static_cast<TypedRamshArgument<typename ramsh_utils::SelectType<n,T0, ... TN>::type*>(RamshCommandArgsBase<0,T0, ... TN>::RamshArgumentProvider::m_arguments[n]);
    //     }
    //
    //     virtual bool execute(T0&, ... TN&) const = 0;
    //
    // protected:
    //     inline bool executeInternal(const RamshArgumentDataProvider&, T0& a0, ... TN& aN&)
    //     {
    //         return execute(a0, ... aN);
    //     }
    //
    //     using RamshCommandArgsBase<0,T0, ... TN>::executeInternal;
    // };

#define ECHO_TYPENAME(N) ,typename T##N
#define ECHO_SPEC_TYPE(N) ,T##N
#define ECHO_SPEC_VOID(N) ,void
#define ECHO_SPEC_(N)
#define ECHO_SPEC_BASE_CLASS(N) <\
    T0 \
    RAMSH_REPEAT2(N, ECHO_SPEC_TYPE, ECHO_SPEC_VOID) \
>
#define ECHO_FUNC_HEADER(N) ,T##N& a##N
#define ECHO_FUNC_UNUSED(N) ,Unused&
#define ECHO_FUNC_CALL(N) ,a##N

#define ECHO_CLASS(N) \
    template<typename T0 \
    RAMSH_REPEAT(N, ECHO_TYPENAME) \
    > \
struct RamshCommandArgs \
    RAMSH_COND_CALL(BASE_CLASS,ECHO_SPEC)(N) \
    : \
    public RamshCommandArgsBase<0,T0 \
    RAMSH_REPEAT(N, ECHO_SPEC_TYPE) \
> { \
    using RamshCommandArgsBase<0,T0 \
    RAMSH_REPEAT(N, ECHO_SPEC_TYPE) \
    >::RamshArgumentProvider::getArgument;\
    \
    using RamshCommandArgsBase<0,T0 \
    RAMSH_REPEAT(N, ECHO_SPEC_TYPE) \
    >::executeInput;\
    \
    template<Int32 n>\
    inline TypedRamshArgument<typename ramsh_utils::SelectType<n,T0\
    RAMSH_REPEAT(N, ECHO_SPEC_TYPE)\
    >::type>& getArgument()\
    {return *static_cast<TypedRamshArgument<typename ramsh_utils::SelectType<n,T0\
    RAMSH_REPEAT(N, ECHO_SPEC_TYPE)\
    >::type>*>(\
    RamshCommandArgsBase<0,T0 \
    RAMSH_REPEAT(N, ECHO_SPEC_TYPE) \
    >::RamshArgumentProvider::m_arguments[n]);}\
    \
    virtual bool execute(T0& a0\
    RAMSH_REPEAT(N, ECHO_FUNC_HEADER) \
    ) const = 0; \
    \
    protected: \
    inline bool executeInternal(const RamshArgumentDataProvider&, T0& a0 \
    RAMSH_REPEAT2(N, ECHO_FUNC_HEADER, ECHO_FUNC_UNUSED) \
    ) const override { \
    return execute(a0 \
    RAMSH_REPEAT(N, ECHO_FUNC_CALL) \
    ); \
    } \
    \
    using RamshCommandArgsBase<0,T0 \
    RAMSH_REPEAT(N, ECHO_SPEC_TYPE) \
    >::executeInternal; \
    \
};

#define BASE_CLASS
    ECHO_CLASS(4)

#undef BASE_CLASS

    ECHO_CLASS(3)
    ECHO_CLASS(2)
    ECHO_CLASS(1)
    ECHO_CLASS(0)

#undef ECHO_CLASS

    // --------------------
    // RamshCommandArgsBase
    // --------------------

    template<Int32 N, typename T0,typename T1,typename T2, typename T3, typename T4>
    inline RamshCommandArgsBase<N,T0,T1,T2,T3,T4>::RamshCommandArgsBase()
    {
        if(0 == N) // initialize on lowest level
            init();
    }

    template<Int32 N, typename T0,typename T1,typename T2, typename T3, typename T4>
    inline bool RamshCommandArgsBase<N,T0,T1,T2,T3,T4>::executeInternal(const RamshArgumentDataProvider& args,T1& a1,T2& a2,T3& a3,T4& a4,Unused&) const
    {
        T0 val;

        // tries to get the value of the current argument and calls downwards with the set value
        if(args.getValue(N,val))
            return executeInternal(args, val, a1, a2, a3, a4);

        return false;
    }

    template<Int32 N, typename T0,typename T1,typename T2, typename T3, typename T4>
    inline void RamshCommandArgsBase<N,T0,T1,T2,T3,T4>::init()
    {
        // add the current (empty) argument definition
        RamshArgumentProvider::addArgument<T0>();
        // call upwards to initialize the next definition
        RamshCommandArgsBase<N+1,T1,T2,T3,T4>::init();
    }

    template<Int32 N>
    inline RamshCommandArgsBase<N,Unused,Unused,Unused,Unused,Unused>::RamshCommandArgsBase()
    {
    }

    template<Int32 N>
    inline bool RamshCommandArgsBase<N,Unused,Unused,Unused,Unused,Unused>::executeInput(const std::vector<std::string>& in)
    {
        // parse the raw input data and call downwards with it
        RamshArgumentDataProvider args = RamshArgumentProvider::parse(in);
        Unused u;
        return executeInternal(args,u,u,u,u,u);
    }

    template<Int32 N>
    inline std::string RamshCommandArgsBase<N,Unused,Unused,Unused,Unused,Unused>::descriptionString() const
    {
        return fmt::format("Usage: {} - {}",
                           RamshArgumentProvider::argumentString(),
                           RamshCommand::descriptionString());
    }

    template<Int32 N>
    inline void RamshCommandArgsBase<N,Unused,Unused,Unused,Unused,Unused>::init()
    {
    }

}// namespace ramses_internal

#endif
