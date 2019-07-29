//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformSharedPointer.h"
#include "gmock/gmock.h"
#include "framework_common_gmock_header.h"

namespace ramses_internal
{
    class BaseClass
    {
    public:
        BaseClass(ramses_capu::uint_t value)
            : mValue(value)
        {
            ++mReferences;
        }

        ~BaseClass()
        {
            --mReferences;
        }

        ramses_capu::uint_t mValue;
        static ramses_capu::uint_t mReferences;

    private:
        BaseClass(const BaseClass&);
    };

    ramses_capu::uint_t BaseClass::mReferences = 0;

    class DerivedClass : public BaseClass
    {
    public:
        DerivedClass(ramses_capu::uint_t value)
            : BaseClass(value)
        {
        }
    };

    class Deleteme
    {
    public:
        MOCK_METHOD0(destructor, void());
        virtual ~Deleteme()
        {
            destructor();
        }
    };

    template <typename T>
    class TestDeleter
    {
    public:
        explicit TestDeleter(bool& wasCalled)
            : mWasCalled(wasCalled)
        {
        }

        void operator()(T* /*p*/)
        {
            assert(!mWasCalled);
            mWasCalled = true;
        }

    private:
        bool& mWasCalled;
    };

    template <typename T>
    class DeletingTestDeleter
    {
    public:
        DeletingTestDeleter(bool& wasCalled)
            : mWasCalled(wasCalled)
        {
        }

        void operator()(T* p)
        {
            assert(!mWasCalled);
            mWasCalled = true;
            delete p;
        }

    private:
        bool& mWasCalled;
    };

    class ASharedPtr : public ::testing::Test
    {
    public:
        ASharedPtr()
        {
            BaseClass::mReferences = 0;
            wasCalled = false;
        }

        bool wasCalled;
    };

    TEST_F(ASharedPtr, DefaultConstructor)
    {
        PlatformSharedPointer<ramses_capu::uint_t> sp;
        EXPECT_EQ(0, sp.use_count());
        EXPECT_TRUE(nullptr == sp.get());
    }

    TEST_F(ASharedPtr, ConstructorWithNullPointerVariable)
    {
        ramses_capu::uint_t *nullPtr = nullptr;
        PlatformSharedPointer<ramses_capu::uint_t> sp(nullPtr);
        EXPECT_EQ(1, sp.use_count());
        EXPECT_TRUE(nullptr == sp.get());
        EXPECT_TRUE(nullptr == sp);
    }

    TEST_F(ASharedPtr, ConstructorWithNullPointer)
    {
        PlatformSharedPointer<ramses_capu::uint_t> sp(nullptr);
        EXPECT_EQ(0, sp.use_count());
        EXPECT_TRUE(nullptr == sp.get());
        EXPECT_TRUE(nullptr == sp);
    }

    TEST_F(ASharedPtr, PointerConstructor)
    {
        PlatformSharedPointer<ramses_capu::uint_t> sp(new ramses_capu::uint_t(123));
        EXPECT_EQ(1, sp.use_count());
        EXPECT_EQ(123u, *sp);
    }

    TEST_F(ASharedPtr, PointerConstructorConvertible)
    {
        PlatformSharedPointer<BaseClass> sp(new DerivedClass(123));
        EXPECT_EQ(1, sp.use_count());
        EXPECT_EQ(123u, sp->mValue);
        EXPECT_EQ(1u, sp->mReferences);
    }

