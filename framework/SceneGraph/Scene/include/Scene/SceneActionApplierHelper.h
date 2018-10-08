//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEACTIONAPPLIERHELPER_H
#define RAMSES_SCENEACTIONAPPLIERHELPER_H

#include "SceneActionApplier.h"
#include "Animation/AnimationSystemFactory.h"
#include "Scene/SceneActionCollection.h"

namespace ramses_internal
{
    class IScene;

    class SceneActionApplierHelper
    {
    public:
        inline
        explicit SceneActionApplierHelper(IScene& scene)
            : m_scene(scene)
            , m_animationSystemFactory(EAnimationSystemOwner_Scenemanager)
        {
        }

        void applyActionsOnScene(const SceneActionCollection& actions)
        {
            SceneActionApplier::ApplyActionsOnScene(m_scene, actions, &m_animationSystemFactory);
        }

    private:
        IScene& m_scene;
        AnimationSystemFactory m_animationSystemFactory;
    };
}

#endif
