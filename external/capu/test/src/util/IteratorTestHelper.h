/*
* Copyright (C) 2017 BMW Car IT GmbH
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

#include "ramses-capu/Config.h"
#include "gmock/gmock.h"
#include <iterator>

namespace ramses_capu
{

    template<typename ITERATABLE, typename IteratorTag>
    class IteratorTestCompilation;

    template<typename ITERATABLE>
    class IteratorTestCompilation<ITERATABLE, std::input_iterator_tag>
    {
    protected:
        typedef typename ITERATABLE::iterator Iterator;

    private:
        void isDefaultConstructable()
        {
            Iterator a;
            UNUSED(a);
        }

        void isCopyConstructable(ITERATABLE& iteratable)
        {
            Iterator a = iteratable.begin();;
            Iterator b(a);
            EXPECT_EQ(a, b);
        }

        void isEqualComparable(ITERATABLE& iteratable)
        {
            Iterator a = iteratable.begin();
            Iterator b = iteratable.begin();
            Iterator c = iteratable.end();
            EXPECT_TRUE(a == b);
            EXPECT_FALSE(a == c);
        }

        void isInEqualComparable(ITERATABLE& iteratable)
        {
            Iterator a = iteratable.begin();
            Iterator b = iteratable.begin();
            Iterator c = iteratable.end();
            EXPECT_FALSE(a != b);
            EXPECT_TRUE(a != c);
        }

        void isDereferencable(ITERATABLE& iteratable)
        {
            Iterator a = iteratable.begin();
            EXPECT_EQ(1u, *a);
        }

        void isPreIncrementable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator current = iteratable.begin();
            Iterator end = iteratable.end();
            EXPECT_FALSE(start == ++current);
            ++current;
            EXPECT_TRUE(end == ++current);
        }

        void isPostIncrementable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator current = iteratable.begin();
            EXPECT_TRUE(start == current++);
            EXPECT_FALSE(start == current);
        }

        void isPostIncremenDereferencable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator current = iteratable.begin();
            EXPECT_TRUE(*start == *current++);
        }

        void isSwapable(ITERATABLE& iteratable)
        {
            Iterator a = iteratable.begin();
            Iterator b = iteratable.end();

            Iterator c = iteratable.begin();

            swap(a, b);

            EXPECT_EQ(b,c);
        }


    public:

        void runTests(ITERATABLE& iteratable)
        {
            isDefaultConstructable();
            isCopyConstructable(iteratable);
            isEqualComparable(iteratable);
            isInEqualComparable(iteratable);
            isDereferencable(iteratable);
            isPreIncrementable(iteratable);
            isPostIncrementable(iteratable);
            isPostIncremenDereferencable(iteratable);
            isSwapable(iteratable);
        }
    };

    template<typename ITERATABLE>
        class IteratorTestCompilation<ITERATABLE, std::forward_iterator_tag> : public IteratorTestCompilation<ITERATABLE, std::input_iterator_tag>
    {
    protected:
        typedef typename IteratorTestCompilation<ITERATABLE, std::input_iterator_tag>::Iterator Iterator;

    private:

        void isCopyAssignable(ITERATABLE& iteratable)
        {
            Iterator a = iteratable.begin();
            Iterator b = a;
            EXPECT_EQ(b, a);
        }

        void isDereferencableAndAssignable(ITERATABLE& iteratable)
        {
            Iterator a = iteratable.begin();
            Iterator b = iteratable.begin();

            *a = *b;
        }

    public:
        void runTests(ITERATABLE& iteratable)
        {
            IteratorTestCompilation<ITERATABLE, std::input_iterator_tag>::runTests(iteratable);
            isCopyAssignable(iteratable);
            isDereferencableAndAssignable(iteratable);
        }
    };

    template<typename ITERATABLE>
    class IteratorTestCompilation<ITERATABLE, std::bidirectional_iterator_tag> : public IteratorTestCompilation<ITERATABLE, std::forward_iterator_tag>
    {
    protected:
        typedef typename IteratorTestCompilation<ITERATABLE, std::forward_iterator_tag>::Iterator Iterator;

    private:
        void isPreDecrementable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator current = iteratable.end();
            Iterator end = iteratable.end();
            EXPECT_FALSE(end == --current);
            --current;
            --current;
            EXPECT_TRUE(start == current);
        }

        void isPostDecrementable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.end();
            Iterator current = iteratable.end();
            EXPECT_TRUE(start == current--);
            EXPECT_FALSE(start == current);
        }

        void isPostDecremenDereferencable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator current = iteratable.begin();
            Iterator moved = iteratable.begin();
            ++moved;
            ++current;
            EXPECT_TRUE(*moved == *current--);
            EXPECT_TRUE(*start == *current);
        }

    public:

        void runTests(ITERATABLE& iteratable)
        {
            IteratorTestCompilation<ITERATABLE, std::forward_iterator_tag>::runTests(iteratable);

            isPreDecrementable(iteratable);
            isPostDecrementable(iteratable);
            isPostDecremenDereferencable(iteratable);
        }
    };

    template<typename ITERATABLE>
    class IteratorTestCompilation<ITERATABLE, std::random_access_iterator_tag> : public IteratorTestCompilation<ITERATABLE,  std::bidirectional_iterator_tag>
    {
    private:

        typedef typename IteratorTestCompilation<ITERATABLE, std::bidirectional_iterator_tag>::Iterator Iterator;

        void isIncrementableByOffset(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator advance = start + 1;
            EXPECT_EQ(advance, ++start);
        }

        void isIncrementAssignable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator advance = iteratable.begin();
            Iterator& result = start += 1;

            EXPECT_EQ(start, ++advance);
            EXPECT_EQ(start, result);
        }

        void isIncrementableByIterator(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator advance = 1 + start;

            EXPECT_EQ(advance, ++start);
        }

        void isDecrementAssignable(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.end();
            Iterator advance = iteratable.end();
            Iterator& result = start -= 1;

            EXPECT_EQ(start, --advance);
            EXPECT_EQ(start, result);
        }

        void isDecrementableByIterator(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.end();
            Iterator advance = 1 - start;

            EXPECT_EQ(advance, --start);
        }

        void isDecrementalByIterator(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator end = iteratable.end();

            EXPECT_TRUE(end - start > 0);
        }

        void isConvertableToReference(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            EXPECT_EQ(start[1], *(start + 1));
        }

        void isComparableSmaler(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator end = iteratable.end();

            EXPECT_TRUE(start < end);
        }

        void isComparableSmalerEqual(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator end = iteratable.begin();

            EXPECT_TRUE(start <= end);
        }

        void isComparableBigger(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator end = iteratable.end();

            EXPECT_TRUE(end > start);
        }

        void isComparableBiggerEqual(ITERATABLE& iteratable)
        {
            Iterator start = iteratable.begin();
            Iterator end = iteratable.begin();

            EXPECT_TRUE(end >= start);
        }

    public:

        void runTests(ITERATABLE& iteratable)
        {
            IteratorTestCompilation<ITERATABLE, std::bidirectional_iterator_tag>::runTests(iteratable);
            isIncrementAssignable(iteratable);
            isIncrementableByOffset(iteratable);
            isIncrementableByIterator(iteratable);
            isDecrementAssignable(iteratable);
            isDecrementalByIterator(iteratable);
            isConvertableToReference(iteratable);
            isComparableSmaler(iteratable);
            isComparableSmalerEqual(iteratable);
            isComparableBigger(iteratable);
            isComparableBiggerEqual(iteratable);
        }
    };

    template<typename ITERATOR, typename iterator_tag>
    struct HasIteratorTag
    {
        static const bool result = false;
    };

    template<>
    struct HasIteratorTag<std::input_iterator_tag, std::input_iterator_tag>
    {
        static const bool result = true;
    };

    template<>
    struct HasIteratorTag<std::forward_iterator_tag, std::forward_iterator_tag>
    {
        static const bool result = true;
    };

    template<>
    struct HasIteratorTag<std::bidirectional_iterator_tag, std::bidirectional_iterator_tag>
    {
        static const bool result = true;
    };

    template<>
    struct HasIteratorTag<std::random_access_iterator_tag, std::random_access_iterator_tag>
    {
        static const bool result = true;
    };


    class IteratorTestHelper
    {
    public:
        template<typename ITERATABLE>
        static void IteratorImplementsAllNecessaryMethods(ITERATABLE& iteratable)
        {
            IteratorTestCompilation<ITERATABLE, typename ITERATABLE::iterator::iterator_category> testCompilation;
            testCompilation.runTests(iteratable);
        }

        template<typename ITERATABLE, typename iterator_tag>
        static void IteratorHasTag()
        {
            bool res = HasIteratorTag<typename ITERATABLE::iterator::iterator_category, iterator_tag>::result;
            EXPECT_TRUE(res);
        }
    };
}