    TEST_F(ASharedPtr, DestructorOnLastOneOwnerOnly)
    {
        {
            PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
            EXPECT_EQ(1u, BaseClass::mReferences);
        }
        EXPECT_EQ(0u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, ConstructorWithNullPointerWithDeleter)
    {
        PlatformSharedPointer<ramses_capu::uint_t> sp(nullptr, DeletingTestDeleter<ramses_capu::uint_t>(wasCalled));
        EXPECT_EQ(1, sp.use_count());
        EXPECT_TRUE(nullptr == sp.get());
        EXPECT_TRUE(nullptr == sp);
        EXPECT_FALSE(wasCalled);
    }

    TEST_F(ASharedPtr, ConstructorWithNullPointerVariableWithDeleter)
    {
        ramses_capu::uint_t *nullPtr = nullptr;
        PlatformSharedPointer<ramses_capu::uint_t> sp(nullPtr, DeletingTestDeleter<ramses_capu::uint_t>(wasCalled));
        EXPECT_EQ(1, sp.use_count());
        EXPECT_TRUE(nullptr == sp.get());
        EXPECT_TRUE(nullptr == sp);
        EXPECT_FALSE(wasCalled);
    }

    TEST_F(ASharedPtr, PointerConstructorWithDeleter)
    {
        {
            PlatformSharedPointer<ramses_capu::uint_t> sp(new ramses_capu::uint_t(123), DeletingTestDeleter<ramses_capu::uint_t>(wasCalled));
            EXPECT_EQ(1, sp.use_count());
            EXPECT_EQ(123u, *sp);
        }
        EXPECT_TRUE(wasCalled);
    }

    TEST_F(ASharedPtr, PointerConstructorConvertibleWithDeleter)
    {
        {
            PlatformSharedPointer<BaseClass> sp(new DerivedClass(123), DeletingTestDeleter<BaseClass>(wasCalled));
            EXPECT_EQ(1, sp.use_count());
            EXPECT_EQ(123u, sp->mValue);
            EXPECT_EQ(1u, sp->mReferences);
        }
        EXPECT_TRUE(wasCalled);
    }

    TEST_F(ASharedPtr, CopyConstructor)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other(sp);
        EXPECT_EQ(2, sp.use_count());
        EXPECT_EQ(2, other.use_count());
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, CopyConstructorConvertibleClass)
    {
        PlatformSharedPointer<DerivedClass> sp(new DerivedClass(123));
        PlatformSharedPointer<BaseClass> other(sp);
        EXPECT_EQ(2, sp.use_count());
        EXPECT_EQ(2, other.use_count());
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, MoveConstructor)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other(std::move(sp));
        EXPECT_EQ(0, sp.use_count());
        EXPECT_TRUE(sp.get() == nullptr);

