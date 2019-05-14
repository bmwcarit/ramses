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
#include "Transfer/ResourceTypes.h"
#include "Collections/Vector.h"
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
        SceneResourceAction(MemoryHandle handle_ = InvalidMemoryHandle, ESceneResourceAction action_ = ESceneResourceAction_Invalid)
            : handle(handle_)
            , action(action_)
        {
        }

        Bool operator!=(const SceneResourceAction& other) const
        {
            return handle != other.handle
                || action != other.action;
        }

        Bool operator==(const SceneResourceAction& other) const
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
        Bool   empty() const;
        void   putToSceneAction(SceneActionCollection& action) const;
        void   getFromSceneAction(SceneActionCollection::SceneActionReader& action);
        String asString() const;
        UInt   getPutSizeEstimate() const;

        ResourceContentHashVector m_addedClientResourceRefs;
        ResourceContentHashVector m_removedClientResourceRefs;

        SceneResourceActionVector m_sceneResourceActions;
    };
}

#endif
