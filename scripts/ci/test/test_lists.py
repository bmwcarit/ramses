#  -------------------------------------------------------------------------
#  Copyright (C) 2023 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import sys
import os

sys.path.append(os.path.realpath(os.path.dirname(__file__) + "/../common"))
import lists  # noqa E402 module level import not at top of file


class TestFilterIncludesExcludes:
    def test_basic_behavior(self):
        assert [] == lists.filter_includes_excludes([])
        assert ['a', 'b', 'c'] == lists.filter_includes_excludes(['a', 'b', 'c'])

    def test_basic_incldues(self):
        tst = ['aa', 'ab', 'bb', 'cc']
        assert [] == lists.filter_includes_excludes(tst, includes=[])
        assert ['aa', 'ab'] == lists.filter_includes_excludes(tst, includes=['a'])
        assert ['ab'] == lists.filter_includes_excludes(tst, includes=['^ab$'])
        assert ['aa', 'ab', 'bb'] == lists.filter_includes_excludes(tst, includes=['a', 'b'])
        assert ['aa', 'ab', 'bb'] == lists.filter_includes_excludes(tst, includes=['a|b'])

    def test_basic_excludes(self):
        tst = ['aa', 'ab', 'bb', 'cc']
        assert tst == lists.filter_includes_excludes(tst, excludes=[])
        assert ['aa', 'cc'] == lists.filter_includes_excludes(tst, excludes=['b'])
        assert ['aa', 'ab', 'bb'] == lists.filter_includes_excludes(tst, excludes=['^cc$'])
        assert ['aa', 'ab'] == lists.filter_includes_excludes(tst, excludes=['bb|cc'])
        assert ['aa', 'ab'] == lists.filter_includes_excludes(tst, excludes=['bb', 'cc'])

    def test_combined(self):
        tst = ['aa', 'ab', 'bb', 'ac', 'cc']
        assert [] == lists.filter_includes_excludes(tst, includes=[], excludes=[])
        assert ['aa', 'ac'] == lists.filter_includes_excludes(tst, includes=['a'], excludes=['b'])

    def test_with_key(self):
        tst = [(1, 'aa'), (2, 'ab'), (2, 'bb'), (3, 'cc')]
        assert tst == lists.filter_includes_excludes(tst, includes=['.*'], excludes=[], key=lambda e: e[1])
        assert [(1, 'aa'), (2, 'ab')] == lists.filter_includes_excludes(tst, includes=['a'], key=lambda e: e[1])
        assert [(1, 'aa'), (3, 'cc')] == lists.filter_includes_excludes(tst, excludes=['b'], key=lambda e: e[1])


class TestStableUnique:
    def test_basic_behavior(self):
        assert [] == lists.stable_unique([])
        assert [1, 2, 3] == lists.stable_unique([1, 2, 3])
        assert [2, 3, 1] == lists.stable_unique([2, 3, 2, 3, 1])

    def test_with_key(self):
        assert [(2, 'C'), (3, 'D'), (1, 'E')] == lists.stable_unique([(2, 'A'), (3, 'B'), (2, 'C'), (3, 'D'), (1, 'E')], key=lambda e: e[0])

    def test_with_non_hashable_type_with_key(self):
        assert [{1: 10}, {1: 20}] == lists.stable_unique([{1: 10}, {1: 20}, {1: 10}], key=lambda e: e[1])


class TestPatternPrioritySort:
    SEQ = [('aa', 3), ('ab', 2), ('bb', 1), ('cc', 4)]

    def test_no_pattern_no_fallback(self):
        assert [] == lists.pattern_priority_sort([], [], lambda e: e[0], [])
        assert self.SEQ == lists.pattern_priority_sort(self.SEQ, [], lambda e: e[0], [])

    def test_non_overlapping_patterns(self):
        patterns = [('^a', 10), ('^b', 20)]
        assert [] == lists.pattern_priority_sort([], patterns, lambda e: e[0], [])
        assert [('bb', 1), ('aa', 3), ('ab', 2), ('cc', 4)] == lists.pattern_priority_sort(self.SEQ, patterns[-1:], lambda e: e[0], [])
        assert [('bb', 1), ('aa', 3), ('ab', 2), ('cc', 4)] == lists.pattern_priority_sort(self.SEQ, patterns, lambda e: e[0], [])
        assert [('bb', 1), ('aa', 3), ('ab', 2), ('cc', 4)] == lists.pattern_priority_sort(self.SEQ, reversed(patterns), lambda e: e[0], [])

    def test_overlapping_patterns(self):
        patterns = [('a', 10), ('b', 20)]
        assert [('ab', 2), ('bb', 1), ('aa', 3), ('cc', 4)] == lists.pattern_priority_sort(self.SEQ, patterns, lambda e: e[0], [])

    def test_fallback_only(self):
        fallbacks = [lambda e: -e[1], lambda e: e[0]]
        assert [] == lists.pattern_priority_sort([], [], lambda e: e[0], fallbacks)
        assert [('bb', 1), ('ab', 2), ('aa', 3), ('cc', 4)] == lists.pattern_priority_sort(self.SEQ, [], lambda e: e[0], fallbacks[:1])
        seq = self.SEQ + [('ac', 1)]
        assert [('bb', 1), ('ac', 1), ('ab', 2), ('aa', 3), ('cc', 4)] == lists.pattern_priority_sort(seq, [], lambda e: e[0], fallbacks)

    def test_other_default_prio(self):
        patterns = [('b', 10)]
        assert [('ab', 2), ('bb', 1), ('aa', 3), ('cc', 4)] == lists.pattern_priority_sort(self.SEQ, patterns, lambda e: e[0], [], default_priority=0)
        assert [('aa', 3), ('cc', 4), ('ab', 2), ('bb', 1)] == lists.pattern_priority_sort(self.SEQ, patterns, lambda e: e[0], [], default_priority=20)

    def test_patterns_and_fallbacks(self):
        patterns = [('a', 20), ('b', 10)]
        assert [('ab', 2), ('aa', 3), ('bb', 1), ('cc', 4)] == lists.pattern_priority_sort(self.SEQ, patterns, lambda e: e[0], [lambda e: -e[1]])

    def test_comlex_elements(self):
        patterns = [('a', 20), ('b', 10)]
        seq = [{'foo': 'aa'}, {'foo': 'bb'}, {'foo': 'ab'}]
        assert [{'foo': 'aa'}, {'foo': 'ab'}, {'foo': 'bb'}] == lists.pattern_priority_sort(seq, patterns, lambda e: e['foo'])