        EXPECT_EQ(1, other.use_count());
        ASSERT_TRUE(other.get() != nullptr);
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, DestructorOnLastOwner)
    {
        PlatformSharedPointer<BaseClass> sp1;
        PlatformSharedPointer<BaseClass> sp2;
        PlatformSharedPointer<BaseClass> sp3(new BaseClass(123));
        sp1 = sp3;
        sp2 = sp3;
        EXPECT_EQ(3, sp1.use_count());
        EXPECT_EQ(1u, BaseClass::mReferences);
        sp3.reset();
        sp2.reset();
        EXPECT_EQ(1, sp1.use_count());
        EXPECT_EQ(1u, BaseClass::mReferences);
        sp1.reset();
        EXPECT_EQ(0, sp1.use_count());
        EXPECT_EQ(0u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, DeleterOnLastOwner)
    {
        PlatformSharedPointer<BaseClass> sp1;
        PlatformSharedPointer<BaseClass> sp2;
        PlatformSharedPointer<BaseClass> sp3(new BaseClass(123), DeletingTestDeleter<BaseClass>(wasCalled));
        sp1 = sp3;
        sp2 = sp3;
        sp3.reset();
        sp2.reset();
        EXPECT_FALSE(wasCalled);
        sp1.reset();
        EXPECT_TRUE(wasCalled);
    }

    TEST_F(ASharedPtr, CopyConstructorWithDeleter)
    {
        {
            PlatformSharedPointer<BaseClass> sp(new BaseClass(123), DeletingTestDeleter<BaseClass>(wasCalled));
            PlatformSharedPointer<BaseClass> other(sp);
            EXPECT_EQ(2, sp.use_count());
            EXPECT_EQ(2, other.use_count());
            EXPECT_EQ(123u, other->mValue);
            EXPECT_EQ(1u, BaseClass::mReferences);
        }
        EXPECT_TRUE(wasCalled);
    }

    TEST_F(ASharedPtr, Accessors)
    {
        BaseClass *c = new BaseClass(123);
        PlatformSharedPointer<BaseClass> sp(c);
        EXPECT_TRUE(c == sp.get());
        EXPECT_TRUE(c == &(*sp));
        EXPECT_EQ(123u, sp->mValue);
    }

    TEST_F(ASharedPtr, AssignSameType)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other;
        other = sp;
        EXPECT_EQ(2, sp.use_count());
        EXPECT_TRUE(sp.get() == other.get());
    }

    TEST_F(ASharedPtr, AssignConvertibleType)
    {
        PlatformSharedPointer<DerivedClass> sp(new DerivedClass(123));
        PlatformSharedPointer<BaseClass> other;
        other = sp;
        EXPECT_EQ(2, sp.use_count());
        EXPECT_TRUE(sp.get() == other.get());
    }

    TEST_F(ASharedPtr, AssignSelf)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        sp = *&sp;
        EXPECT_EQ(1, sp.use_count());
    }

    TEST_F(ASharedPtr, AssignSamePointer)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other(sp);
        EXPECT_EQ(2, sp.use_count());
        sp = other;
        EXPECT_EQ(2, sp.use_count());
    }

    TEST_F(ASharedPtr, AssignEmptySharedPtr)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other;
        EXPECT_EQ(1, sp.use_count());
        sp = other;
        EXPECT_EQ(0, sp.use_count());
        EXPECT_EQ(0u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, MoveAssignToEmpty)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other;
        other = std::move(sp);

        EXPECT_EQ(0, sp.use_count());
        EXPECT_TRUE(sp.get() == nullptr);

        EXPECT_EQ(1, other.use_count());
        ASSERT_TRUE(other.get() != nullptr);
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, MoveAssignToInUse)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other(new BaseClass(456));
        other = std::move(sp);

        EXPECT_EQ(0, sp.use_count());
        EXPECT_TRUE(sp.get() == nullptr);

        EXPECT_EQ(1, other.use_count());
        ASSERT_TRUE(other.get() != nullptr);
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, ResetToNull)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        sp.reset();
        EXPECT_TRUE(sp.get() == nullptr);
        EXPECT_EQ(0, sp.use_count());
        EXPECT_EQ(0u, BaseClass::mReferences);
        sp.reset();
        EXPECT_EQ(0, sp.use_count());
    }

    TEST_F(ASharedPtr, ResetToNullShared)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other(sp);
        EXPECT_EQ(2, sp.use_count());
        sp.reset();
        EXPECT_TRUE(sp.get() == nullptr);
        EXPECT_EQ(0, sp.use_count());
        EXPECT_EQ(1, other.use_count());
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, ResetEmptyToPointer)
    {
        PlatformSharedPointer<BaseClass> sp;
        sp.reset(new BaseClass(123));
        EXPECT_TRUE(sp.get() != nullptr);
        EXPECT_EQ(1, sp.use_count());
        EXPECT_EQ(123u, sp->mValue);
    }

    TEST_F(ASharedPtr, ResetToPointer)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(456));
        sp.reset(new BaseClass(123));
        EXPECT_TRUE(sp.get() != nullptr);
        EXPECT_EQ(1, sp.use_count());
        EXPECT_EQ(123u, sp->mValue);
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, ResetToPointerDeleterBefore)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(456), DeletingTestDeleter<BaseClass>(wasCalled));
        sp.reset(new BaseClass(123));
        EXPECT_TRUE(wasCalled);
    }

    TEST_F(ASharedPtr, ResetToPointerDeleterOnNew)
    {
        {
            PlatformSharedPointer<BaseClass> sp(new BaseClass(456));
            sp.reset(new BaseClass(123), DeletingTestDeleter<BaseClass>(wasCalled));
            EXPECT_FALSE(wasCalled);
        }
        EXPECT_TRUE(wasCalled);
        EXPECT_EQ(0u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, ResetToConvertiblePointer)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(456));
        sp.reset(new DerivedClass(123));
        EXPECT_EQ(123u, sp->mValue);
    }

    TEST_F(ASharedPtr, ResetToConvertiblePointerDeleterOnNew)
    {
        {
            PlatformSharedPointer<BaseClass> sp(new BaseClass(456));
            sp.reset(new DerivedClass(123), DeletingTestDeleter<DerivedClass>(wasCalled));
            EXPECT_FALSE(wasCalled);
        }
        EXPECT_TRUE(wasCalled);
        EXPECT_EQ(0u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, Swap)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other(new BaseClass(456));
        sp.swap(other);
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(456u, sp->mValue);
        EXPECT_EQ(1, sp.use_count());
        EXPECT_EQ(1, other.use_count());
        EXPECT_EQ(2u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, SwapWithEmpty)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other;
        sp.swap(other);
        EXPECT_TRUE(sp.get() == nullptr);
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(1u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, SwapFreeFunction)
    {
        PlatformSharedPointer<BaseClass> sp(new BaseClass(123));
        PlatformSharedPointer<BaseClass> other(new BaseClass(456));
        using std::swap;
        swap(sp, other);
        EXPECT_EQ(123u, other->mValue);
        EXPECT_EQ(456u, sp->mValue);
        EXPECT_EQ(2u, BaseClass::mReferences);
    }

    TEST_F(ASharedPtr, OperatorBool)
    {
        PlatformSharedPointer<BaseClass> sp;
        EXPECT_FALSE(sp);
        sp.reset(new BaseClass(123));
        EXPECT_TRUE(!!sp);
        sp.reset();
        EXPECT_FALSE(sp);
    }

    TEST_F(ASharedPtr, EqualUnequalOperators)
    {
        PlatformSharedPointer<DerivedClass> sp(new DerivedClass(123));
        PlatformSharedPointer<DerivedClass> sameAsSp(sp);
        PlatformSharedPointer<BaseClass> sameAsSpButOtherType(sp);
        PlatformSharedPointer<DerivedClass> different(new DerivedClass(123));
        PlatformSharedPointer<BaseClass> differentOtherType(new BaseClass(123));

        EXPECT_TRUE(sp == sp);
        EXPECT_TRUE(sp == sameAsSp);
        EXPECT_TRUE(sp == sameAsSpButOtherType);
        EXPECT_FALSE(sp == different);
        EXPECT_FALSE(sp == differentOtherType);

        EXPECT_FALSE(sp != sp);
        EXPECT_FALSE(sp != sameAsSp);
        EXPECT_FALSE(sp != sameAsSpButOtherType);
        EXPECT_TRUE(sp != different);
        EXPECT_TRUE(sp != differentOtherType);
    }

    TEST_F(ASharedPtr, HashesSameAsInternalPointer)
    {
        PlatformSharedPointer<DerivedClass> sp1;
        PlatformSharedPointer<DerivedClass> sp2(new DerivedClass(123));
        PlatformSharedPointer<DerivedClass> sp3 = sp2;

        EXPECT_TRUE(ramses_capu::HashValue(sp1) == ramses_capu::HashValue(sp1.get()));
        EXPECT_TRUE(ramses_capu::HashValue(sp2) == ramses_capu::HashValue(sp2.get()));
        EXPECT_TRUE(ramses_capu::HashValue(sp3) == ramses_capu::HashValue(sp3.get()));
    }

    TEST_F(ASharedPtr, HashesSameWhenReferencingSameObject)
    {
        PlatformSharedPointer<DerivedClass> sp1(new DerivedClass(123));
        PlatformSharedPointer<DerivedClass> sp2 = sp1;
        EXPECT_TRUE(ramses_capu::HashValue(sp1) == ramses_capu::HashValue(sp2));
    }

    TEST_F(ASharedPtr, HashesDifferentWhenReferencingDifferentObject)
    {
        PlatformSharedPointer<DerivedClass> sp1(new DerivedClass(123));
        PlatformSharedPointer<DerivedClass> sp2(new DerivedClass(123));
        PlatformSharedPointer<DerivedClass> sp3;

        EXPECT_TRUE(ramses_capu::HashValue(sp1) != ramses_capu::HashValue(sp2));
        EXPECT_TRUE(ramses_capu::HashValue(sp1) != ramses_capu::HashValue(sp3));
    }

    TEST_F(ASharedPtr, HashesSameWhenHasDeleter)
    {
        struct NoopDeleter
        {
            void operator()(int64_t*)
            {
            }
        };

        PlatformSharedPointer<int64_t> ptr( new int64_t(858918934591ll), NoopDeleter() );
        PlatformSharedPointer<int64_t> same_ptr( ptr.get(), NoopDeleter() );

        EXPECT_EQ(ramses_capu::HashValue(ptr), ramses_capu::HashValue(same_ptr));

        delete ptr.get();
    }

    TEST_F(ASharedPtr, ADestructorGetsCalledWhenPlatformSharedPointerGoesOutOfScope)
    {
        Deleteme* object = new Deleteme();
        PlatformSharedPointer<Deleteme> pointer(object);

        EXPECT_CALL(*object, destructor());
    }

    TEST_F(ASharedPtr, DeleterGetsCalledWhenPlatformSharedPointerGoesOutOfScope)
    {
        Deleteme object;
        Bool deleterCalled = false;
        TestDeleter<Deleteme> deleter(deleterCalled);
        {
            PlatformSharedPointer<Deleteme> pointer(&object, deleter);
        }
        ::testing::Mock::VerifyAndClearExpectations(&object);

        //using gmock does not work here as Deleter gets copied internally
        EXPECT_TRUE(deleterCalled);

        EXPECT_CALL(object, destructor());
    }
}
