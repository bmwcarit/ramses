//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationSystemFactory.h"
#include "Animation/AnimationSystem.h"
#include "Animation/ActionCollectingAnimationSystem.h"

namespace ramses_internal
{
    AnimationSystemFactory::AnimationSystemFactory(EAnimationSystemOwner ownerType, SceneActionCollection* actionCollector)
        : m_ownerType(ownerType)
        , m_actionCollector(actionCollector)
    {
    }

    IAnimationSystem* AnimationSystemFactory::createAnimationSystem(UInt32 flags, const AnimationSystemSizeInformation& sizeInfo)
    {
        switch (m_ownerType)
        {
        case EAnimationSystemOwner_Scenemanager:
            // Scene manager does not need full processing
            flags &= ~EAnimationSystemFlags_FullProcessing;
            return new AnimationSystem(flags, sizeInfo);
        case EAnimationSystemOwner_Renderer:
            // Renderer requires full processing
            flags |= EAnimationSystemFlags_FullProcessing;
            return new AnimationSystem(flags, sizeInfo);
        case EAnimationSystemOwner_Client:
            assert(m_actionCollector != 0);
            return new ActionCollectingAnimationSystem(flags, *m_actionCollector, sizeInfo);
        default:
            return NULL;
        }
    }
}
