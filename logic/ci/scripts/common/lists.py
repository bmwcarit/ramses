#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

import re
from collections import OrderedDict


def filter_includes_excludes(iterable, *, includes=['.*'], excludes=[], key=None):
    """Filter iterable by lists in include and exclude regexps.

    Both patterns only remove elements, an element is only kept when it matches
    at least one entry in includes and no entry in excludes.

    :param iterable: The input sequence
    :param includes: List of include regexp. Keep all elements that match at least
                     one of these. Empty list filters out all elements.
    :param excludes: List of exclude regexp. Remove all elements that match any of
                     these.
    :param key: Optional function to get key to match regexps against from element.
    """
    key_fun = key or (lambda e: e)
    include_re = re.compile('|'.join([f'(?:{i})'for i in includes]))
    exclude_re = re.compile('|'.join([f'(?:{e})'for e in excludes]))

    res = []
    for e in iterable:
        k = key_fun(e)
        if includes and include_re.search(k) and not (excludes and exclude_re.search(k)):
            res.append(e)
    return res


def stable_unique(iterable, *, key=None):
    """Remove duplicate entries in iterable while keeping their relative order.

    :param iterable: The input sequence.
    :param key: Optional function to extract keys from elements that should be
                unique. Uses whole element when None.
    """
    key_fun = key or (lambda e: e)
    d = OrderedDict()
    for e in iterable:
        d[key_fun(e)] = e
    return list(d.values())


def pattern_priority_sort(iterable, sort_patterns, pattern_key_fun, fallback_funs=[], default_priority=0):
    """Sort iterable by a list of regexp patterns with associated priority.

    This method takes an iterable and a list of regexp with priority and checks which regexps match
    the elements. The pattern with the highest matching priority determines the resulting priority.

    Same resulting priorities can be resolved by optional fallback functions. They have to return a
    numeric value for each element. Fallback functions are ordered, i.e. earlier function have higher
    precedence, later one are only consulted to disambiguate when all previous patterns and functions
    result in the same values.

    Result will be sorted from highest to lowest priority.

    :param iterable: The input sequence.
    :param sort_patterns: Tuples with (regexp string, numeric priority value).
    :param pattern_key_fun: Function to extract value that is used for regexp matching from elements.
    :param fallback_funs: Functions that gets elements (without applying pattern_key_fun) and return a
                          sorting priority value.
    :param default_priority: Default priority value for elements that do not match any regexp.
    """
    re_patterns = [(re.compile(e[0]), e[1]) for e in sort_patterns]

    def key_fun(e):
        prio = None
        for p in re_patterns:
            rx, rx_prio = p
            if rx.search(pattern_key_fun(e)):
                prio = max(prio or rx_prio, rx_prio)
        return tuple([prio or default_priority] + [f(e) for f in fallback_funs])
    return sorted(iterable, key=key_fun, reverse=True)
