//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENERESOURCECHANGES_H
#define RAMSES_SCENERESOURCECHANGES_H

#include "SceneAPI/SceneTypes.h"
#include "Scene/SceneActionCollection.h"
#include "Collections/String.h"

namespace ramses_internal
{
    class SceneAction;

    enum ESceneResourceAction
    {
        ESceneResourceAction_Invalid = 0,

        ESceneResourceAction_CreateRenderBuffer,
        ESceneResourceAction_DestroyRenderBuffer,

        ESceneResourceAction_CreateRenderTarget,
        ESceneResourceAction_DestroyRenderTarget,

        ESceneResourceAction_CreateStreamTexture,
        ESceneResourceAction_DestroyStreamTexture,

        ESceneResourceAction_CreateBlitPass,
        ESceneResourceAction_DestroyBlitPass,

        ESceneResourceAction_CreateDataBuffer,
        ESceneResourceAction_UpdateDataBuffer,
        ESceneResourceAction_DestroyDataBuffer,

        ESceneResourceAction_CreateTextureBuffer,
        ESceneResourceAction_UpdateTextureBuffer,
        ESceneResourceAction_DestroyTextureBuffer,

        ESceneResourceAction_NUMBER_OF_ELEMENTS
    };

    struct SceneResourceAction
    {
        SceneResourceAction()
            : SceneResourceAction(InvalidMemoryHandle, ESceneResourceAction_Invalid)
        {}

        SceneResourceAction(MemoryHandle handle_, ESceneResourceAction action_)
            : handle(handle_)
            , action(action_)
        {
        }

        bool operator!=(const SceneResourceAction& other) const
        {
            return handle != other.handle
                || action != other.action;
        }

        bool operator==(const SceneResourceAction& other) const
        {
            return !(*this != other);
        }

        MemoryHandle         handle;
        ESceneResourceAction action;
    };

    typedef std::vector<SceneResourceAction> SceneResourceActionVector;

    struct SceneResourceChanges
    {
        void   clear();
        bool   empty() const;
        void   putToSceneAction(SceneActionCollection& action) const;
        void   getFromSceneAction(SceneActionCollection::SceneActionReader& action);
        String asString() const;
        UInt   getPutSizeEstimate() const;

        ResourceContentHashVector m_addedClientResourceRefs;
        ResourceContentHashVector m_removedClientResourceRefs;

        SceneResourceActionVector m_sceneResourceActions;

        bool operator==(const SceneResourceChanges& other) const
        {
            return m_addedClientResourceRefs == other.m_addedClientResourceRefs
                && m_removedClientResourceRefs == other.m_removedClientResourceRefs
                && m_sceneResourceActions == other.m_sceneResourceActions;
        }

        bool operator!=(const SceneResourceChanges& other) const
        {
            return !(*this == other);
        }
    };
}

#endif
