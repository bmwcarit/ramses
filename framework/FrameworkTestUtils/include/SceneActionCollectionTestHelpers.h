//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEACTIONCOLLECTIONTESTHELPERS_H
#define RAMSES_SCENEACTIONCOLLECTIONTESTHELPERS_H

#include "gmock/gmock.h"
#include "Scene/SceneActionCollection.h"

namespace ramses_internal
{
    class SceneActionCollectionEqMatcher : public MatcherInterface<SceneActionCollection&>
    {
    public:
        SceneActionCollectionEqMatcher(const SceneActionCollection& actions)
            : m_actions(actions.copy())
        {
        }

        virtual bool MatchAndExplain(SceneActionCollection& arg, MatchResultListener* /*listener*/) const override
        {
            return arg == m_actions;
        }

        virtual void DescribeTo(::std::ostream* os) const override
        {
            *os << "equal to other SceneActionCollection";
        }
    private:
        SceneActionCollection m_actions;
    };

    inline Matcher<SceneActionCollection&> SceneActionCollectionEq(const SceneActionCollection& actions) {
        return MakeMatcher(new SceneActionCollectionEqMatcher(actions));
    }
}

#endif
