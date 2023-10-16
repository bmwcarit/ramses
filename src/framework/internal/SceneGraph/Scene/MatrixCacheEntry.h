//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ETransformMatrixType.h"
#include "impl/DataTypesImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include <array>

namespace ramses::internal
{
    struct MatrixCacheEntry
    {
        MatrixCacheEntry()
        {
            setDirty();
        }

        void setDirty()
        {
            m_matrixDirty[ETransformationMatrixType_World] = true;
            m_matrixDirty[ETransformationMatrixType_Object] = true;
        }

        std::array<glm::mat4, ETransformationMatrixType_COUNT> m_matrix{};
        std::array<bool, ETransformationMatrixType_COUNT>      m_matrixDirty{};
        bool                                                   m_isIdentity{true};
    };
}
