//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMFACTORY_H
#define RAMSES_ANIMATIONSYSTEMFACTORY_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "AnimationAPI/IAnimationSystem.h"

namespace ramses_internal
{
    class SceneActionCollection;
    class IAnimationSystem;
    struct AnimationSystemSizeInformation;

    enum EAnimationSystemOwner
    {
        EAnimationSystemOwner_Scenemanager = 0,
        EAnimationSystemOwner_Renderer,
        EAnimationSystemOwner_Client
    };

    class AnimationSystemFactory
    {
    public:
        AnimationSystemFactory(EAnimationSystemOwner ownerType, SceneActionCollection* actionCollector = 0);

        IAnimationSystem* createAnimationSystem(UInt32 flags, const AnimationSystemSizeInformation& sizeInfo);

    protected:
        EAnimationSystemOwner m_ownerType;
        SceneActionCollection* m_actionCollector;
    };
}

#endif
